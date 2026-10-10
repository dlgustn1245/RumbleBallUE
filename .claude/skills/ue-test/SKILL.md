---
name: ue-test
description: RumbleBallUE의 UE Automation 테스트를 작성하고 실행한다. 경험치 곡선, 레벨업 후보 추첨, 아이템 등급 추첨, 상자 가격, 데미지 계산, 웨이브 테이블처럼 분기·반복·수식이 있는 게임 로직을 새로 만들거나 고칠 때는 사용자가 테스트를 요청하지 않아도 이 스킬로 테스트를 남긴다. "테스트 돌려줘", "테스트 추가", "Automation", "RunTests" 요청에도 쓴다.
---

# UE Automation 테스트

## 무엇을 테스트하나

수식, 확률, 테이블 조회처럼 Actor 없이 계산되는 순수 로직을 테스트한다.
로직이 Actor 안에 묶여 있으면 먼저 `F` 구조체나 정적 함수로 분리한다. 예: `FRBExperience::GetRequiredXP(Level)`.
World나 Actor가 필요한 테스트는 만들지 않는다. 느리고 불안정하며, 그런 동작은 PIE에서 사용자가 확인하는 편이 빠르다.

테스트는 로직 하나당 최소 1개, 그 로직이 깨지면 실패하는 가장 작은 케이스로 만든다. 경계값(레벨 1, 최대 레벨, 확률 0%·100%)을 우선한다.

## 파일 규칙

- 위치: `Source/RumbleBallUE/Tests/RB<영역>Tests.cpp` (헤더 없음)
- 테스트 이름: `RumbleBall.<영역>.<케이스>`. 실행 필터가 `RumbleBall` 접두어에 의존한다.
- 전체를 `#if WITH_DEV_AUTOMATION_TESTS`로 감싼다. Shipping 빌드에서 제외하기 위해서다.

```cpp
#include "Misc/AutomationTest.h"
#include "Progression/RBExperience.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRBExperienceCurveTest, "RumbleBall.Progression.ExperienceCurve",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FRBExperienceCurveTest::RunTest(const FString& Parameters)
{
	// 1레벨 필요 경험치는 테이블 시작값과 같아야 한다
	TestEqual(TEXT("Lv1 필요 경험치"), FRBExperience::GetRequiredXP(1), 5);

	// 필요 경험치는 레벨이 오를수록 줄어들면 안 된다
	for (int32 Level = 1; Level < 50; ++Level)
	{
		TestTrue(FString::Printf(TEXT("Lv%d -> Lv%d 단조 증가"), Level, Level + 1),
			FRBExperience::GetRequiredXP(Level + 1) >= FRBExperience::GetRequiredXP(Level));
	}
	return true;
}

#endif
```

UE 5.5부터 `EAutomationTestFlags`가 enum class로 바뀌었다. 컨텍스트 마스크는 `EAutomationTestFlags_ApplicationContextMask`를 쓴다. 예전 문법인 `EAutomationTestFlags::ApplicationContextMask`는 쓰지 않는다.

확률 로직은 시드를 고정한 `FRandomStream`을 인자로 받게 만들어서 결과를 결정적으로 검증한다.

## 실행

먼저 `ue-build` 스킬로 에디터 타깃을 빌드한다. 테스트는 디스크에 있는 바이너리로 돌기 때문에, 빌드하지 않으면 예전 코드가 테스트된다.

```powershell
powershell -File .claude/skills/ue-test/scripts/run-tests.ps1                                  # RumbleBall.* 전체
powershell -File .claude/skills/ue-test/scripts/run-tests.ps1 -Filter RumbleBall.Progression   # 일부만
```

프로젝트 로딩 때문에 1~3분 걸린다. 결과는 테스트별 `[Success]`/`[Fail]`과 실패 메시지로 요약된다.

| 종료 코드 | 의미 |
|-----------|------|
| 0 | 전부 통과 |
| 1 | 실패한 테스트 있음 |
| 2 | 리포트 없음(필터와 맞는 테스트가 없거나 크래시). `Saved/Logs/ClaudeTests.log` 확인 |

실패하면 테스트를 고쳐서 통과시키지 말고, 로직과 테스트 중 어느 쪽이 기획(README 규칙, 밸런스 CSV)과 다른지 먼저 판단한다.
