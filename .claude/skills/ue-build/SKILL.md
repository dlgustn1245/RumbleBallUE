---
name: ue-build
description: RumbleBallUE C++ 코드를 빌드하고 컴파일·링크·UHT 에러를 고친다. .h/.cpp/.Build.cs를 수정한 뒤 검증할 때는 사용자가 빌드를 언급하지 않아도 이 스킬을 쓴다. "빌드해줘", "컴파일 에러", "LNK2019", "unresolved external", "generated.h", "UHT 에러", "Live Coding" 같은 요청에도 쓴다.
---

# UE 빌드 · 에러 해결

## 빌드 실행

```powershell
powershell -File .claude/skills/ue-build/scripts/build.ps1                          # 에디터 타깃, Development
powershell -File .claude/skills/ue-build/scripts/build.ps1 -Target RumbleBallUE -Config Shipping   # 게임 타깃
```

스크립트는 에러·경고 줄만 요약해 출력하고, 전체 로그는 `Saved/Logs/ClaudeBuild.log`에 남긴다. 요약만으로 원인이 안 보이면 그 로그를 읽는다.

| 종료 코드 | 의미 | 할 일 |
|-----------|------|-------|
| 0 | 성공 | 남은 경고 수와 함께 보고 |
| 1 | 빌드 실패 | 아래 "에러 해결 순서" |
| 2 | 에디터 실행 중이라 빌드 거부 | 빌드하지 말고 사용자에게 요청 |

종료 코드 2일 때는 사용자에게 다음 둘 중 하나를 요청한다.
- **에디터를 닫는다**: 헤더, `UCLASS`/`UPROPERTY`/`UFUNCTION`을 바꿨다면 이쪽을 권한다. Live Coding은 리플렉션 변경을 안전하게 반영하지 못한다.
- **에디터에서 Live Coding을 실행한다**(Ctrl+Alt+F11): `.cpp` 함수 본문만 바꿨을 때 쓴다.

에디터 프로세스를 직접 종료하지 않는다. 저장하지 않은 레벨·블루프린트 작업이 사라진다.

## 에러 해결 순서

1. 첫 번째 에러부터 본다. 뒤의 에러는 대부분 첫 에러의 연쇄 효과다.
2. 아래 표에서 패턴을 찾아 고치고 다시 빌드한다.
3. 같은 에러가 두 번 반복되면 추측 수정을 멈춘다. 전체 로그와 관련 헤더를 다시 읽는다.

| 증상 | 원인 | 해결 |
|------|------|------|
| `LNK2019`/`LNK2001` unresolved external | 사용하는 엔진 모듈이 Build.cs 의존성에 없음 | 심볼이 속한 모듈(엔진 경로 `Engine/Source/Runtime/<Module>/`)을 `PublicDependencyModuleNames`에 추가 |
| `Cannot open include file` | 경로 오류 또는 모듈 누락 | 프로젝트 헤더는 모듈 루트 기준(`"Enemy/RBEnemy.h"`), 엔진 헤더는 모듈 의존성 확인 |
| `*.generated.h` 관련 에러 | include 순서 | `.generated.h`를 헤더의 마지막 include로 |
| UHT `Error:` | 리플렉션이 지원하지 않는 타입이나 지정자 | 메시지의 줄 번호 확인. UObject 포인터는 `TObjectPtr`, 중첩 컨테이너(`TArray<TArray<>>`)는 USTRUCT로 감싼다 |
| `C2027` use of undefined type | 헤더에는 전방 선언만 하고 `.cpp` include를 빠뜨림 | `.cpp`에 해당 헤더 include |
| `C4996` deprecated | UE 5.8에서 폐기된 API | 경고 메시지가 안내하는 대체 API 사용 |
| `Unable to build while Live Coding is active` | 에디터가 열려 있음 | 종료 코드 2와 동일하게 처리 |

엔진 API가 확실하지 않으면 기억에 의존하지 말고 엔진 헤더를 직접 확인한다. UE 5.8은 API가 자주 바뀐다.
엔진 소스 위치는 `C:\Program Files\Epic Games\UE_5.8\Engine\Source\`이다.

## 완료 보고

- 성공하면 "빌드 성공(타깃/구성)"과 남은 경고 수를 보고한다.
- 빌드하지 못했으면(에디터 실행 중 등) 그 사실과 사용자가 할 일을 적는다. 빌드하지 않은 변경을 완료라고 보고하지 않는다.
- 새 `UCLASS`나 `UPROPERTY`를 추가했다면, 에디터를 다시 열어야 반영된다고 알린다.
