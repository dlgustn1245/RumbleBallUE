---
name: ue-editor-steps
description: Claude가 직접 편집할 수 없는 언리얼 에디터 작업(블루프린트, 레벨, UMG 위젯, Enhanced Input 에셋, 머티리얼, Niagara, DataTable import)을 사용자가 그대로 따라 할 수 있는 단계로 안내한다. 기능 구현이 .uasset/.umap 변경 없이는 끝나지 않을 때, 또는 사용자가 "에디터에서 어떻게 해?", "BP 만들어줘", "레벨에 배치", "입력 추가"라고 할 때 쓴다.
---

# 에디터 작업 안내

`.uasset`과 `.umap`은 바이너리(LFS)라 Claude가 편집할 수 없다. 이런 작업은 사용자가 그대로 따라 할 수 있는 단계로 넘긴다. 단계가 모호하면 사용자는 엉뚱한 메뉴에서 시간을 버리고, 결과 확인도 어려워진다.

## 먼저: 텍스트로 처리할 수 있는지 본다

에디터 작업처럼 보여도 텍스트 파일이면 Claude가 직접 고친다. 단, 에디터가 열려 있으면 에디터가 덮어쓰므로, 사용자에게 에디터를 닫아 달라고 한 뒤에 고친다.

| 작업 | 파일 |
|------|------|
| 기본 맵, 기본 게임모드 | `Config/DefaultEngine.ini`(`GameDefaultMap`, `GlobalDefaultGameMode`) |
| 프로젝트 이름, 게임 설정 | `Config/DefaultGame.ini` |
| 콘솔 변수, 렌더링 설정 | `Config/DefaultEngine.ini` |
| 밸런스 수치 | `Design/Balance/*.csv` (`ue-balance-data`) |

## 안내 형식

```markdown
### 에디터 작업: <무엇을 하는지 한 줄>

**전제**: <빌드 완료, 에디터 재시작 필요 여부 등>

1. Content Browser에서 `Content/RumbleBall/Enemy` 폴더를 연다.
2. 빈 곳 우클릭 → **Blueprint Class** → All Classes에서 `RBEnemy` 검색 → 선택.
3. 이름을 `BP_RBEnemy_Slime`으로 정한다.
4. 더블클릭으로 연다 → Components의 **Mesh** 선택 → Details의 **Static Mesh**에 `SM_Slime` 지정.
5. **Compile** → **Save**.

**확인**: PIE(Alt+P)로 실행해 <눈으로 확인할 결과>가 보이는지 확인한다.
**알려줄 것**: <결과가 다를 때 Claude에게 전달할 정보, 예: Output Log의 빨간 줄>
```

형식 규칙:
- 메뉴, 버튼, 프로퍼티 이름은 에디터 영문 UI 표기를 **굵게** 쓴다. 사용자 에디터가 한국어 UI면 대응하는 한국어를 괄호로 함께 적는다.
- 한 단계에는 한 가지 조작만 넣는다. 에셋 경로와 이름은 `코드`로 쓴다.
- 마지막에는 항상 **확인** 방법을 적는다. 사용자가 성공 여부를 스스로 판단할 수 있어야 한다.

## 에셋 이름 규칙

에셋은 `Content/RumbleBall/<기능>/` 아래에 둔다.

| 접두어 | 에셋 종류 |
|--------|-----------|
| `BP_` | 블루프린트 |
| `WBP_` | 위젯 블루프린트 |
| `DT_` | DataTable |
| `DA_` | DataAsset |
| `IA_` | Input Action |
| `IMC_` | Input Mapping Context |
| `M_` | 머티리얼 |
| `MI_` | 머티리얼 인스턴스 |
| `NS_` | Niagara 시스템 |
| `SM_` | Static Mesh |
| `SK_` | Skeletal Mesh |
| `T_` | 텍스처 |
| `S_` | 사운드 |
| `L_` | 레벨 |

## 자주 쓰는 작업

- **C++ 클래스의 BP 자식**: 위 예시대로 만든다. `UCLASS(abstract)` 클래스는 BP 자식을 만들어야 쓸 수 있다.
- **입력 추가**:
  1. `IA_` 에셋을 만들고 Value Type을 정한다. 이동은 **Axis2D**, 대시와 상호작용은 **Digital(bool)**이다.
  2. `IMC_`에 키를 매핑한다. WASD 이동은 **Swizzle Input Axis Values**와 **Negate** 모디파이어로 구성한다.
  3. 캐릭터 BP의 Details에서 C++ `UInputAction*` 프로퍼티에 그 `IA_`를 지정한다.
- **위젯**:
  1. `WBP_`의 부모 클래스를 C++ `URB...Widget`으로 바꾼다(**File → Reparent Blueprint**).
  2. C++의 `meta=(BindWidget)` 이름과 똑같은 이름으로 위젯을 배치한다. 이름이 다르면 컴파일 에러가 난다.
- **레벨에서 게임모드 지정**: **World Settings → GameMode Override**에서 지정한다. 기본 게임모드를 프로젝트 전체에 적용하려면 ini를 고친다(위 표).
- **에셋 이동·이름 변경**: 반드시 에디터 안에서 한다. 그다음 폴더 우클릭 → **Fix Up Redirectors**를 실행한다. 파일 탐색기에서 옮기면 참조가 깨진다.

## 안내한 뒤

사용자가 결과를 알려주면 이어서 진행한다. 사용자가 단계를 마쳤다고 말하기 전에는 그 단계에 의존하는 작업(PIE 동작 확인 등)을 완료로 보고하지 않는다.
