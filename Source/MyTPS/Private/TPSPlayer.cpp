// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayer.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "KismetTraceUtils.h"
#include "Components/StaticMeshComponent.h"
#include "BulletFXActor.h"
#include "EngineUtils.h"
#include "TPSMainGameModeBase.h"
#include "MainWidget.h"


ATPSPlayer::ATPSPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	// 스켈레탈 메시 에셋 할당하기
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> playerMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/AnimStarterPack/UE4_Mannequin/Mesh/SK_Mannequin.SK_Mannequin'"));
	if (playerMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(playerMesh.Object);
	}

	GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));

	springArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Spring Arm"));
	springArmComp->SetupAttachment(RootComponent);
	springArmComp->SetRelativeLocation(FVector(0, 50, 60));
	springArmComp->TargetArmLength = 300;
	springArmComp->SocketOffset = FVector(0, 0, 0);
	// 스프링 암의 target -> socket 사이의 장애물 검사
	springArmComp->bDoCollisionTest = true;
	springArmComp->ProbeSize = 12.0f;
	springArmComp->ProbeChannel = ECC_Camera;
	// 스프링 암의 이동을 지연시키는 효과를 켜기
	springArmComp->bEnableCameraLag = true;
	springArmComp->CameraLagSpeed = 50.0f;

	cameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Player Camera"));
	//cameraComp->SetupAttachment(GetCapsuleComponent());
	//cameraComp->SetupAttachment(RootComponent);
	//cameraComp->SetRelativeLocation(FVector(-500, 0, 60));
	cameraComp->SetupAttachment(springArmComp, USpringArmComponent::SocketName);
	// 카메라의 회전 값을 컨트롤 로테이션에 따르게 하기
	cameraComp->bUsePawnControlRotation = false;

	gunMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun Mesh"));
	gunMeshComp->SetupAttachment(GetMesh(), FName("GunSocket"));


	// 캐릭터의 최대 이동 속력과 가속력을 설정한다.(cm/s)
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->MaxAcceleration = 2048.0f;
	// 캐릭터의 지면 마찰력과 감속력을 설정한다.
	GetCharacterMovement()->GroundFriction = 8.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;

	// 컨트롤러의 입력 값을 캐릭터의 회전 값에 적용하기 위한 설정
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// 이동하는 방향으로 캐릭터를 회전시키기
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// 점프력 설정
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.05f;
	
	// 연속 점프 가능 수
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
	
	// 월드에 있는 총알 피격 효과 액터를 찾아서 변수에 참조시킨다.
	for (TActorIterator<ABulletFXActor> iter(GetWorld()); iter; ++iter)
	{
		bulletFX = *iter;
	}

	gm = Cast<ATPSMainGameModeBase>(GetWorld()->GetAuthGameMode());
}

void ATPSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 카메라와 캐릭터 사이의 방해물을 검사하는 함수
	//CheckObstacles();
	
	// 카메라 줌 인 아웃 처리
	float direction = bZoomIn ? 1.0f : -1.0f;
	alpha += DeltaTime * direction * 5.0f;
	alpha = FMath::Clamp(alpha, 0.0f, 1.0f);

	float result = FMath::Lerp(90, 40, alpha);
	cameraComp->SetFieldOfView(result);

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
		enhancedInputComponent->BindAction(ia_fire, ETriggerEvent::Started, this, &ATPSPlayer::PlayerFire);
		enhancedInputComponent->BindAction(ia_alpha1, ETriggerEvent::Started, this, &ATPSPlayer::SetWeapon1);
		enhancedInputComponent->BindAction(ia_alpha2, ETriggerEvent::Started, this, &ATPSPlayer::SetWeapon2);
		enhancedInputComponent->BindAction(ia_aimFocusing, ETriggerEvent::Started, this, &ATPSPlayer::SniperGunZoomInOut);
		enhancedInputComponent->BindAction(ia_aimFocusing, ETriggerEvent::Completed, this, &ATPSPlayer::SniperGunZoomInOut);
	}
}

