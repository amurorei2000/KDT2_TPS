// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSFunctionLibrary.h"


TArray<FVector> UTPSFunctionLibrary::CalculateThrowPoints(AActor* baseActor, const FVector& dir, float power, float interval, float simulTime, float gravityZ)
{
	TArray<FVector> simulPoints;

	// ÃÑ È½¼ö = ½Ã¹Ä·¹ÀÌ¼Ç ÃÑ ½Ã°£ / ½Ã¹Ä·¹ÀÌ¼Ç °£°Ý
	int32 segment = simulTime / interval;
	FVector startLocation = baseActor->GetActorLocation() + baseActor->GetActorForwardVector() * 100;
	FVector gravityValue = FVector(0, 0, gravityZ);
	float mass = 1;

	for (int32 i = 0; i < segment; i++)
	{
		// p = p0 + vt - 0.5*g*t*t * mass * mass
		float term = interval * i;
		FVector predictLocation = startLocation + dir * power * term + 0.5f * gravityValue * term * term * mass * mass;
		simulPoints.Add(predictLocation);
	}

	return simulPoints;
}