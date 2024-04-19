// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"
#include "TPSPlayer.h"



void UEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	enemy = Cast<AEnemy>(GetOwningActor());
	if (enemy != nullptr)
	{
		idleNumber = enemy->SelectIdleAnimation();
		UE_LOG(LogTemp, Warning, TEXT("AnimBP idle number:  %d"), idleNumber);
	}
}

void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (enemy != nullptr)
	{
		currentState = enemy->enemyState;
	}
}

void UEnemyAnimInstance::AnimNotify_Kick()
{
	ATPSPlayer* player = Cast<ATPSPlayer>(enemy->GetCurrentTarget());

	if (player != nullptr)
	{
		if (player->tpsPlayerState == EPlayerState::PLAYING)
		{
			player->OnDamaged(3, enemy);
			UE_LOG(LogTemp, Warning, TEXT("Attack Player!"));
		}
		else if (player->tpsPlayerState == EPlayerState::DEATH)
		{
			enemy->RemoveTarget();
			enemy->enemyState = EEnemyState::RETURN;
		}
	}
}

void UEnemyAnimInstance::AnimNotify_Destroy()
{
	if (enemy != nullptr && !GetWorld()->GetTimerManager().IsTimerActive(deathTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(deathTimer, FTimerDelegate::CreateLambda([&]() {
			enemy->Destroy();
			}), 3.0f, false);
	}
}
