// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"


UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	IDLE		UMETA(DisplayName="Base State"),
	MOVE		UMETA(DisplayName = "Walk State"),
	ATTACK		UMETA(DisplayName = "Attack State"),
	ATTACKDELAY	UMETA(DisplayName = "Attack Delay State"),
	RETURN		UMETA(DisplayName = "Return State"),
	DAMAGED		UMETA(DisplayName = "Damaged State"),
	DIE			UMETA(DisplayName = "Dead State"),
};


UCLASS()
class MYTPS_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemy();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, Category = "MySettings")
	EEnemyState enemyState = EEnemyState::IDLE;
	
private:
	UPROPERTY()
	class AActor* target;
	
	float currentTime = 0;
	
	void Idle(float deltaSeconds);
	void MoveToTarget();
	void Attack();
	void AttackDelay();
	void ReturnHome();
	void OnDamaged();
	void Die();
};
