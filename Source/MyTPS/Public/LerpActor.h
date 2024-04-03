// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LerpActor.generated.h"

UCLASS()
class MYTPS_API ALerpActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ALerpActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	class USceneComponent* rootComp;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* mesh1;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* mesh2;

	UPROPERTY(EditAnywhere, meta=(ClampMin=0.0f, ClampMax=2.0f, UIMin=0.0f, UIMax=2.0f))
	float alpha = 0;

	UPROPERTY(EditAnywhere)
	TArray<FVector> positions;
};
