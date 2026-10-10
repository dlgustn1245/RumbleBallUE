# CLAUDE.md

RumbleBall: 탑뷰 뱀서라이크(Survivors-like) 게임. UE 5.8, C++ + Blueprint, Windows(Win64) 전용.
레퍼런스는 Megabonk다. 맵 탐색, 상자, 신전, 보스 포탈, 무기·토메·아이템 시스템을 참고하되, 시점은 탑뷰다.
게임 디자인과 규칙은 [README.md](README.md)를 참고한다. 대화와 문서는 한국어로 쓴다.
Megabonk의 고유 명칭, 아트, 텍스트는 가져오지 않고 자체 이름을 쓴다.

## 빌드 · 실행 · 테스트

엔진 경로는 `C:\Program Files\Epic Games\UE_5.8`이다.

```powershell
# 에디터 타깃 빌드 (C++ 변경 후 기본 검증). 에러·경고만 요약하고 전체 로그는 Saved/Logs/ClaudeBuild.log
powershell -File .claude/skills/ue-build/scripts/build.ps1

# Automation 테스트 (이름 필터 기본값: RumbleBall)
powershell -File .claude/skills/ue-test/scripts/run-tests.ps1
```

- 에디터가 열려 있으면 Live Coding 때문에 빌드가 거부된다(스크립트 종료 코드 2). 이 경우 사용자에게 에디터를 닫거나 Live Coding(Ctrl+Alt+F11)을 실행해 달라고 요청한다. 에디터 프로세스를 임의로 종료하지 않는다.
- 헤더, `UPROPERTY`/`UFUNCTION`, 새 클래스 변경은 Live Coding으로 반영되지 않을 수 있다. 에디터를 닫고 풀 빌드한다.
- Windows 패키징은 `/ue-package`로 한다.
- 완료 기준: C++ 변경은 에디터 타깃 빌드 성공까지 확인한다. 빌드하지 못했거나 PIE 확인이 필요하면 그 사실을 그대로 보고한다.

## 스킬 (`.claude/skills/`)

| 스킬 | 언제 쓰나 |
|------|-----------|
| `ue-build` | C++ 변경 후 빌드할 때, 컴파일·링크·UHT 에러를 고칠 때 |
| `ue-new-class` | 새 C++ 클래스나 구조체를 추가할 때 (부모 클래스 선택, 파일 형식) |
| `ue-test` | 게임 로직 테스트를 작성하고 실행할 때 |
| `ue-balance-data` | 밸런스 수치(CSV + DataTable)를 만들거나 조정할 때 |
| `ue-editor-steps` | BP, 레벨, 위젯, 입력 에셋처럼 사용자가 에디터에서 해야 할 작업을 안내할 때 |
| `ue-package` | Windows 패키징할 때. 사용자가 `/ue-package`로 직접 호출한다 |

## 코드 구조

- 모듈은 `Source/RumbleBallUE/` 하나(Runtime)다. 로그 카테고리는 `LogRumbleBallUE`([RumbleBallUE.h](Source/RumbleBallUE/RumbleBallUE.h)).
- 신규 게임 코드는 모듈 루트 아래 기능별 폴더에 둔다: `Core/`(GameMode, 런·스테이지 타이머), `Player/`, `Enemy/`, `Weapon/`, `Progression/`(경험치, 레벨업, 토메, 아이템), `Stage/`(보스 포탈, 상자, 신전), `UI/`, `Tests/`.
- 하위 폴더 헤더는 모듈 루트 기준 경로로 include한다(`#include "Enemy/RBEnemy.h"`).
- 신규 게임 에셋은 `Content/RumbleBall/` 아래에 둔다.
- 템플릿 코드:
  - `RumbleBallUECharacter/GameMode/PlayerController`와 `Variant_Strategy`는 이 게임과 무관하다.
  - `Variant_TwinStick`은 탑뷰 이동, 대시, 스포너, 투사체, 픽업 구현의 참고용이다. 단, 적 1마리당 `ACharacter`+`AIController`+StateTree 구조라 `NPCCap=20` 규모용이다. 대량 적 구현에 그대로 쓰지 않는다.
  - 템플릿 파일은 수정하지 않는다. 필요한 부분은 `RB` 클래스로 새로 작성한다. 템플릿 삭제는 사용자 승인 후에만 한다.
