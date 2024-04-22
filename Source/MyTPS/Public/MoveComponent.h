// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "MoveComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYTPS_API UMoveComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMoveComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category="MySettings", meta = (UIMin="0.01", UIMax="1.99", ClampMin="0.01", ClampMax="1.99"))
	float mouseSensibility = 0.2f;

	UPROPERTY(EditAnywhere, Category="MySettings")
	class UInputAction* ia_move;

	UPROPERTY(EditAnywhere, Category="MySettings")
	class UInputAction* ia_rotate;

	UPROPERTY(EditAnywhere, Category="MySettings")
	class UInputAction* ia_jump;

	void SetupPlayerInputComponent(class UEnhancedInputComponent* enhancedInputComponent);


private:
	FVector moveDirection;
	FRotator deltaRotation;

	UPROPERTY()
	class ATPSPlayer* player;


	UFUNCTION()
	void PlayerMove(const FInputActionValue& value);

	UFUNCTION()
	void PlayerRotate(const FInputActionValue& value);

	UFUNCTION()
	void PlayerJump(const FInputActionValue& value);

	UFUNCTION()
	void PlayerJumpEnd(const FInputActionValue& value);

};
