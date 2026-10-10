---
name: ue-new-class
description: RumbleBallUE에 새 C++ 클래스(Actor, Component, Subsystem, UObject, DataAsset, UUserWidget, USTRUCT, UENUM)를 프로젝트 컨벤션대로 만든다. 적, 무기, 투사체, 경험치 보석, 상자, 신전, 매니저, HUD 같은 게임 기능을 구현하다가 새 클래스나 파일이 필요해지면 이 스킬을 쓴다.
argument-hint: <클래스 이름> [부모 클래스] [폴더]
---

# 새 C++ 클래스 만들기

## 1. 만들기 전에 확인

- 같은 역할의 클래스가 이미 있는지 `Source/RumbleBallUE`를 Grep한다. 있으면 그 클래스를 확장한다.
  - 플레이어 입력·조준은 `ARumbleBallUEPlayerController`, 플레이어 이동 동작은 `ARumbleBallUECharacter`, 게임 규칙은 `ARumbleBallUEGameMode`다. 이런 기능은 새 클래스를 만들지 말고 이 클래스들에 추가한다.
- `Variant_TwinStick`에 비슷한 구현(스포너, 투사체, 픽업, 대시)이 있으면 구조를 참고한다. 템플릿 파일 자체는 고치지 않는다.

## 2. 이름 · 부모 · 위치

이름은 `RB` + 기능 이름이고, 그 앞에 UE 타입 접두어를 붙인다: `A`(Actor), `U`(UObject/Component/Widget/Subsystem), `F`(struct), `E`(enum), `I`(interface).
파일 이름에는 타입 접두어를 뺀다(`ARBEnemy` → `RBEnemy.h`/`RBEnemy.cpp`). 파일은 기능 폴더(`Core/`, `Enemy/`, `Weapon/`, `Progression/`, `Stage/`, `UI/`)에 둔다.

부모는 수백 개씩 생기는지가 기준이다. 뱀서라이크는 대량 객체가 성능을 좌우한다.

| 용도 | 부모 | 이유 |
|------|------|------|
| 대량 스폰되는 적, 투사체, 경험치 보석 | `AActor` + 최소 컴포넌트 | `ACharacter`/`CharacterMovement`/AIController는 개당 비용이 커서 500마리에서 프레임이 무너진다 |
| 플레이어 | 새로 만들지 않음: 기존 `ARumbleBallUECharacter`(`ACharacter`) 확장 | 1개뿐이라 CharacterMovement의 편의 기능이 이득이다 |
| 적 일괄 이동, 오브젝트 풀, 스폰 관리 | `UTickableWorldSubsystem` | 레벨에 배치하지 않아도 월드마다 자동 생성되고, Tick 한 번으로 전체를 처리한다 |
| 여러 액터가 공유하는 기능(체력, 자석 반경) | `UActorComponent` | 상속 대신 조합 |
| 무기·캐릭터 정의 같은 정적 데이터 묶음 | `UPrimaryDataAsset` | 에디터에서 편집하고, 메시나 이펙트 참조를 담을 수 있다 |
| 표 형태 밸런스 수치 | `FTableRowBase` 구조체 | `ue-balance-data` 스킬로 넘긴다 |
| HUD, 레벨업 선택창 | `UUserWidget` | 레이아웃은 UMG, 로직은 C++ |

## 3. 코드 형식

템플릿과 같은 형식(탭, `/** */`)을 쓰되, 주석은 한국어로 쓰고 파일은 UTF-8로 저장한다.

```cpp
// Enemy/RBEnemy.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RBEnemy.generated.h"

class UStaticMeshComponent;

/**
 *  플레이어를 추적하는 기본 적.
 *  이동은 URBEnemySubsystem이 일괄 처리하고, 이 클래스는 체력과 피격만 담당한다.
 */
UCLASS(abstract)
class ARBEnemy : public AActor
{
	GENERATED_BODY()

	/** 적 메시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> Mesh;

protected:

	/** 최대 체력 */
	UPROPERTY(EditAnywhere, Category="Enemy", meta=(ClampMin=1))
	float MaxHealth = 10.0f;

public:

	/** 생성자 */
	ARBEnemy();

protected:

	/** 피격 연출용 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category="Enemy", meta=(DisplayName="Damaged"))
	void BP_Damaged();
};
```

```cpp
// Enemy/RBEnemy.cpp
#include "Enemy/RBEnemy.h"
#include "Components/StaticMeshComponent.h"

ARBEnemy::ARBEnemy()
{
	// 대량 스폰되는 액터라 Tick을 끈다. 이동은 서브시스템이 처리한다.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	// 적끼리의 물리 충돌은 비용이 크므로 끈다
	Mesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}
```

이 형식에 담긴 규칙:
- **include**: 헤더에는 전방 선언만 하고, include는 `.cpp`에서 모듈 루트 기준 경로(`"Enemy/RBEnemy.h"`)로 한다.
- **UObject 멤버**: `UPROPERTY()` + `TObjectPtr<>`로 둔다. `UPROPERTY`가 없으면 GC가 참조를 모른다.
- **튜닝 값**: `EditAnywhere` + `ClampMin`/`Units`로 노출한다. 밸런스 표에 들어갈 수치라면 DataTable로 보낸다.
- **`UCLASS(abstract)`**: 메시나 이펙트를 블루프린트 자식에서 지정해야 하는 클래스에 붙인다(템플릿 패턴). 이런 클래스는 C++에서 `TSubclassOf<>`로 BP 자식을 받아 스폰한다.
- **연출**: `BlueprintImplementableEvent`로 BP에 넘긴다.
- **export 매크로**: 단일 모듈이라 `RUMBLEBALLUE_API`는 붙이지 않는다(템플릿과 동일).

## 4. 마무리

1. `ue-build` 스킬로 빌드한다.
2. 새 `UCLASS`는 에디터를 다시 열어야 보인다고 사용자에게 알린다.
3. BP 자식, 레벨 배치, 입력 연결이 필요하면 `ue-editor-steps` 스킬로 에디터 작업 단계를 안내한다.
4. 분기나 수식이 있는 로직이 들어갔다면 `ue-test` 스킬로 테스트를 남긴다.