- 현재 기본 맵과 게임모드는 템플릿 것(`Lvl_TopDown`, `BP_TopDownGameMode`, [DefaultEngine.ini](Config/DefaultEngine.ini))이다.

## 코딩 컨벤션

- Epic C++ Coding Standard를 따른다. 탭 들여쓰기를 쓴다.
- 주석은 한국어로, 템플릿과 같은 `/** */` 형식으로 쓴다. 템플릿 파일의 기존 영어 주석은 그대로 둔다. 한국어가 깨지지 않도록 소스 파일은 UTF-8로 저장한다.
- 신규 클래스 접두어는 `RB`다: `ARBEnemy`, `URBWeaponData`, `FRBUpgradeRow`.
- UObject 멤버 포인터는 `UPROPERTY()` + `TObjectPtr<>`로 둔다. 헤더에는 전방 선언만 하고 include는 `.cpp`에 한다.
- 튜닝 값은 `UPROPERTY(EditAnywhere, Category="...", meta=(ClampMin=..., Units="..."))`로 노출한다(템플릿 패턴).
- C++과 Blueprint 분담:
  - C++: 로직, 상태, 데이터 타입을 담당한다.
  - Blueprint: C++를 상속해 메시, 이펙트, 사운드를 지정하고 연출(`BlueprintImplementableEvent`)을 맡는다.
  - UI: C++ `UUserWidget` 베이스 + UMG 레이아웃으로 만든다(`TwinStickUI` 패턴).

## 데이터 주도 밸런스

- 무기, 토메, 아이템, 적, 웨이브, 상자 가격 수치는 C++에 하드코딩하지 않고 `DataTable` 또는 `UPrimaryDataAsset`에 둔다.
- 밸런스 테이블의 원본은 텍스트 CSV(`Design/Balance/*.csv`)로 두고 DataTable로 reimport한다. CSV는 Claude가 직접 수정할 수 있다.

## 뱀서라이크 성능 규칙

목표는 적 500마리 동시 등장 시 60 FPS(Win64 Development 기준)다. 측정은 `stat unit`, `stat game`, Unreal Insights로 한다.

- **적**: 대량 적에 `CharacterMovementComponent`, 적별 AIController, StateTree/BehaviorTree를 쓰지 않는다.
  - 경량 Pawn/Actor로 만들고, 매니저가 플레이어 추적과 분리 이동을 일괄 처리한다.
  - 적끼리의 물리 충돌은 끈다.
- **풀링**: 적, 투사체, 경험치 보석, 데미지 숫자는 풀에서 재사용한다. 전투 중 `SpawnActor`/`Destroy`를 반복하지 않는다.
- **Tick**: 기본값은 `PrimaryActorTick.bCanEverTick = false`다. 보석과 픽업은 개별 Tick 대신 매니저 한 곳에서 처리한다.
- **조회**: 게임 루프에서 `GetAllActorsOfClass` 같은 전역 탐색을 반복하지 않는다. 결과를 캐시하거나 매니저가 목록을 관리한다.
- **렌더**: 적 메시는 ISM/HISM 또는 저폴리 + 단순 머티리얼로 하고, 이펙트는 Niagara로 한다. 수천 마리 규모가 필요해지면 Mass를 검토한다.

## 테스트

- 순수 로직(경험치 곡선, 레벨업 후보 추첨, 아이템 등급 추첨, 상자 가격, 데미지 계산, 웨이브 테이블)은 Automation 테스트를 최소 1개 남긴다.
- 위치는 `Source/RumbleBallUE/Tests/`, 이름은 `RumbleBall.<영역>.<케이스>`로 하고, `#if WITH_DEV_AUTOMATION_TESTS`로 감싼다.
- 로직은 Actor에서 분리한 순수 함수나 구조체로 만들어 테스트 가능하게 둔다.

## 에셋 · Git

- `.uasset`/`.umap`은 바이너리(Git LFS)라 직접 편집할 수 없다. Blueprint, 레벨, 위젯 작업은 사용자에게 에디터 조작 단계로 안내한다.
- 에셋을 파일시스템에서 이동, 이름 변경, 삭제하지 않는다(참조가 깨진다). 에디터에서 처리하고 Fix Up Redirectors를 실행한다.
- `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`는 생성물이므로 수정하거나 커밋하지 않는다.
- 커밋 메시지는 `feat:`/`fix:`/`refactor:`/`docs:`/`chore:` + 한국어 요약으로 쓴다(기존 히스토리 형식).
