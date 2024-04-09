// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "EngineUtils.h"
#include "TPSPlayer.h"


AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	// �⺻ ���¸� IDLE ���·� �ʱ�ȭ�Ѵ�.
	enemyState = EEnemyState::IDLE;

	// ���忡 �ִ� �÷��̾ ã�´�.
	for (TActorIterator<ATPSPlayer> player(GetWorld()); player; ++player)
	{
		target = *player;
	}


}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (enemyState)
	{
	case EEnemyState::IDLE:
		Idle(DeltaTime);
		break;
	case EEnemyState::MOVE:
		MoveToTarget();
		break;
	case EEnemyState::ATTACK:
		Attack();
		break;
	case EEnemyState::ATTACKDELAY:
		AttackDelay();
		break;
	case EEnemyState::RETURN:
		ReturnHome();
		break;
	case EEnemyState::DAMAGED:
		OnDamaged();
		break;
	case EEnemyState::DIE:
		Die();
		break;
	default:
		break;
	}
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::Idle(float deltaSeconds)
{
	//UE_LOG(LogTemp, Warning, TEXT("Idle State"));

	// 5�ʰ� ������ ���¸� MOVE ���·� �����Ѵ�.
	currentTime += deltaSeconds;

	if (currentTime > 5.0f)
	{
		currentTime = 0;
		enemyState = EEnemyState::MOVE;
	}
}

void AEnemy::MoveToTarget()
{
	//UE_LOG(LogTemp, Warning, TEXT("MoveToTarget State"));
	// Ÿ���� �ִٸ� Ÿ���� �Ѿư���.

}

void AEnemy::Attack()
{
	
}

void AEnemy::AttackDelay()
{
	
}

void AEnemy::ReturnHome()
{
	
}

void AEnemy::OnDamaged()
{
	
}

void AEnemy::Die()
{
	
}