void ATPSPlayer::PlayerMove(const FInputActionValue& value)
{
	FVector2D inputValue = value.Get<FVector2D>();
	moveDirection = FVector(inputValue.Y, inputValue.X, 0);

	// p = p0 + vt
	// v = v0 + at
	//SetActorLocation(GetActorLocation() + moveDirection * 600 * DeltaTime);

	// UCharacterMovementComponent의 함수를 이용해서 이동하는 방식
	// 입력 방향 벡터를 자신의 회전 값 기준으로 변환한다.(world -> local)
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
	// 라인 트레이스 방식
	FHitResult hitInfo;
	FVector startLoc = cameraComp->GetComponentLocation();
	FVector endLoc = startLoc + cameraComp->GetForwardVector() * 1000.0f;
	// 충돌 체크에 포함할 오브젝트 타입
	FCollisionObjectQueryParams objQueryParams;
	objQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	objQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	// 충돌 체크에서 제외할 액터 또는 컴포넌트
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this);

	// 싱글 방식
	//bool bResult = GetWorld()->LineTraceSingleByObjectType(hitInfo, startLoc, endLoc, objQueryParams, queryParams);
	bool bResult = GetWorld()->LineTraceSingleByChannel(hitInfo, startLoc, endLoc, ECC_Visibility, queryParams);

	// 멀티 방식
	//TArray<FHitResult> hitInfos;

	//bool bResult = GetWorld()->LineTraceMultiByChannel(hitInfos, startLoc, endLoc, ECC_Visibility, queryParams);
	//bool bResult = GetWorld()->LineTraceMultiByObjectType(hitInfos, startLoc, endLoc, objQueryParams, queryParams);

	if (bResult)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Hit Actor Name: %s"), *hitInfo.GetActor()->GetActorNameOrLabel());
		//DrawDebugLine(GetWorld(), startLoc, hitInfo.ImpactPoint, FColor(0, 255, 0), false, 2.0f, 0, 1);
		//DrawDebugLine(GetWorld(), hitInfo.ImpactPoint, endLoc, FColor(255, 0, 0), false, 2.0f, 0, 1);

		// 충돌한 지점에 폭발 효과 이펙트를 출력한다.
		if (bulletFX != nullptr)
		{
			bulletFX->SetActorLocation(hitInfo.ImpactPoint);
			bulletFX->PlayFX();
		}
	}
	else
	{
		//DrawDebugLine(GetWorld(), startLoc, endLoc, FColor(255, 0, 0), false, 2.0f, 0, 1.0f);
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

	// 정육면체를 45도 회전시킨 상태로 발사한다.
	bool bResult = GetWorld()->SweepSingleByChannel(hitInfo, startLoc, endLoc, startRot, ECC_Visibility, FCollisionShape::MakeBox(FVector(10)), queryParams);

	if (bResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit Actor name: %s"), *hitInfo.GetActor()->GetActorNameOrLabel());
		FVector centerPos = (startLoc + endLoc) * 0.5f;
	}

	DrawDebugBoxTraceSingle(GetWorld(), startLoc, endLoc, FVector(10), FRotator(0, 0, 45), EDrawDebugTrace::ForDuration, true, hitInfo, FLinearColor::Green, FLinearColor::Red, 2.0f);
}

void ATPSPlayer::SetWeapon1(const FInputActionValue& value)
{
	ChangeGunType(0);
}

void ATPSPlayer::SetWeapon2(const FInputActionValue& value)
{
	ChangeGunType(1);
}

void ATPSPlayer::SniperGunZoomInOut(const FInputActionValue& value)
{
	bool inputValue = value.Get<bool>();

	if (inputValue)
	{
		springArmComp->TargetArmLength = -100;
	}
	else
	{
		springArmComp->TargetArmLength = 300;
	}

	// zoom in 효과 on/off 설정
	bZoomIn = inputValue;

	if (gm != nullptr && gm->mainWidget_inst != nullptr)
	{
		gm->mainWidget_inst->SetSniperMode(!inputValue);
	}
}

void ATPSPlayer::CheckObstacles()
{
	// 플레이어->카메라 방향으로 라인 트레이스를 이용해서 Visibility 채널을 검색한다.
	FHitResult hitInfo;
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this);
	FVector relativeCamPos = GetTransform().TransformVector(camPosition);
	bool bSensored = GetWorld()->LineTraceSingleByChannel(hitInfo, GetActorLocation(), GetActorLocation() + relativeCamPos, ECC_Visibility, queryParams);

	// 만일, 검사된 것이 있다면...
	if (bSensored)
	{
		// 충돌한 지점에서 10cm 앞쪽에 카메라를 위치시킨다.
		FVector camLocation = hitInfo.ImpactPoint + hitInfo.ImpactNormal * 10;
		FVector lerpLocation = FMath::Lerp(cameraComp->GetComponentLocation(), camLocation, GetWorld()->GetDeltaSeconds() * 5);
		
		cameraComp->SetWorldLocation(lerpLocation);
	}
	// 그렇지 않다면...
	else
	{
		// 카메라를 원래 위치에 위치시킨다.
		//cameraComp->SetRelativeLocation(camPosition);
		
		// 카메라 지연 효과
		SetCameraLag(GetWorld()->GetDeltaSeconds(), 5.0f);
	}
}

void ATPSPlayer::SetCameraLag(float deltaTime, float traceSpeed)
{
	// Lerp를 이용해서 카메라의 위치를 지연 이동시킨다.
	FVector targetLoc = GetActorLocation() + GetTransform().TransformVector(camPosition);
	FVector lerpLoc = FMath::Lerp(previousCamLoc, targetLoc, deltaTime * traceSpeed);
	cameraComp->SetWorldLocation(lerpLoc);

	// 현재 프레임의 카메라 위치를 변수에 저장시킨다.
	previousCamLoc = cameraComp->GetComponentLocation();
}

void ATPSPlayer::ChangeGunType(int32 number)
{
	// 메시 변경
	gunMeshComp->SetStaticMesh(gunTypes[number]);
	gunMeshComp->SetRelativeLocation(gunOffset[number]);
	
	// UI 텍스쳐 변경
	if (gm != nullptr)
	{
		gm->mainWidget_inst->SetWeaponTexture(number);
	}
}

