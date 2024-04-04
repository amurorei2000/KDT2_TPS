// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayer.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include <../../../../../../../Source/Runtime/Engine/Public/KismetTraceUtils.h>
#include "Components/SkeletalMeshComponent.h"


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

	springArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Spring Arm"));
	springArmComp->SetupAttachment(RootComponent);
	springArmComp->TargetArmLength = 500;
	springArmComp->SocketOffset = FVector(0, 0, 60);
	// ������ ���� target -> socket ������ ��ֹ� �˻�
	springArmComp->bDoCollisionTest = true;
	springArmComp->ProbeSize = 12.0f;
	springArmComp->ProbeChannel = ECC_Camera;
	// ������ ���� �̵��� ������Ű�� ȿ���� �ѱ�
	springArmComp->bEnableCameraLag = true;
	springArmComp->CameraLagSpeed = 10.0f;

	cameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Player Camera"));
	//cameraComp->SetupAttachment(GetCapsuleComponent());
	//cameraComp->SetupAttachment(RootComponent);
	//cameraComp->SetRelativeLocation(FVector(-500, 0, 60));
	cameraComp->SetupAttachment(springArmComp, USpringArmComponent::SocketName);
	// ī�޶��� ȸ�� ���� ��Ʈ�� �����̼ǿ� ������ �ϱ�
	cameraComp->bUsePawnControlRotation = true;

	gunMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gun Mesh"));
	gunMeshComp->SetupAttachment(GetMesh(), FName("GunSocket"));


	// ĳ������ �ִ� �̵� �ӷ°� ���ӷ��� �����Ѵ�.(cm/s)
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->MaxAcceleration = 2048.0f;
	// ĳ������ ���� �����°� ���ӷ��� �����Ѵ�.
	GetCharacterMovement()->GroundFriction = 8.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;

	// ��Ʈ�ѷ��� �Է� ���� ĳ������ ȸ�� ���� �����ϱ� ���� ����
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// �̵��ϴ� �������� ĳ���͸� ȸ����Ű��
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// ������ ����
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.05f;
	
	// ���� ���� ���� ��
	JumpMaxCount = 2;
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

	// ī�޶�� ĳ���� ������ ���ع��� �˻��ϴ� �Լ�
	//CheckObstacles();
}

void ATPSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* enhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (enhancedInputComponent != nullptr)
	{
		enhancedInputComponent->BindAction(ia_move, ETriggerEvent::Triggered, this, &ATPSPlayer::PlayerMove);
		//enhancedInputComponent->BindAction(ia_move, ETriggerEvent::Completed, this, &ATPSPlayer::PlayerMove);
		enhancedInputComponent->BindAction(ia_rotate, ETriggerEvent::Triggered, this, &ATPSPlayer::PlayerRotate);
		//enhancedInputComponent->BindAction(ia_rotate, ETriggerEvent::Completed, this, &ATPSPlayer::PlayerRotate);
		enhancedInputComponent->BindAction(ia_jump, ETriggerEvent::Started, this, &ATPSPlayer::PlayerJump);
		//enhancedInputComponent->BindAction(ia_jump, ETriggerEvent::Started, this, &ACharacter::Jump);
		enhancedInputComponent->BindAction(ia_fire, ETriggerEvent::Started, this, &ATPSPlayer::PlayerFire2);

	}
}

void ATPSPlayer::PlayerMove(const FInputActionValue& value)
{
	FVector2D inputValue = value.Get<FVector2D>();
	moveDirection = FVector(inputValue.Y, inputValue.X, 0);

	// p = p0 + vt
	// v = v0 + at
	//SetActorLocation(GetActorLocation() + moveDirection * 600 * DeltaTime);

	// UCharacterMovementComponent�� �Լ��� �̿��ؼ� �̵��ϴ� ���
	// �Է� ���� ���͸� �ڽ��� ȸ�� �� �������� ��ȯ�Ѵ�.(world -> local)
	//FVector localMoveDirection = GetTransform().TransformVector(moveDirection);
	FVector localMoveDirection = cameraComp->GetComponentTransform().TransformVector(moveDirection);

	//FVector forward = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::X);
	//FVector right = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Y);
	//FVector localMoveDirection = GetActorForwardVector() * moveDirection.X + GetActorRightVector() * moveDirection.Y;
	AddMovementInput(localMoveDirection.GetSafeNormal());
}

void ATPSPlayer::PlayerRotate(const FInputActionValue& value)
{
	FVector2D inputValue = value.Get<FVector2D>();
	deltaRotation = FRotator(inputValue.Y, inputValue.X, 0);
	//UE_LOG(LogTemp, Warning, TEXT("(%.2f, %.2f)"), inputValue.X, inputValue.Y);

	//SetActorRotation(GetActorRotation() + deltaRotation);
	AddControllerYawInput(deltaRotation.Yaw * mouseSensibility);
	AddControllerPitchInput(deltaRotation.Pitch * mouseSensibility);
}

