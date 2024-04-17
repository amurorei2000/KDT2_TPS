// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TPSPlayer.generated.h"

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

	UPROPERTY(EditAnywhere, Category="MySettings|Inputs")
	class UInputMappingContext* imc_tpsKeyMap;

	UPROPERTY(EditAnywhere, Category="MySettings|Inputs")
	class UInputAction* ia_move;

	UPROPERTY(EditAnywhere, Category="MySettings|Inputs")
	class UInputAction* ia_rotate;

	UPROPERTY(EditAnywhere, Category="MySettings|Inputs")
	class UInputAction* ia_jump;

	UPROPERTY(EditAnywhere, Category="MySettings|Inputs")
	class UInputAction* ia_fire;

	UPROPERTY(EditAnywhere, Category="MySettings|Inputs")
	class UInputAction* ia_alpha1;

	UPROPERTY(EditAnywhere, Category="MySettings|Inputs")
	class UInputAction* ia_alpha2;
	
	UPROPERTY(EditAnywhere, Category="MySettings|Inputs")
	class UInputAction* ia_aimFocusing;

	UPROPERTY(EditAnywhere, Category="MySettings|Inputs")
	class UInputAction* ia_releaseWeapon;

	UPROPERTY(EditAnywhere, Category="MySettings|Options", meta = (UIMin="0.01", UIMax="1.99", ClampMin="0.01", ClampMax="1.99"))
	float mouseSensibility = 0.2f;

	UPROPERTY(EditAnywhere, Category="MySettings|Variables")
	class ABulletFXActor* bulletFX;

	UPROPERTY(EditAnywhere, Category="MySettings|Variables")
	TArray<class UStaticMesh*> gunTypes;

	UPROPERTY(EditAnywhere, Category="MySettings|Variables")
	TArray<FVector> gunOffset;

	UPROPERTY(EditAnywhere, Category="MySettings|Variables")
	TSubclassOf<class UCameraShakeBase> playerHitShake_bp;

	UPROPERTY()
	class AWeaponActor* attachedWeapon;

	UPROPERTY(EditAnywhere, Category="MySettings|Animations")
	TArray<class UAnimMontage*> fire_montages;

	UPROPERTY(EditAnywhere, Category="MySettings|Animations")
	class UAnimMontage* hitMotage;


	/*UPROPERTY(EditAnywhere, Category="MySettings|Variables")
	TArray<class UAnimationAsset*> anims;*/
	UPROPERTY(EditAnywhere, Category="MySettings|Variables")
	int32 maxHP = 50;

	void SetGunAnimType(bool sniper);
	void SetCurrentWeaponNumber(bool bSniper);
	FORCEINLINE int32 GetCurrentHP() { return currentHP; };

	UFUNCTION()
	void OnDamaged(int32 dmg, class AEnemy* attacker);

private:
	FVector moveDirection;
	FRotator deltaRotation;
	FVector camPosition = FVector(-500, 0, 60);
	FVector previousCamLoc;
	FTimerHandle endFireTimer;
	bool bZoomIn = false;
	float alpha = 0;
	int32 currentWeaponNumber = 0;
	int32 currentHP = 0;
	class UEnemyHealthWidget* playerHealthWidget;

	UPROPERTY()
	class APlayerController* pc;

	UPROPERTY()
	class UPlayerAnimInstance* playerAnim;

	UPROPERTY()
	class ATPSMainGameModeBase* gm;

	UFUNCTION()
	void PlayerMove(const FInputActionValue& value);

	/*UFUNCTION()
	void PlayerMoveStart(const FInputActionValue& value);

	UFUNCTION()
	void PlayerMoveEnd(const FInputActionValue& value);*/

	UFUNCTION()
	void PlayerRotate(const FInputActionValue& value);

	UFUNCTION()
	void PlayerJump(const FInputActionValue& value);

	UFUNCTION()
	void PlayerJumpEnd(const FInputActionValue& value);

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

	void CheckObstacles();
	void SetCameraLag(float deltaTime, float traceSpeed);
	void ChangeGunType(int32 number);

};
