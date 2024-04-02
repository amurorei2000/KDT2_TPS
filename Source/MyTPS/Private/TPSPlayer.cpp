// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayer.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


ATPSPlayer::ATPSPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	// 스켈레탈 메시 에셋 할당하기
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> playerMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny'"));
	if (playerMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(playerMesh.Object);
	}

	GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));

	cameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Player Camera"));
	//cameraComp->SetupAttachment(GetCapsuleComponent());
	cameraComp->SetupAttachment(RootComponent);
	cameraComp->SetRelativeLocation(FVector(-500, 0, 60));

	// 캐릭터의 최대 이동 속력과 가속력을 설정한다.(cm/s)
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->MaxAcceleration = 2048.0f;

	// 컨트롤러의 입력 값을 캐릭터의 회전 값에 적용하기 위한 설정
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// 카메라의 회전 값을 컨트롤 로테이션에 따르게 하기
	cameraComp->bUsePawnControlRotation = true;

	// 이동하는 방향으로 캐릭터를 회전시키기
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;


}

void ATPSPlayer::BeginPlay()
{
	Super::BeginPlay();
	
	//APlayerController* pc = GetWorld()->GetFirstPlayerController();
	APlayerController* pc = GetController<APlayerController>();
	if (pc != nullptr)
	{
		UEnhancedInputLocalPlayerSubsystem* subsys = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer());
		
		if (subsys != nullptr)
		{
			subsys->AddMappingContext(imc_tpsKeyMap, 0);
		}
	}

}

void ATPSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// p = p0 + vt
	// v = v0 + at
	//SetActorLocation(GetActorLocation() + moveDirection * 600 * DeltaTime);

	// UCharacterMovementComponent의 함수를 이용해서 이동하는 방식
	// 입력 방향 벡터를 자신의 회전 값 기준으로 변환한다.(world -> local)
	FVector localMoveDirection = GetTransform().TransformVector(moveDirection);

	//FVector forward = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::X);
	//FVector right = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Y);
	//FVector localMoveDirection = GetActorForwardVector() * moveDirection.X + GetActorRightVector() * moveDirection.Y;

	AddMovementInput(localMoveDirection.GetSafeNormal());
	
	//SetActorRotation(GetActorRotation() + deltaRotation);
	AddControllerYawInput(deltaRotation.Yaw * mouseSensibility);
	AddControllerPitchInput(deltaRotation.Pitch * mouseSensibility);
}

void ATPSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* enhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (enhancedInputComponent != nullptr)
	{
		enhancedInputComponent->BindAction(ia_move, ETriggerEvent::Triggered, this, &ATPSPlayer::PlayerMove);
		enhancedInputComponent->BindAction(ia_move, ETriggerEvent::Completed, this, &ATPSPlayer::PlayerMove);
		enhancedInputComponent->BindAction(ia_rotate, ETriggerEvent::Triggered, this, &ATPSPlayer::PlayerRotate);
		enhancedInputComponent->BindAction(ia_rotate, ETriggerEvent::Completed, this, &ATPSPlayer::PlayerRotate);
	}
}

void ATPSPlayer::PlayerMove(const FInputActionValue& value)
{
	FVector2D inputValue = value.Get<FVector2D>();
	moveDirection = FVector(inputValue.Y, inputValue.X, 0);
}

void ATPSPlayer::PlayerRotate(const FInputActionValue& value)
{
	FVector2D inputValue = value.Get<FVector2D>();
	deltaRotation = FRotator(inputValue.Y, inputValue.X, 0);
	//UE_LOG(LogTemp, Warning, TEXT("(%.2f, %.2f)"), inputValue.X, inputValue.Y);
}

