---
name: ue-balance-data
description: 무기, 토메, 아이템, 적, 웨이브, 상자 가격, 경험치 곡선 같은 밸런스 수치를 DataTable(FTableRowBase 구조체 + Design/Balance CSV)로 만들거나 수정한다. "데미지 올려줘", "웨이브 조정", "밸런스 패치", "CSV", "DataTable" 요청에 쓴다. 게임 로직에 숫자를 하드코딩하려는 순간에도 이 스킬을 쓴다.
---

# 밸런스 데이터 (DataTable + CSV)

밸런스 수치의 원본은 텍스트 CSV다. `.uasset`인 DataTable은 Claude가 편집할 수 없지만, CSV는 직접 고치고 diff로 리뷰할 수 있다. 에디터의 DataTable은 CSV를 reimport한 결과물이다.

| 무엇 | 위치 |
|------|------|
| 행 구조체(C++) | 기능 폴더, 예: `Source/RumbleBallUE/Weapon/RBWeaponRow.h` |
| 원본 CSV | `Design/Balance/<테이블>.csv`, 예: `Weapons.csv` |
| DataTable 에셋 | `Content/RumbleBall/Data/DT_<테이블>` |

## 새 테이블 만들기

### 1. 행 구조체를 만든다

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RBWeaponRow.generated.h"

/** 무기 레벨별 밸런스 수치. 행 이름은 "<무기ID>_<레벨>" (예: Sword_1) */
USTRUCT(BlueprintType)
struct FRBWeaponRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 1회 공격 데미지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Damage = 0.0f;

	/** 공격 간격(초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Units="s"))
	float Cooldown = 1.0f;

	/** 한 번에 발사하는 투사체 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ProjectileCount = 1;
};
```

### 2. CSV를 만든다

```csv
Name,Damage,Cooldown,ProjectileCount
Sword_1,10,1.2,1
Sword_2,14,1.1,1
```

### 3. 빌드하고 import한다

`ue-build`로 빌드한 뒤, 에디터 작업을 `ue-editor-steps` 형식으로 안내한다.
1. 에디터를 다시 연다.
2. CSV를 `Content/RumbleBall/Data`로 드래그한다.
3. Row Type에서 `RBWeaponRow`를 고른다.
4. 에셋 이름을 `DT_Weapons`로 정한다.

## CSV 규칙

- **첫 열**은 행 이름이다(헤더 `Name`). 행 이름은 중복될 수 없고, 코드에서 `FName`으로 조회하므로 영문·숫자·`_`만 쓴다.
- **나머지 헤더**는 구조체의 `UPROPERTY` 이름과 정확히 같아야 한다. 이름이 다르면 import 때 그 열이 조용히 무시된다.
- **값 형식**:

  | 타입 | 형식 |
  |------|------|
  | float | `1.5` |
  | bool | `True`/`False` |
  | enum | 값 이름(`Rare`) |
  | 에셋 참조 | `/Game/RumbleBall/...` 경로 |
  | 배열 | `"(1,2,3)"` |
  | 구조체 | `"(X=1,Y=2)"` |

  값 안에 쉼표가 있으면 큰따옴표로 감싼다.
- **표시용 이름**(한국어 아이템 이름 등)은 `FText` 열로 둔다. 파일은 UTF-8로 저장한다.
  사용자가 Excel로 편집한다면 "CSV UTF-8"로 저장해야 한다. 그냥 CSV로 저장하면 CP949로 저장돼 한글이 깨진다.

## 수치 수정

1. CSV만 고친다. 같은 수치를 C++ 기본값에 중복으로 두지 않는다.
2. 고친 뒤 무결성을 확인한다: 행 이름 중복이 없는지, 열 수가 헤더와 같은지, 숫자 칸이 숫자인지.
   ```powershell
   Import-Csv Design/Balance/Weapons.csv | Group-Object Name | Where-Object Count -gt 1
   ```
3. 바뀐 값을 "행 / 열 / 이전 → 이후" 표로 보고한다.
4. 사용자에게 DataTable 우클릭 → **Reimport**를 요청한다. 구조체에 열을 추가했다면 빌드하고 에디터를 다시 연 뒤 reimport한다.

## 코드에서 읽기

```cpp
/** 무기 밸런스 테이블 (DT_Weapons) */
UPROPERTY(EditAnywhere, Category="Data", meta=(RequiredAssetDataTags="RowStructure=/Script/RumbleBallUE.RBWeaponRow"))
TObjectPtr<UDataTable> WeaponTable;

const FRBWeaponRow* Row = WeaponTable->FindRow<FRBWeaponRow>(RowName, TEXT("Weapon level up"));
if (!Row) { UE_LOG(LogRumbleBallUE, Warning, TEXT("무기 행 없음: %s"), *RowName.ToString()); return; }
```

- 무기를 장착하거나 레벨업할 때처럼 값이 바뀌는 시점에 행을 조회해 캐시한다. 적 수백 마리가 매 프레임 `FindRow`를 부르지 않게 한다.
- 행이 없으면 기본값으로 조용히 넘어가지 말고 로그를 남긴다. 오타 난 행 이름은 그래야 바로 드러난다.
