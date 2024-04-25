// Fill out your copyright notice in the Description page of Project Settings.


#include "MoveComponent.h"
#include "TPSPlayer.h"
#include "Camera/CameraComponent.h"
#include "PlayerAnimInstance.h"
#include "EnhancedInputComponent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"

UMoveComponent::UMoveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UMoveComponent::BeginPlay()
{
	Super::BeginPlay();

	// 부모인 플레이어 클래스를 캐싱한다.
	player = GetOwner<ATPSPlayer>();

	pc = player->GetController<APlayerController>();
	if (pc != nullptr)
	{
		pc->SetShowMouseCursor(player->inputType == EInputType::RTS_INPUT);
	}

	for (TActorIterator<AActor> iter(GetWorld()); iter; ++iter)
	{
		AActor* findActor = *iter;

		if (findActor->GetActorNameOrLabel().Contains("ClickActor"))
		{
			clickActor = findActor;
		}
	}

	targetMoveLocation = player->GetActorLocation();
}


void UMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	if (player->inputType == EInputType::RTS_INPUT)
	{
		FVector dir = targetMoveLocation - player->GetActorLocation();
		dir.Z = 0;

		if (dir.Length() > 10)
		{
			player->AddMovementInput(dir.GetSafeNormal());
		}

		if (player->GetPlayerAnim() != nullptr)
		{
			if (player->GetCharacterMovement()->Velocity.Length() > 0.5f)
			{
				player->GetPlayerAnim()->moveDirection = dir.GetSafeNormal();
			}
			else
			{
				player->GetPlayerAnim()->moveDirection = FVector::ZeroVector;
			}
		}
	}

	if (player->inputType == EInputType::MOBILE_INPUT)
	{
		if (pc != nullptr)
		{
			float finger1_x = 0;
			float finger2_x = 0;
			float finger1_y = 0;
			float finger2_y = 0;
			bool bfinger1Touched = false;
			bool bfinger2Touched = false;

			// 닿은 손가락 위치 정보(Screen)를 가져온다.
			pc->GetInputTouchState(ETouchIndex::Touch1, finger1_x, finger1_y, bfinger1Touched);
			pc->GetInputTouchState(ETouchIndex::Touch2, finger2_x, finger2_y, bfinger2Touched);

			// 월드 좌표로 변환하기
			/*if (bfinger1Touched)
			{
				FVector finger1WorldPosition;
				FVector finger1WorldDirection;
				pc->DeprojectScreenPositionToWorld(finger1_x, finger1_y, finger1WorldPosition, finger1WorldDirection);
			}*/

			// 매우 중요 * 100000000
			// 두 좌표 값의 델타량을 이용해서 스케일을 변경한다.
			if (bfinger1Touched && bfinger2Touched)
			{
				// 현재 손가락 거리
				float fingerDistance = FVector2D::Distance(FVector2D(finger1_x, finger1_y), FVector2D(finger2_x, finger2_y));
				
				// 첫 터치인지 체크
				if (prevFingerDist > -1)
				{
					float deltaFingerDist = fingerDistance - prevFingerDist;
					clickActor->SetActorScale3D(FVector(1 + deltaFingerDist));
				}

				// 직전 손가락 거리로 저장
				prevFingerDist = fingerDistance;
			}
			else
			{
				// 첫 터치 초기화
				prevFingerDist = -1;
			}
		}
	}
}

void UMoveComponent::SetupPlayerInputComponent(class UEnhancedInputComponent* enhancedInputComponent)
{
	// TPS Type
	enhancedInputComponent->BindAction(ia_move, ETriggerEvent::Triggered, this, &UMoveComponent::PlayerMove);
	enhancedInputComponent->BindAction(ia_move, ETriggerEvent::Completed, this, &UMoveComponent::PlayerMove);
	enhancedInputComponent->BindAction(ia_rotate, ETriggerEvent::Triggered, this, &UMoveComponent::PlayerRotate);
	enhancedInputComponent->BindAction(ia_jump, ETriggerEvent::Started, this, &UMoveComponent::PlayerJump);
	enhancedInputComponent->BindAction(ia_jump, ETriggerEvent::Completed, this, &UMoveComponent::PlayerJumpEnd);

	// RTS Type
	enhancedInputComponent->BindAction(ia_leftClick, ETriggerEvent::Started, this, &UMoveComponent::SelectAction);
	enhancedInputComponent->BindAction(ia_rightClick, ETriggerEvent::Started, this, &UMoveComponent::ClickMoveAction);

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

void UMoveComponent::SelectAction(const FInputActionValue& value)
{
	if (player->inputType != EInputType::RTS_INPUT)
	{
		return;
	}

	if (pc != nullptr)
	{
		// 마우스 커서 위치 좌표(스크린 좌표계)를 월드 좌표계로 변환한다. -> Deprojction
		FVector mouseLocation;
		FVector mouseDirection;

		bool bTransform = pc->DeprojectMousePositionToWorld(mouseLocation, mouseDirection);

		if (bTransform)
		{
			// 마우스 위치에서 카메라가 마우스 커서를 바라보는 방향으로 라인트레이스를 월드 끝까지 발사한다.
			FHitResult hitInfo;
			FVector startLoc = mouseLocation;
			FVector endLoc = startLoc + mouseDirection * 10000;
			FCollisionQueryParams params;
			params.AddIgnoredActor(player);

			if (GetWorld()->LineTraceSingleByChannel(hitInfo, startLoc, endLoc, ECC_Visibility, params))
			{
				UE_LOG(LogTemp, Warning, TEXT("Selected Object Name: %s"), *hitInfo.GetActor()->GetActorNameOrLabel());

				//hitInfo.GetActor()->Destroy();
			}
		}
	}
}

void UMoveComponent::ClickMoveAction(const FInputActionValue& value)
{

	if (player->inputType != EInputType::RTS_INPUT)
	{
		return;
	}

	if (pc != nullptr)
	{
		// 마우스 커서 위치 좌표(스크린 좌표계)를 월드 좌표계로 변환한다. -> Deprojction
		FVector mouseLocation;
		FVector mouseDirection;

		bool bTransform = pc->DeprojectMousePositionToWorld(mouseLocation, mouseDirection);

		if (bTransform)
		{
			// 마우스 위치에서 카메라가 마우스 커서를 바라보는 방향으로 라인트레이스를 월드 끝까지 발사한다.
			FHitResult hitInfo;
			FVector startLoc = mouseLocation;
			FVector endLoc = startLoc + mouseDirection * 10000;
			FCollisionQueryParams params;
			params.AddIgnoredActor(player);

			if (GetWorld()->LineTraceSingleByChannel(hitInfo, startLoc, endLoc, ECC_Visibility, params))
			{
				// clickActor 오브젝트를 라인 트레이스에 충돌한 지점으로 옮긴다.
				if (clickActor != nullptr)
				{
					clickActor->SetActorLocation(hitInfo.ImpactPoint);
				}
				targetMoveLocation = hitInfo.ImpactPoint;

				
			}
		}
	}
}
