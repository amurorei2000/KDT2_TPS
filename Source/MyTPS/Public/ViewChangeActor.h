// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ViewChangeActor.generated.h"

UCLASS()
class MYTPS_API AViewChangeActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AViewChangeActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	class USceneComponent* rootComp;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* triggerBox;

	UPROPERTY(EditAnywhere)
	class AActor* cameraActor;

private:
	UPROPERTY()
	class ATPSPlayer* player;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
