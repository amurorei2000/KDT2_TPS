// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSFunctionLibrary.h"


TArray<FVector> UTPSFunctionLibrary::CalculateThrowPoints(AActor* baseActor, const FVector& dir, float power, float interval, float simulTime, float gravityZ)
{
	TArray<FVector> simulPoints;

	// 총 횟수 = 시뮬레이션 총 시간 / 시뮬레이션 간격
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

// 시야각 체크
float UTPSFunctionLibrary::CheckSight(const AActor* myActor, const AActor* otherActor)
{
	FVector forwardVec = myActor->GetActorForwardVector();
	FVector directionVec = (otherActor->GetActorLocation() - myActor->GetActorLocation()).GetSafeNormal();

	float cosTheta = FVector::DotProduct(forwardVec, directionVec);
	float theta_radian = FMath::Acos(cosTheta);
	float theta_degree = FMath::RadiansToDegrees(theta_radian);
	
	return theta_degree;
}

// 주변 액터 검사
TArray<AActor*> UTPSFunctionLibrary::SearchAroundActor(const AActor* myActor, float sightDistance, ECollisionChannel searchObject, UWorld* worldContext)
{
	TArray<AActor*> otherActors;

	TArray<FOverlapResult> hitInfos;
	FCollisionObjectQueryParams objectParams;
	objectParams.AddObjectTypesToQuery(searchObject);
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(myActor);

	bool bCheckResult = worldContext->OverlapMultiByObjectType(hitInfos, myActor->GetActorLocation(), FQuat::Identity, objectParams, FCollisionShape::MakeSphere(sightDistance), queryParams);

	// 오버랩된 데이터마다 전부 돌면서...
	for (const FOverlapResult& hitInfo : hitInfos)
	{
		// 오버랩된 액터를 otherActors 배열에 추가한다.
		otherActors.Add(hitInfo.GetActor());
	}

	return otherActors;
}
