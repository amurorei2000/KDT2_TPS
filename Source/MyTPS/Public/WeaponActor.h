// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponActor.generated.h"

UCLASS()
class MYTPS_API AWeaponActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category="MySettings")
	class UBoxComponent* boxComp;

	UPROPERTY(VisibleAnywhere, Category="MySettings")
	class UStaticMeshComponent* meshComp;

	UPROPERTY(EditAnywhere, Category="MySettings")
	int32 damage = 5;

	UPROPERTY(EditAnywhere, Category="MySettings")
	int32 magazine = 30;

	UPROPERTY(EditAnywhere, Category="MySettings")
	float fireDistance = 1000.0f;

	UPROPERTY(EditAnywhere, Category="MySettings")
	FVector snapOffset;

	UPROPERTY(EditAnywhere, Category="MySettings")
	bool bSniperGun = false;

	void Release();


private:
	UFUNCTION()
	void OnOverlapPlayer(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
