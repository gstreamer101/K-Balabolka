"""K-Balabolka GUI (Stage 4) — minimal PySide6 frontend over macttssink.

Layout follows the Windows Balabolka style at a high level: toolbar with
play/stop, an engine info label, a large text edit, and a status bar.
Only what macttssink currently supports (Stage 2) is exposed; properties
like rate/voice/pitch arrive in Stage 3 and will be added later.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

from PySide6.QtCore import QProcess, QProcessEnvironment, Qt
from PySide6.QtGui import QAction, QIcon, QKeySequence
from PySide6.QtWidgets import (
    QApplication,
    QLabel,
    QMainWindow,
    QPlainTextEdit,
    QStatusBar,
    QStyle,
    QToolBar,
    QVBoxLayout,
    QWidget,
)


PROJECT_ROOT = Path(__file__).resolve().parent.parent
PLUGIN_DIR = PROJECT_ROOT / "plugin" / "builddir"
GST_LAUNCH = "/Library/Frameworks/GStreamer.framework/Versions/1.0/bin/gst-launch-1.0"

# 문장 종결로 인정하는 문자 (이미 끝나있으면 마침표 중복 안 붙임)
_TERMINATORS = ".!?…。！？"
_INLINE_WHITESPACE = re.compile(r"[ \t]+")


def preprocess_for_speech(text: str) -> str:
    """모든 줄바꿈을 단락 구분으로 취급해 줄 사이마다 자연 휴식을 만든다.

    - 빈 줄과 단순 Enter를 동일하게 단락으로 처리
    - 줄 내부의 연속 공백/탭은 단일 공백으로 정리
    - 종결 부호(.!?…)로 끝나지 않는 줄엔 마침표를 추가해 휴식 유도
    - 줄들을 공백 하나로 이어 한 utterance로
    """
    lines = []
    for raw in text.splitlines():
        line = _INLINE_WHITESPACE.sub(" ", raw).strip()
        if not line:
            continue
        if line[-1] not in _TERMINATORS:
            line += "."
        lines.append(line)
    return " ".join(lines)


class MainWindow(QMainWindow):
    def __init__(self) -> None:
        super().__init__()
        self.setWindowTitle("K-Balabolka")
        self.resize(720, 520)

        self._process: QProcess | None = None

        self._build_toolbar()
        self._build_central()
        self._build_statusbar()

        self._update_char_count()

    # ---- UI construction ---------------------------------------------------

    def _build_toolbar(self) -> None:
        toolbar = QToolBar("Main")
        toolbar.setMovable(False)
        toolbar.setIconSize(toolbar.iconSize() * 1.1)
        self.addToolBar(toolbar)

        style = self.style()

        self.play_action = QAction(
            style.standardIcon(QStyle.StandardPixmap.SP_MediaPlay),
            "재생",
            self,
        )
        self.play_action.setShortcut(QKeySequence("Ctrl+Return"))
        self.play_action.setToolTip("선택된 텍스트를 음성으로 재생 (Cmd+Enter)")
        self.play_action.triggered.connect(self._on_play)
        toolbar.addAction(self.play_action)

        self.stop_action = QAction(
            style.standardIcon(QStyle.StandardPixmap.SP_MediaStop),
            "정지",
            self,
        )
        self.stop_action.setShortcut(QKeySequence("Ctrl+."))
        self.stop_action.setToolTip("재생 중지 (Cmd+.)")
        self.stop_action.triggered.connect(self._on_stop)
        self.stop_action.setEnabled(False)
        toolbar.addAction(self.stop_action)

    def _build_central(self) -> None:
        central = QWidget()
        layout = QVBoxLayout(central)
        layout.setContentsMargins(8, 8, 8, 8)
        layout.setSpacing(6)

        engine_label = QLabel("엔진: macOS AVSpeechSynthesizer (macttssink)")
        engine_label.setStyleSheet("color: #555; padding: 2px 4px;")
        layout.addWidget(engine_label)

        self.text_edit = QPlainTextEdit()
        self.text_edit.setPlaceholderText("여기에 읽을 텍스트를 붙여넣으세요…")
        self.text_edit.textChanged.connect(self._update_char_count)
        layout.addWidget(self.text_edit, stretch=1)

        self.setCentralWidget(central)

    def _build_statusbar(self) -> None:
        bar = QStatusBar()
        self.setStatusBar(bar)

        self.status_label = QLabel("준비")
        self.engine_status = QLabel("macttssink")
        self.engine_status.setStyleSheet("color: #777;")
        self.char_label = QLabel("글자: 0")

        bar.addWidget(self.status_label, stretch=1)
        bar.addPermanentWidget(self.engine_status)
        bar.addPermanentWidget(QLabel("│"))
        bar.addPermanentWidget(self.char_label)

    # ---- Actions -----------------------------------------------------------

    def _update_char_count(self) -> None:
        n = len(self.text_edit.toPlainText())
        self.char_label.setText(f"글자: {n}")

    def _on_play(self) -> None:
        if self._process is not None:
            return  # 이미 재생 중

        text = preprocess_for_speech(self.text_edit.toPlainText())
        if not text:
            self.status_label.setText("입력된 텍스트가 없습니다.")
            return

        if not PLUGIN_DIR.exists():
            self._fail(f"플러그인 빌드 폴더를 찾을 수 없음: {PLUGIN_DIR}")
            return
        if not Path(GST_LAUNCH).exists():
            self._fail(f"gst-launch-1.0이 없음: {GST_LAUNCH}")
            return

        env = QProcessEnvironment.systemEnvironment()
        env.insert("GST_PLUGIN_PATH", str(PLUGIN_DIR))

        proc = QProcess(self)
        proc.setProcessEnvironment(env)
        proc.setProcessChannelMode(QProcess.ProcessChannelMode.MergedChannels)
        proc.finished.connect(self._on_finished)
        proc.errorOccurred.connect(self._on_error)

        args = [
            "--quiet",
            "fdsrc",
            "!",
            "text/x-raw,format=utf8",
            "!",
            "macttssink",
        ]

        self._process = proc
        proc.start(GST_LAUNCH, args)
        if not proc.waitForStarted(3000):
            self._fail("gst-launch-1.0 시작 실패")
            self._process = None
            return

        proc.write(text.encode("utf-8"))
        proc.closeWriteChannel()

        self.play_action.setEnabled(False)
        self.stop_action.setEnabled(True)
        self.status_label.setText("재생 중…")

    def _on_stop(self) -> None:
        if self._process is None:
            return
        self._process.terminate()
        if not self._process.waitForFinished(1500):
            self._process.kill()
        # finished 시그널에서 상태 정리됨

    def _on_finished(self, exit_code: int, exit_status: QProcess.ExitStatus) -> None:
        self._process = None
        self.play_action.setEnabled(True)
        self.stop_action.setEnabled(False)
        if exit_status == QProcess.ExitStatus.CrashExit:
            self.status_label.setText("정지됨")
        elif exit_code != 0:
            self.status_label.setText(f"실패 (exit {exit_code})")
        else:
            self.status_label.setText("준비")

    def _on_error(self, err: QProcess.ProcessError) -> None:
        self._fail(f"프로세스 에러: {err}")

    def _fail(self, message: str) -> None:
        self.status_label.setText(message)
        self.play_action.setEnabled(True)
        self.stop_action.setEnabled(False)


def main() -> int:
    app = QApplication(sys.argv)
    app.setApplicationName("K-Balabolka")
    window = MainWindow()
    window.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
