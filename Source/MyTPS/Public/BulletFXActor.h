// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletFXActor.generated.h"

UCLASS()
class MYTPS_API ABulletFXActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ABulletFXActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category="MySettings")
	class USceneComponent* rootComp;

	UPROPERTY(VisibleAnywhere, Category="MySettings")
	//class UParticleSystemComponent* particleComp;
	class UNiagaraComponent* particleComp;

	void PlayFX();
};
