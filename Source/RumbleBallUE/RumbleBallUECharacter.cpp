// Copyright Epic Games, Inc. All Rights Reserved.

#include "RumbleBallUECharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ARumbleBallUECharacter::ARumbleBallUECharacter()
{
	// 조준은 컨트롤러가 처리하므로 캐릭터는 Tick이 필요 없다
	PrimaryActorTick.bCanEverTick = false;

	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// 컨트롤러가 조준 방향을 컨트롤 회전으로 넘겨주므로 yaw만 따라간다
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// 몸은 이동 방향이 아니라 조준 방향을 바라본다.
	// 뱀서라이크는 조작 반응이 즉각적이어야 하므로 가속·감속을 높게 둔다.
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = false;
	Movement->MaxAcceleration = 4000.0f;
	Movement->BrakingDecelerationWalking = 4000.0f;
	Movement->bConstrainToPlane = true;
	Movement->bSnapToPlaneAtStart = true;
}

void ARumbleBallUECharacter::BeginPlay()
{
	Super::BeginPlay();

	// 블루프린트에서 조정한 값을 기준으로 대시 후 복구한다
	DefaultGroundFriction = GetCharacterMovement()->GroundFriction;
	DefaultBrakingDeceleration = GetCharacterMovement()->BrakingDecelerationWalking;
}

void ARumbleBallUECharacter::Dash(const FVector& Direction)
{
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now < NextDashTime || Direction.IsNearlyZero())
	{
		return;
	}
	NextDashTime = Now + DashCooldown;

	// 대시 동안 마찰과 감속을 없애 DashSpeed × DashDuration 거리만큼 미끄러지게 한다
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->GroundFriction = 0.0f;
	Movement->BrakingDecelerationWalking = 0.0f;
	LaunchCharacter(Direction.GetSafeNormal2D() * DashSpeed, true, true);

	GetWorldTimerManager().SetTimer(DashTimer, this, &ARumbleBallUECharacter::EndDash, DashDuration);
}

void ARumbleBallUECharacter::EndDash()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->GroundFriction = DefaultGroundFriction;
	Movement->BrakingDecelerationWalking = DefaultBrakingDeceleration;
}
