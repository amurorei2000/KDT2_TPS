// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RadioActor.generated.h"

UCLASS()
class MYTPS_API ARadioActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ARadioActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category="MySettings|Components")
	class USceneComponent* rootComp;

	UPROPERTY(VisibleAnywhere, Category="MySettings|Components")
	class UAudioComponent* soundComp;

};
