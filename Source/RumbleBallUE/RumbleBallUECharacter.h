// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RumbleBallUECharacter.generated.h"

/**
 *  탑뷰 플레이어 캐릭터.
 *  컨트롤러가 넘겨주는 조준 방향(컨트롤 회전 yaw)을 바라보고, 요청을 받으면 대시한다.
 *  카메라(SpringArm, Camera)는 블루프린트(BP_TopDownCharacter)에서 구성한다.
 */
UCLASS(abstract)
class ARumbleBallUECharacter : public ACharacter
{
	GENERATED_BODY()

protected:

	/** 대시 속도 */
	UPROPERTY(EditAnywhere, Category="Dash", meta=(ClampMin=0, Units="cm/s"))
	float DashSpeed = 2000.0f;

	/** 대시 지속 시간. 이 동안 마찰 없이 대시 속도를 유지한다 */
	UPROPERTY(EditAnywhere, Category="Dash", meta=(ClampMin=0.01, Units="s"))
	float DashDuration = 0.2f;

	/** 대시 재사용 대기 시간 (대시 시작 기준) */
	UPROPERTY(EditAnywhere, Category="Dash", meta=(ClampMin=0, Units="s"))
	float DashCooldown = 1.0f;

private:

	/** 다음 대시가 가능한 게임 시간 */
	double NextDashTime = 0.0;

	/** 대시가 끝나면 되돌릴 이동 마찰 값 */
	float DefaultGroundFriction = 0.0f;
	float DefaultBrakingDeceleration = 0.0f;

	/** 대시 종료 타이머 */
	FTimerHandle DashTimer;

public:

	/** 생성자 */
	ARumbleBallUECharacter();

	/** 주어진 수평 방향으로 대시한다. 재사용 대기 중이면 무시한다 */
	void Dash(const FVector& Direction);

protected:

	/** 대시 후 되돌릴 이동 값 저장 */
	virtual void BeginPlay() override;

	/** 대시를 끝내고 이동 마찰을 되돌린다 */
	void EndDash();
};
