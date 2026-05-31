<!--
이 템플릿은 GitHub이 PR 작성 시 자동으로 채워줍니다.
각 섹션을 채우고, 체크리스트는 해당되는 항목에 [x] 로 표시해 주세요.
해당 없는 항목은 그대로 [ ] 로 두시면 됩니다.
-->

## 요약

<!-- 이 PR이 무엇을 하고, 왜 하는지 한두 문장으로. -->

## 관련 이슈

<!--
이슈를 자동으로 닫으려면 "Closes #N" 형식으로.
참조만 하려면 "Refs #N".
관련 이슈가 없으면 이 섹션은 비워두셔도 됩니다.
-->

Closes #

## 변경 유형

- [ ] 버그 수정 (bug fix)
- [ ] 새 기능 (feature)
- [ ] 문서 (documentation)
- [ ] 리팩토링 (refactor)
- [ ] 빌드 / CI / 도구 (build / ci / tooling)
- [ ] 기타:

## 빌드 / 동작 확인

- [ ] 로컬에서 [BUILD.md](../blob/main/BUILD.md) 절차로 빌드가 성공한다
- [ ] 변경한 부분의 동작을 직접 확인했다 (재생 / export / GUI 등)
- [ ] 기존 기능에 회귀가 없음을 확인했다 (해당 시)

## 코드 스타일 — [#11](https://github.com/gstreamer101/K-Balabolka/issues/11)

해당하는 파일이 있을 때만 체크.

- [ ] Python 변경: `ruff check gui/` 및 `ruff format gui/` 통과
- [ ] C / Objective-C 변경: `clang-format -i` 적용

## 라이선스 / SPDX — [CONTRIBUTING.md § 4](../blob/main/CONTRIBUTING.md#4-디렉터리-간-코드-이동-규칙-라이선스-무결성)

- [ ] 새로 추가한 소스 파일에 SPDX 라이선스 헤더를 추가했다
- [ ] 디렉터리 간에 코드를 옮긴 변경이 있다면, 원본의 SPDX 헤더를 함께 옮겼다
- [ ] 라이선스가 다른 디렉터리로 코드를 옮긴 경우, 본 PR 본문에 그 사실을 명시했다

## DCO 서명 — [CONTRIBUTING.md § 2](../blob/main/CONTRIBUTING.md#2-dco-developer-certificate-of-origin-동의)

- [ ] 이 PR의 모든 커밋이 `git commit -s`로 작성되어 `Signed-off-by:` 줄이 있다

> 서명이 누락된 커밋이 있다면 `git rebase -i HEAD~N` → `reword` → 마지막에 `Signed-off-by: 이름 <이메일>` 한 줄 추가 후 force-push로 보완할 수 있습니다. 또는 `git commit --amend -s` (단일 커밋).

## 추가 참고 사항

<!-- 리뷰어가 알아두면 좋을 컨텍스트, 스크린샷, 트레이드오프 결정 이유 등 -->
