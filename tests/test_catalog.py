# SPDX-License-Identifier: MIT
# Copyright (c) 2026 dlgus8648

"""테스트 카탈로그(tests/README.md) 동기화 가드.

테스트를 추가했는데 카탈로그 표에 적지 않으면 여기서 실패한다. 그래야 "무엇이
테스트되는지" 문서가 코드와 어긋나지 않고 계속 진실을 유지한다.
"""

from __future__ import annotations

import re
from pathlib import Path

import pytest

_TESTS_DIR = Path(__file__).resolve().parent
_CATALOG = (_TESTS_DIR / "README.md").read_text(encoding="utf-8")

# 카탈로그가 다뤄야 하는 테스트 파일 (이 가드 파일 자체는 제외).
_CATALOGUED_FILES = ["test_textproc.py", "test_voices.py", "test_plugin_cli.py"]


def _test_functions(path: Path) -> list[str]:
    return re.findall(r"^def (test_\w+)", path.read_text(encoding="utf-8"), re.MULTILINE)


@pytest.mark.parametrize("filename", _CATALOGUED_FILES)
def test_every_test_is_documented(filename):
    path = _TESTS_DIR / filename
    missing = [fn for fn in _test_functions(path) if fn not in _CATALOG]
    assert not missing, (
        f"{filename}의 다음 테스트가 tests/README.md 카탈로그에 없습니다: {missing}.\n"
        "테스트를 추가/이름변경하면 tests/README.md 표도 함께 갱신하세요."
    )
