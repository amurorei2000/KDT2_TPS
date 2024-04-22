// Fill out your copyright notice in the Description page of Project Settings.


#include "MoveComponent.h"
#include "TPSPlayer.h"
#include "Camera/CameraComponent.h"
#include "PlayerAnimInstance.h"
#include "EnhancedInputComponent.h"


UMoveComponent::UMoveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


void UMoveComponent::BeginPlay()
{
	Super::BeginPlay();

	// 부모인 플레이어 클래스를 캐싱한다.
	player = GetOwner<ATPSPlayer>();
}


void UMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UMoveComponent::SetupPlayerInputComponent(class UEnhancedInputComponent* enhancedInputComponent)
{
	enhancedInputComponent->BindAction(ia_move, ETriggerEvent::Triggered, this, &UMoveComponent::PlayerMove);
	enhancedInputComponent->BindAction(ia_move, ETriggerEvent::Completed, this, &UMoveComponent::PlayerMove);
	enhancedInputComponent->BindAction(ia_rotate, ETriggerEvent::Triggered, this, &UMoveComponent::PlayerRotate);
	enhancedInputComponent->BindAction(ia_jump, ETriggerEvent::Started, this, &UMoveComponent::PlayerJump);
	enhancedInputComponent->BindAction(ia_jump, ETriggerEvent::Completed, this, &UMoveComponent::PlayerJumpEnd);
}

void UMoveComponent::PlayerMove(const FInputActionValue& value)
{
	FVector2D inputValue = value.Get<FVector2D>();
	moveDirection = FVector(inputValue.Y, inputValue.X, 0);

	// p = p0 + vt
	// v = v0 + at
	//SetActorLocation(GetActorLocation() + moveDirection * 600 * DeltaTime);

	// UCharacterMovementComponent의 함수를 이용해서 이동하는 방식
	// 입력 방향 벡터를 자신의 회전 값 기준으로 변환한다.(world -> local)
	//FVector localMoveDirection = GetTransform().TransformVector(moveDirection);
	FVector localMoveDirection = player->cameraComp->GetComponentTransform().TransformVector(moveDirection);

	//FVector forward = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::X);
	//FVector right = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Y);
	//FVector localMoveDirection = GetActorForwardVector() * moveDirection.X + GetActorRightVector() * moveDirection.Y;
	player->AddMovementInput(localMoveDirection.GetSafeNormal());

	// 애니메이션 인스턴스에 있는 moveDirection 변수에 현재 입력 값을 전달한다.
	if (player->GetPlayerAnim() != nullptr)
	{
		player->GetPlayerAnim()->moveDirection = moveDirection;
	}
}


void UMoveComponent::PlayerRotate(const FInputActionValue& value)
{
	FVector2D inputValue = value.Get<FVector2D>();
	deltaRotation = FRotator(inputValue.Y, inputValue.X, 0);
	//UE_LOG(LogTemp, Warning, TEXT("(%.2f, %.2f)"), inputValue.X, inputValue.Y);

	//SetActorRotation(GetActorRotation() + deltaRotation);
	player->AddControllerYawInput(deltaRotation.Yaw * mouseSensibility);
	player->AddControllerPitchInput(deltaRotation.Pitch * mouseSensibility);
}

void UMoveComponent::PlayerJump(const FInputActionValue& value)
{
	player->Jump();
}

void UMoveComponent::PlayerJumpEnd(const FInputActionValue& value)
{
	player->StopJumping();
}