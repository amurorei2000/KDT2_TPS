// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MYTPS_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MySettings")
	FVector moveDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MySettings")
	bool bIsJump = false;

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UPROPERTY()
	class ATPSPlayer* player;

	UPROPERTY()
	class UCharacterMovementComponent* playerMovementComp;
};
