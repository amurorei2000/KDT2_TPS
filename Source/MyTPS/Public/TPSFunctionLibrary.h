// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TPSFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class MYTPS_API UTPSFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static TArray<FVector> CalculateThrowPoints(class AActor* baseActor, const FVector& dir, float power, float interval, float simulTime, float gravityZ);
	
};