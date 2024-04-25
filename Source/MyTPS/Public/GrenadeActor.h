// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrenadeActor.generated.h"

UCLASS()
class MYTPS_API AGrenadeActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AGrenadeActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category="MySettings|Components")
	class USphereComponent* sphereComp;

	UPROPERTY(VisibleAnywhere, Category="MySettings|Components")
	class UStaticMeshComponent* meshComp;

	UPROPERTY(VisibleAnywhere, Category="MySettings|Components")
	class UNiagaraComponent* effectComp;

	UPROPERTY(EditAnywhere, Category="MySettings|Variables")
	float damageRadius = 500;

	UPROPERTY(EditAnywhere, Category="MySettings|Variables")
	int32 damagePower = 10;

	UPROPERTY(EditAnywhere, Category="MySettings|Variables")
	float UpForce = 10000;

	UPROPERTY(EditAnywhere, Category="MySettings|Variables")
	TEnumAsByte<ECollisionChannel> affectType;

	UPROPERTY(EditAnywhere, Category="MySettings|Variables")
	class UNiagaraSystem* bombFx;

	void Throw(const FVector& throwDir, float throwPower);

private:
	UFUNCTION()
	void BombAction(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

};
