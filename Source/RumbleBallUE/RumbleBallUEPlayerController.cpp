// Copyright Epic Games, Inc. All Rights Reserved.

#include "RumbleBallUEPlayerController.h"
#include "RumbleBallUECharacter.h"
#include "Camera/PlayerCameraManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "RumbleBallUE.h"

ARumbleBallUEPlayerController::ARumbleBallUEPlayerController()
{
	// 마우스 커서로 조준하므로 커서를 보인다
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void ARumbleBallUEPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 로컬 플레이어 컨트롤러만 입력을 받는다
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARumbleBallUEPlayerController::Move);
		EnhancedInputComponent->BindAction(StickAimAction, ETriggerEvent::Triggered, this, &ARumbleBallUEPlayerController::StickAim);

		// 누르고 있어도 한 번만 대시하도록 Started에 바인딩
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &ARumbleBallUEPlayerController::Dash);
	}
	else
	{
		UE_LOG(LogRumbleBallUE, Error, TEXT("'%s' Enhanced Input Component를 찾지 못했습니다. 이 컨트롤러는 Enhanced Input 전용입니다."), *GetNameSafe(this));
	}
}

void ARumbleBallUEPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	UpdateMouseAim();
}

FVector ARumbleBallUEPlayerController::InputToWorld(const FVector2D& Input) const
{
	// 화면 위쪽 = 카메라 전방, 화면 오른쪽 = 카메라 우측 (수평 성분만)
	const double CameraYaw = PlayerCameraManager ? PlayerCameraManager->GetCameraRotation().Yaw : 0.0;
	const FRotationMatrix YawMatrix(FRotator(0.0, CameraYaw, 0.0));
	return YawMatrix.GetUnitAxis(EAxis::X) * Input.Y + YawMatrix.GetUnitAxis(EAxis::Y) * Input.X;
}

void ARumbleBallUEPlayerController::Move(const FInputActionValue& Value)
{
	if (APawn* ControlledPawn = GetPawn())
	{
		// 스틱의 아날로그 크기를 살리기 위해 정규화하지 않고 길이만 1로 자른다
		ControlledPawn->AddMovementInput(InputToWorld(Value.Get<FVector2D>()).GetClampedToMaxSize(1.0f));
	}
}

void ARumbleBallUEPlayerController::StickAim(const FInputActionValue& Value)
{
	const FVector Direction = InputToWorld(Value.Get<FVector2D>()).GetSafeNormal2D();
	if (Direction.IsNearlyZero())
	{
		return;
	}

	SetControlRotation(Direction.Rotation());
	bUsingStickAim = true;
	SetShowMouseCursor(false);
}

void ARumbleBallUEPlayerController::UpdateMouseAim()
{
	// 마우스가 움직이면 스틱 조준에서 마우스 조준으로 돌아온다
	float MouseX = 0.0f;
	float MouseY = 0.0f;
	if (GetMousePosition(MouseX, MouseY) && !FVector2D(MouseX, MouseY).Equals(LastMousePosition, 1.0f))
	{
		LastMousePosition = FVector2D(MouseX, MouseY);
		if (bUsingStickAim)
		{
			bUsingStickAim = false;
			SetShowMouseCursor(true);
		}
	}

	const APawn* ControlledPawn = GetPawn();
	if (bUsingStickAim || !ControlledPawn)
	{
		return;
	}

	// 커서 광선을 캐릭터 높이의 수평면과 교차시킨다. 충돌 채널 설정 없이 어디서든 동작한다.
	FVector RayOrigin;
	FVector RayDirection;
	if (!DeprojectMousePositionToWorld(RayOrigin, RayDirection) || FMath::IsNearlyZero(RayDirection.Z))
	{
		return;
	}

	const FVector PawnLocation = ControlledPawn->GetActorLocation();
	const FVector AimPoint = FMath::LinePlaneIntersection(RayOrigin, RayOrigin + RayDirection, FPlane(PawnLocation, FVector::UpVector));
	const FVector Direction = (AimPoint - PawnLocation).GetSafeNormal2D();

	// 커서가 캐릭터 바로 위에 있으면 방향이 불안정하므로 이전 방향을 유지한다
	if (!Direction.IsNearlyZero())
	{
		SetControlRotation(Direction.Rotation());
	}
}

void ARumbleBallUEPlayerController::Dash()
{
	ARumbleBallUECharacter* PlayerCharacter = Cast<ARumbleBallUECharacter>(GetPawn());
	if (!PlayerCharacter)
	{
		return;
	}

	FVector Direction = PlayerCharacter->GetLastMovementInputVector().GetSafeNormal2D();
	if (Direction.IsNearlyZero())
	{
		Direction = GetControlRotation().Vector().GetSafeNormal2D();
	}

	PlayerCharacter->Dash(Direction);
}
