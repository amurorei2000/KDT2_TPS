// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FSMComponent.generated.h"


UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	IDLE,		//UMETA(DisplayName="Base State"),
	PATROL,
	MOVE,		//UMETA(DisplayName = "Walk State"),
	ATTACK,		//UMETA(DisplayName = "Attack State"),
	ATTACKDELAY,	//UMETA(DisplayName = "Attack Delay State"),
	RETURN,		//UMETA(DisplayName = "Return State"),
	DAMAGED,		//UMETA(DisplayName = "Damaged State"),
	DAMAGED_BOMB,
	DIE			//UMETA(DisplayName = "Dead State"),
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYTPS_API UFSMComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFSMComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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

	FVector targetLoc;

	UFUNCTION()
	FORCEINLINE AActor* GetCurrentTarget() { return target; };

	UFUNCTION()
	FORCEINLINE void RemoveTarget() { target = nullptr; };

	void SetNewTarget(AActor* newActor);
	int32 SelectIdleAnimation();

private:
	UPROPERTY()
	class AEnemy* enemy;

	UPROPERTY()
	class AActor* target;
	
	float currentTime = 0;
	FVector originLocation;
	FRotator originRotation;
	class AAIController* aiCon;


	void Idle(float deltaSeconds);
	void MoveToTarget(float deltaSeconds);
	void Attack();
	void AttackDelay(float deltaSeconds);
	void ReturnHome(float deltaSeconds);
	void DamageProcess(float deltaSeconds);
	
};
