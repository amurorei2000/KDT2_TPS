// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayer.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


ATPSPlayer::ATPSPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	// ���̷�Ż �޽� ���� �Ҵ��ϱ�
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

	// ĳ������ �ִ� �̵� �ӷ°� ���ӷ��� �����Ѵ�.(cm/s)
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->MaxAcceleration = 2048.0f;

	// ��Ʈ�ѷ��� �Է� ���� ĳ������ ȸ�� ���� �����ϱ� ���� ����
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// ī�޶��� ȸ�� ���� ��Ʈ�� �����̼ǿ� ������ �ϱ�
	cameraComp->bUsePawnControlRotation = true;

	// �̵��ϴ� �������� ĳ���͸� ȸ����Ű��
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

	// UCharacterMovementComponent�� �Լ��� �̿��ؼ� �̵��ϴ� ���
	// �Է� ���� ���͸� �ڽ��� ȸ�� �� �������� ��ȯ�Ѵ�.(world -> local)
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

