// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DecalActor.h"
#include "BombDecalActor.generated.h"

/**
 * 
 */
UCLASS()
class MYTPS_API ABombDecalActor : public ADecalActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category="MySettings")
	float decalRadius = 500;

	// 외형 켜고 끄기
	void SetShowDecal(bool visible);

protected:
	virtual void BeginPlay() override;

};
