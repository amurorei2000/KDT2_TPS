// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"





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

	UPROPERTY(VisibleAnywhere, Category="MySettings|Components")
	class UWidgetComponent* floatingWidgetComp;

	UPROPERTY(VisibleAnywhere, Category="MySettings|Components")
	class UNavigationInvokerComponent* navInvokerComp;

	UPROPERTY(VisibleAnywhere, Category="MySettings|Components")
	class UFSMComponent* fsmComp;

	UPROPERTY(EditAnywhere, Category = "MySettings")
	int32 maxHP = 30;

	UPROPERTY(EditAnywhere, Category = "MySettings")
	class UAnimMontage* death_montage;

	int32 idleAnimNumber = 1;
	class UMaterialInstanceDynamic* dynamicMat;

	void OnDamaged(int32 dmg, AActor* attacker);
	void HitBomb(int32 dmg, const FVector& attackDir, float maxRadius, float upPower);
	FORCEINLINE class UEnemyAnimInstance* GetEnemyAnim() { return anim; };

private:
	int32 currentHP = 0;
	FVector hitLocation;
	FVector hitDirection;
	FVector bombDir;
	float bombDist;
	float bombUpPower;

	UPROPERTY()
	class UEnemyAnimInstance* anim;
	
	UPROPERTY()
	class UEnemyHealthWidget* healthWidget;

	UPROPERTY()
	class ATPSPlayer* player;

	void Die();
	FRotator BillboardWidgetComponent(class AActor* camActor);
	void BombImpact();
};
