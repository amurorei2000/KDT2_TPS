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

	UPROPERTY(EditAnywhere, Category="MySettings|Inputs")
	class UInputMappingContext* imc_tpsKeyMap;

	UPROPERTY(EditAnywhere, Category="MySettings|Inputs")
	class UInputAction* ia_move;

	UPROPERTY(EditAnywhere, Category="MySettings|Inputs")
	class UInputAction* ia_rotate;

	UPROPERTY(EditAnywhere, Category="MySettings|Options", meta = (UIMin="0.01", UIMax="1.99", ClampMin="0.01", ClampMax="1.99"))
	float mouseSensibility = 1.0f;

private:
	FVector moveDirection;
	FRotator deltaRotation;
	FVector camPosition = FVector(-500, 0, 60);
	FVector previousCamLoc;

	UFUNCTION()
	void PlayerMove(const FInputActionValue& value);

	UFUNCTION()
	void PlayerRotate(const FInputActionValue& value);

	void CheckObstacles();
	void SetCameraLag(float deltaTime, float traceSpeed);
};
