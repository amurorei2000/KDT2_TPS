// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "TPSPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	player = Cast<ATPSPlayer>(GetOwningActor());
	
	if (player != nullptr)
	{
		playerMovementComp = player->GetCharacterMovement();
	}
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (playerMovementComp != nullptr)
	{
		// 캐릭터 무브먼트 컴포넌트의 isFalling() 함수 반환 값을 bIsJump 변수에 저장한다.
		bIsJump = playerMovementComp->IsFalling();
	}
}
