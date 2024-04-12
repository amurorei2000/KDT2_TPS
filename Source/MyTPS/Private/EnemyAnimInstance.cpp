// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"



void UEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	enemy = Cast<AEnemy>(GetOwningActor());
	
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
	UE_LOG(LogTemp, Warning, TEXT("Attack Player!"));
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
