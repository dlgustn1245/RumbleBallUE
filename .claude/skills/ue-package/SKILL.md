---
name: ue-package
description: RumbleBallUE를 Windows(Win64) 실행 파일로 패키징한다(BuildCookRun). 빌드, 쿡, 스테이징까지 수십 분이 걸리는 작업이라 사용자가 /ue-package로 직접 요청할 때만 실행한다.
argument-hint: [Development|Shipping]
disable-model-invocation: true
---

# Windows 패키징

구성은 인자로 받는다. 기본값은 `Development`다.
- `Development`: 콘솔 명령, `stat`, 로그를 쓸 수 있어 플레이테스트와 성능 측정에 쓴다.
- `Shipping`: 배포용이다. 테스트 코드(`WITH_DEV_AUTOMATION_TESTS`)와 콘솔이 빠진다.

## 실행

에디터가 열려 있어도 패키징은 되지만, 쿡이 에셋을 읽는 동안 에디터에서 저장하면 충돌할 수 있다. 실행 전에 사용자에게 에디터 작업을 저장해 달라고 알린다.

```powershell
$UE = "C:\Program Files\Epic Games\UE_5.8\Engine"
$P  = "C:\Github\RumbleBallUE\RumbleBallUE.uproject"
& "$UE\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="$P" -platform=Win64 -clientconfig=Development `
	-build -cook -stage -pak -archive -archivedirectory="C:\Github\RumbleBallUE\Saved\Packages" -nop4 -utf8output |
	Out-File -FilePath "C:\Github\RumbleBallUE\Saved\Logs\ClaudePackage.log" -Encoding utf8
```

오래 걸리므로 백그라운드로 실행하고, 끝나면 로그를 확인한다. 출력은 `Saved/Packages/Windows/`에 생긴다. `Saved/`는 gitignore 대상이라 커밋되지 않는다.

## 결과 확인

로그 끝의 `BUILD SUCCESSFUL` 또는 `AutomationTool exiting with ExitCode=0`으로 성공을 판단한다. 실패하면 첫 `Error:` 줄부터 본다.

| 증상 | 원인 | 해결 |
|------|------|------|
| `Failed to load` / `Couldn't find file for package` | 쿡 대상 에셋이 깨졌거나 삭제된 에셋을 참조함 | 로그의 에셋 경로를 사용자에게 알려 에디터에서 참조를 고치게 한다 |
| 컴파일 에러(게임 타깃에서만) | `WITH_EDITOR` 전용 코드가 런타임 코드에 섞임 | 에디터 전용 코드를 `#if WITH_EDITOR`로 감싼다 |
| 맵이 빠짐(실행하면 빈 화면) | 쿡할 맵 목록에 게임 맵이 없음 | `Config/DefaultGame.ini`의 `[/Script/UnrealEd.ProjectPackagingSettings]`에 `+MapsToCook=(FilePath="/Game/RumbleBall/Maps/L_...")` 추가 |

## 보고

결과물 경로, 소요 시간, 패키지 크기(`Saved/Packages/Windows` 폴더 크기)를 보고한다.
실행 확인(`RumbleBallUE.exe` 실행 → 메뉴 진입 → 1판 플레이)은 사용자 몫이므로, 확인할 항목을 짧게 적어 준다.
