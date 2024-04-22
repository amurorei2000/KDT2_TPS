// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TPSPlayer.generated.h"

UENUM(BlueprintType)
enum class EPlayerState
{
	READY,
	PLAYING,
	DEATH,
};


UCLASS()
class MYTPS_API ATPSPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	ATPSPlayer();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, Category="MySettings|Components")
	class UCameraComponent* cameraComp;

	UPROPERTY(VisibleAnywhere, Category="MySettings|Components")
	class USpringArmComponent* springArmComp;

	UPROPERTY(VisibleAnywhere, Category="MySettings|Components")
	class UStaticMeshComponent* gunMeshComp;

	UPROPERTY(VisibleAnywhere, Category="MySettings|Components")
	class UWidgetComponent* floatingWidgetComp;

	// 사용자 컴포넌트들
	UPROPERTY(VisibleAnywhere, Category="MySettings|Components")
	class UMoveComponent* moveComp;

	UPROPERTY(VisibleAnywhere, Category="MySettings|Components")
	class UWeaponComponent* weaponComp;

	UPROPERTY(EditAnywhere, Category="MySettings|Inputs")
	class UInputMappingContext* imc_tpsKeyMap;

	UPROPERTY(EditAnywhere, Category="MySettings|Variables")
	TArray<class UStaticMesh*> gunTypes;

	UPROPERTY(EditAnywhere, Category="MySettings|Variables")
	TArray<FVector> gunOffset;

	UPROPERTY(EditAnywhere, Category="MySettings|Variables")
	TSubclassOf<class UCameraShakeBase> playerHitShake_bp;

	UPROPERTY(EditAnywhere, Category="MySettings|Variables")
	EPlayerState tpsPlayerState;

	UPROPERTY()
	class AWeaponActor* attachedWeapon;

	UPROPERTY(EditAnywhere, Category="MySettings|Animations")
	class UAnimMontage* hitMotage;

	UPROPERTY(EditAnywhere, Category="MySettings|Variables")
	int32 maxHP = 50;
	

	void SetGunAnimType(bool sniper);
	
	FORCEINLINE int32 GetCurrentHP() { return currentHP; };
	FORCEINLINE class UPlayerAnimInstance* GetPlayerAnim() { return playerAnim; };

	UFUNCTION()
	void OnDamaged(int32 dmg, class AEnemy* attacker);

private:
	FVector camPosition = FVector(-500, 0, 60);
	FVector previousCamLoc;
	
	int32 currentHP = 0;
	class UEnemyHealthWidget* playerHealthWidget;

	UPROPERTY()
	class APlayerController* pc;

	UPROPERTY()
	class UPlayerAnimInstance* playerAnim;

	UPROPERTY()
	class ATPSMainGameModeBase* gm;
	

	void CheckObstacles();
	void SetCameraLag(float deltaTime, float traceSpeed);
	
};