void ATPSPlayer::PlayerJump(const FInputActionValue& value)
{
	Jump();

}

void ATPSPlayer::PlayerFire(const FInputActionValue& value)
{
	// ���� Ʈ���̽� ���
	FHitResult hitInfo;
	FVector startLoc = GetActorLocation();
	FVector endLoc = startLoc + GetActorForwardVector() * 1000.0f;
	// �浹 üũ�� ������ ������Ʈ Ÿ��
	FCollisionObjectQueryParams objQueryParams;
	objQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	objQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	// �浹 üũ���� ������ ���� �Ǵ� ������Ʈ
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this);

	// �̱� ���
	//bool bResult = GetWorld()->LineTraceSingleByObjectType(hitInfo, startLoc, endLoc, objQueryParams, queryParams);
	bool bResult = GetWorld()->LineTraceSingleByChannel(hitInfo, startLoc, endLoc, ECC_Visibility, queryParams);

	// ��Ƽ ���
	//TArray<FHitResult> hitInfos;

	//bool bResult = GetWorld()->LineTraceMultiByChannel(hitInfos, startLoc, endLoc, ECC_Visibility, queryParams);
	//bool bResult = GetWorld()->LineTraceMultiByObjectType(hitInfos, startLoc, endLoc, objQueryParams, queryParams);

	if (bResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit Actor Name: %s"), *hitInfo.GetActor()->GetActorNameOrLabel());
		DrawDebugLine(GetWorld(), startLoc, hitInfo.ImpactPoint, FColor(0, 255, 0), false, 2.0f, 0, 1);
		DrawDebugLine(GetWorld(), hitInfo.ImpactPoint, endLoc, FColor(255, 0, 0), false, 2.0f, 0, 1);
	}
	else
	{
		DrawDebugLine(GetWorld(), startLoc, endLoc, FColor(255, 0, 0), false, 2.0f, 0, 1.0f);
	}
}

void ATPSPlayer::PlayerFire2(const FInputActionValue& value)
{
	FHitResult hitInfo;
	FVector startLoc = GetActorLocation();
	FVector endLoc = startLoc + GetActorForwardVector() * 1000.0f;
	FQuat startRot = FRotator(0, 0, 45).Quaternion();
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this);

	// ������ü�� 45�� ȸ����Ų ���·� �߻��Ѵ�.
	bool bResult = GetWorld()->SweepSingleByChannel(hitInfo, startLoc, endLoc, startRot, ECC_Visibility, FCollisionShape::MakeBox(FVector(10)), queryParams);

	if (bResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit Actor name: %s"), *hitInfo.GetActor()->GetActorNameOrLabel());
		FVector centerPos = (startLoc + endLoc) * 0.5f;
	}

	DrawDebugBoxTraceSingle(GetWorld(), startLoc, endLoc, FVector(10), FRotator(0, 0, 45), EDrawDebugTrace::ForDuration, true, hitInfo, FLinearColor::Green, FLinearColor::Red, 2.0f);
}

void ATPSPlayer::CheckObstacles()
{
	// �÷��̾�->ī�޶� �������� ���� Ʈ���̽��� �̿��ؼ� Visibility ä���� �˻��Ѵ�.
	FHitResult hitInfo;
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this);
	FVector relativeCamPos = GetTransform().TransformVector(camPosition);
	bool bSensored = GetWorld()->LineTraceSingleByChannel(hitInfo, GetActorLocation(), GetActorLocation() + relativeCamPos, ECC_Visibility, queryParams);

	// ����, �˻�� ���� �ִٸ�...
	if (bSensored)
	{
		// �浹�� �������� 10cm ���ʿ� ī�޶� ��ġ��Ų��.
		FVector camLocation = hitInfo.ImpactPoint + hitInfo.ImpactNormal * 10;
		FVector lerpLocation = FMath::Lerp(cameraComp->GetComponentLocation(), camLocation, GetWorld()->GetDeltaSeconds() * 5);
		
		cameraComp->SetWorldLocation(lerpLocation);
	}
	// �׷��� �ʴٸ�...
	else
	{
		// ī�޶� ���� ��ġ�� ��ġ��Ų��.
		//cameraComp->SetRelativeLocation(camPosition);
		
		// ī�޶� ���� ȿ��
		SetCameraLag(GetWorld()->GetDeltaSeconds(), 5.0f);
	}
}

void ATPSPlayer::SetCameraLag(float deltaTime, float traceSpeed)
{
	// Lerp�� �̿��ؼ� ī�޶��� ��ġ�� ���� �̵���Ų��.
	FVector targetLoc = GetActorLocation() + GetTransform().TransformVector(camPosition);
	FVector lerpLoc = FMath::Lerp(previousCamLoc, targetLoc, deltaTime * traceSpeed);
	cameraComp->SetWorldLocation(lerpLoc);

	// ���� �������� ī�޶� ��ġ�� ������ �����Ų��.
	previousCamLoc = cameraComp->GetComponentLocation();
}

