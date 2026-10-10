// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RumbleBallUEPlayerController.generated.h"

struct FInputActionValue;
class UInputMappingContext;
class UInputAction;

/**
 *  탑뷰 플레이어 컨트롤러.
 *  WASD(왼쪽 스틱)로 화면 기준 이동하고 Space(A)로 대시한다.
 *  마우스 커서(오른쪽 스틱) 방향을 컨트롤 회전으로 넘겨 캐릭터가 그쪽을 바라보게 한다.
 */
UCLASS(abstract)
class ARumbleBallUEPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** 플레이어 입력 매핑 */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** 이동 입력 (Axis2D, X = 화면 오른쪽, Y = 화면 위쪽) */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	/** 게임패드 조준 입력 (Axis2D, 오른쪽 스틱) */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> StickAimAction;

	/** 대시 입력 */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> DashAction;

	/** true면 오른쪽 스틱 조준 중, false면 마우스 조준 중 */
	bool bUsingStickAim = false;

	/** 마우스 움직임 감지용 직전 커서 위치 */
	FVector2D LastMousePosition = FVector2D::ZeroVector;

public:

	/** 생성자 */
	ARumbleBallUEPlayerController();

protected:

	/** 입력 매핑 등록과 액션 바인딩 */
	virtual void SetupInputComponent() override;

	/** 매 프레임 마우스 조준 갱신 */
	virtual void PlayerTick(float DeltaTime) override;

	/** 이동 입력 처리 */
	void Move(const FInputActionValue& Value);

	/** 오른쪽 스틱 조준 입력 처리 */
	void StickAim(const FInputActionValue& Value);

	/** 대시 입력 처리. 이동 중이면 이동 방향, 서 있으면 조준 방향으로 대시한다 */
	void Dash();

	/** 마우스 커서가 가리키는 지면 쪽으로 조준 방향을 갱신 */
	void UpdateMouseAim();

	/** 화면 기준 2D 입력(X = 오른쪽, Y = 위쪽)을 카메라 yaw 기준 월드 수평 방향으로 변환 */
	FVector InputToWorld(const FVector2D& Input) const;
};
