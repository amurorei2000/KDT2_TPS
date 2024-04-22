// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYTPS_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWeaponComponent();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditAnywhere, Category="MySettings")
	class ABulletFXActor* bulletFX;

	UPROPERTY(EditAnywhere, Category="MySettings")
	TArray<class UAnimMontage*> fire_montages;

	UPROPERTY(EditAnywhere, Category="MySettings")
	FVector throwDir;

	UPROPERTY(EditAnywhere, Category="MySettings")
	float throwPower = 500;

	UPROPERTY(EditAnywhere, Category="MySettings")
	TSubclassOf<class AGrenadeActor> grenade_bp;

	UPROPERTY(EditAnywhere, Category="MySettings")
	TSubclassOf<class ABombDecalActor> bombDecal_bp;

	UPROPERTY(EditAnywhere, Category="MySettings")
	class UInputAction* ia_fire;

	UPROPERTY(EditAnywhere, Category="MySettings")
	class UInputAction* ia_alpha1;

	UPROPERTY(EditAnywhere, Category="MySettings")
	class UInputAction* ia_alpha2;
	
	UPROPERTY(EditAnywhere, Category="MySettings")
	class UInputAction* ia_aimFocusing;

	UPROPERTY(EditAnywhere, Category="MySettings")
	class UInputAction* ia_releaseWeapon;


	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void SetupPlayerInputComponent(class UEnhancedInputComponent* enhancedInputComponent);
	void SetCurrentWeaponNumber(bool bSniper);

private:
	UPROPERTY()
	class ATPSPlayer* player;

	UPROPERTY()
	class ABombDecalActor* bombDecal_inst;
	
	UPROPERTY()
	class ATPSMainGameModeBase* gm;

	FTimerHandle endFireTimer;
	int32 currentWeaponNumber = 0;
	bool bShowLine = false;
	bool bZoomIn = false;
	float alpha = 0;

	UFUNCTION()
	void PlayerFire(const FInputActionValue& value);

	UFUNCTION()
	void PlayerFire2(const FInputActionValue& value);

	UFUNCTION()
	void SetWeapon1(const FInputActionValue& value);

	UFUNCTION()
	void SetWeapon2(const FInputActionValue& value);

	UFUNCTION()
	void SniperGunZoomInOut(const FInputActionValue& value);

	UFUNCTION()
	void ReleaseAction(const FInputActionValue& value);

	UFUNCTION()
	void EndFire();

	void ChangeGunType(int32 number);
};
