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

	UPROPERTY(EditAnywhere, Category = "MySettings")
	float traceSpeed = 750.0f;

	UPROPERTY(EditAnywhere, Category = "MySettings")
	float attackDistance = 170.0f;

	UPROPERTY(EditAnywhere, Category = "MySettings")
	float rotSpeed = 5;

	UPROPERTY(EditAnywhere, Category = "MySettings")
	float attackDelayTime = 1.5f;
	
	UPROPERTY(EditAnywhere, Category = "MySettings")
	float limitDistance = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "MySettings")
	float returnSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "MySettings")
	float sightDistance = 700.0f;

	UPROPERTY(EditAnywhere, Category = "MySettings")
	float sightAngle = 30.0f;

	UPROPERTY(EditAnywhere, Category = "MySettings")
	int32 maxHP = 30;

	void OnDamaged(int32 dmg);

private:
	UPROPERTY()
	class AActor* target;
	
	float currentTime = 0;
	FVector originLocation;
	FRotator originRotation;
	int32 currentHP = 0;
	FVector hitLocation;
	
	void Idle(float deltaSeconds);
	void MoveToTarget(float deltaSeconds);
	void Attack();
	void AttackDelay(float deltaSeconds);
	void ReturnHome(float deltaSeconds);
	void DamageProcess(float deltaSeconds);
	void Die();
};
