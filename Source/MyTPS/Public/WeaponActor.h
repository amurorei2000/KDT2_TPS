// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponActor.generated.h"

USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	int32 damage;
	UPROPERTY(EditAnywhere)
	int32 magazine;
	UPROPERTY(EditAnywhere)
	float fireDistance;
	UPROPERTY(EditAnywhere)
	FVector snapOffset;
	UPROPERTY(EditAnywhere)
	bool bSniperGun;

	// 이니셜라이저를 이용한 생성자 선언
	//FWeaponData(int32 dmg, int32 bullets, float dist, FVector offset, bool bSniper) : damage(dmg), magazine(bullets), fireDistance(dist), snapOffset(offset), bSniperGun(bSniper)
	//{};
};



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
	struct FWeaponData weaponData;

	void Release();


private:
	UFUNCTION()
	void OnOverlapPlayer(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
