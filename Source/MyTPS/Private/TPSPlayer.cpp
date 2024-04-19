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
#include "WeaponActor.h"
#include "PlayerAnimInstance.h"
#include "Enemy.h"
#include "EnemyHealthWidget.h"
#include <../../../../../../../Source/Runtime/UMG/Public/Components/WidgetComponent.h>
#include "GrenadeActor.h"



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
	springArmComp->bUsePawnControlRotation = true;
	springArmComp->bInheritYaw = true;
	springArmComp->bInheritPitch = true;
	springArmComp->bInheritRoll = false;

	cameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Player Camera"));
	//cameraComp->SetupAttachment(GetCapsuleComponent());
	//cameraComp->SetupAttachment(RootComponent);
	//cameraComp->SetRelativeLocation(FVector(-500, 0, 60));
	cameraComp->SetupAttachment(springArmComp, USpringArmComponent::SocketName);
	// 카메라의 회전 값을 컨트롤 로테이션에 따르게 하기
	cameraComp->bUsePawnControlRotation = false;

	gunMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun Mesh"));
	gunMeshComp->SetupAttachment(GetMesh(), FName("GunSocket"));

	floatingWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("Floating Widget Component"));
	floatingWidgetComp->SetupAttachment(GetMesh());
	floatingWidgetComp->SetRelativeLocation(FVector(0, 0, 200));
	floatingWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	floatingWidgetComp->SetDrawSize(FVector2D(150, 100));

	static ConstructorHelpers::FClassFinder<UEnemyHealthWidget> playerHPWidget(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/UI/WBP_PlayerHealthWidget.WBP_PlayerHealthWidget_C'"));
	if (playerHPWidget.Succeeded())
	{
		floatingWidgetComp->SetWidgetClass(playerHPWidget.Class);
	}


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

	// 0번 플레이어(로컬 플레이어)로 등록한다.
	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ATPSPlayer::BeginPlay()
{
	Super::BeginPlay();

	//APlayerController* pc = GetWorld()->GetFirstPlayerController();
	pc = GetController<APlayerController>();
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
	playerAnim = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());

	currentHP = maxHP;
	playerHealthWidget = Cast<UEnemyHealthWidget>(floatingWidgetComp->GetWidget());

	tpsPlayerState = EPlayerState::PLAYING;
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

	//UE_LOG(LogTemp, Warning, TEXT("Jump: %s"), GetCharacterMovement()->IsFalling() ? *FString("True") : *FString("False"));
}

void ATPSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* enhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (enhancedInputComponent != nullptr)
	{
		//enhancedInputComponent->BindAction(ia_move, ETriggerEvent::Started, this, &ATPSPlayer::PlayerMoveStart);
		enhancedInputComponent->BindAction(ia_move, ETriggerEvent::Triggered, this, &ATPSPlayer::PlayerMove);
		enhancedInputComponent->BindAction(ia_move, ETriggerEvent::Completed, this, &ATPSPlayer::PlayerMove);
		enhancedInputComponent->BindAction(ia_rotate, ETriggerEvent::Triggered, this, &ATPSPlayer::PlayerRotate);
		//enhancedInputComponent->BindAction(ia_rotate, ETriggerEvent::Completed, this, &ATPSPlayer::PlayerRotate);
		enhancedInputComponent->BindAction(ia_jump, ETriggerEvent::Started, this, &ATPSPlayer::PlayerJump);
		enhancedInputComponent->BindAction(ia_jump, ETriggerEvent::Completed, this, &ATPSPlayer::PlayerJumpEnd);
		//enhancedInputComponent->BindAction(ia_jump, ETriggerEvent::Started, this, &ACharacter::Jump);
		enhancedInputComponent->BindAction(ia_fire, ETriggerEvent::Started, this, &ATPSPlayer::PlayerFire);
		enhancedInputComponent->BindAction(ia_alpha1, ETriggerEvent::Started, this, &ATPSPlayer::SetWeapon1);
		enhancedInputComponent->BindAction(ia_alpha2, ETriggerEvent::Started, this, &ATPSPlayer::SetWeapon2);
		enhancedInputComponent->BindAction(ia_aimFocusing, ETriggerEvent::Started, this, &ATPSPlayer::SniperGunZoomInOut);
		enhancedInputComponent->BindAction(ia_aimFocusing, ETriggerEvent::Completed, this, &ATPSPlayer::SniperGunZoomInOut);
		enhancedInputComponent->BindAction(ia_releaseWeapon, ETriggerEvent::Started, this, &ATPSPlayer::ReleaseAction);
	}
}

void ATPSPlayer::SetGunAnimType(bool sniper)
{
	if (playerAnim != nullptr)
	{
		playerAnim->bUseSniper = sniper;
	}
}

void ATPSPlayer::SetCurrentWeaponNumber(bool bSniper)
{
	currentWeaponNumber = (int32)bSniper;
}

void ATPSPlayer::OnDamaged(int32 dmg, AEnemy* attacker)
{
	currentHP = FMath::Clamp(currentHP - dmg, 0, maxHP);

	if (playerHealthWidget != nullptr)
	{
		playerHealthWidget->SetHealthBar((float)currentHP / (float)maxHP, FLinearColor(1.0f, 0.138f, 0.059f, 1.0f));
	}

	if (currentHP <= 0)
	{
		// 자신의 상태를 죽음 상태로 전환한다.
		tpsPlayerState = EPlayerState::DEATH;
		playerAnim->bDead = true;
		GetCharacterMovement()->DisableMovement();

		attacker->RemoveTarget();
		attacker->enemyState = EEnemyState::RETURN;

		// 페이드 인 효과를 준다.
		if (pc != nullptr)
		{
			pc->PlayerCameraManager->StartCameraFade(0, 1, 3.0f, FLinearColor::Black);

			FTimerHandle restartHandle;
			GetWorldTimerManager().SetTimer(restartHandle, FTimerDelegate::CreateLambda([&]() {
				// 시작 위치에서 다시 시작한다.
				Cast<ATPSMainGameModeBase>(GetWorld()->GetAuthGameMode())->RespawnPlayer(pc, this);
				}), 3.0f, false);
		}

	}

	// 카메라를 흔드는 효과를 준다.
	if (pc != nullptr)
	{
		pc->ClientStopCameraShake(playerHitShake_bp);
		pc->ClientStartCameraShake(playerHitShake_bp);
	}

	// 히트 UI를 표시한다.
	/*gm->mainWidget_inst->ShowHitBorder(true);

	FTimerHandle hitBorderHandler;

	GetWorldTimerManager().SetTimer(hitBorderHandler, FTimerDelegate::CreateLambda([&]() {
		gm->mainWidget_inst->ShowHitBorder(false);
		}), 0.4f, false);*/
	gm->mainWidget_inst->PlayHitAnimation();

	int32 number = FMath::RandRange(1, 4);
	//FString sectionName = FString("hit") + FString::FromInt(number);
	FString sectionName = FString::Printf(TEXT("hit%d"), number);

	PlayAnimMontage(hitMotage, 1.0f, FName(sectionName));
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

	// 애니메이션 인스턴스에 있는 moveDirection 변수에 현재 입력 값을 전달한다.
	if (playerAnim != nullptr)
	{
		playerAnim->moveDirection = moveDirection;
	}
}

//void ATPSPlayer::PlayerMoveStart(const FInputActionValue& value)
//{
//	// 달리기 애니메이션으로 교체한다.
//	GetMesh()->PlayAnimation(anims[1], true);
//}

//void ATPSPlayer::PlayerMoveEnd(const FInputActionValue& value)
//{
//	// 대기 애니메이션으로 교체한다.
//	GetMesh()->PlayAnimation(anims[0], true);
//}

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

void ATPSPlayer::PlayerJumpEnd(const FInputActionValue& value)
{
	StopJumping();
}

void ATPSPlayer::PlayerFire(const FInputActionValue& value)
{
	// 총을 들고 있지 않거나 또는 총을 발사하는 애니메이션 중일 때에는 이 함수를 종료한다.
	if (attachedWeapon == nullptr || GetWorldTimerManager().IsTimerActive(endFireTimer))
	{
		return;
	}

	// 라인 트레이스 방식
	FHitResult hitInfo;
	FVector startLoc = cameraComp->GetComponentLocation();
	FVector endLoc = startLoc + cameraComp->GetForwardVector() * attachedWeapon->fireDistance;
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

		// 감지된 지점에 폭발 효과 이펙트를 출력한다.
		if (bulletFX != nullptr)
		{
			bulletFX->SetActorLocation(hitInfo.ImpactPoint);
			bulletFX->PlayFX();
		}

		// 만일, 감지된 대상이 Enemy라면...
		AEnemy* enemy = Cast<AEnemy>(hitInfo.GetActor());

		if (enemy != nullptr)
		{
			// Enemy의 OnDamage() 함수를 실행시킨다.
			enemy->OnDamaged(attachedWeapon->damage, this);
		}
	}
	//else
	//{
		//DrawDebugLine(GetWorld(), startLoc, endLoc, FColor(255, 0, 0), false, 2.0f, 0, 1.0f);
	//}

	if (playerAnim != nullptr)
	{
		playerAnim->bFire = true;

		GetWorldTimerManager().SetTimer(endFireTimer, this, &ATPSPlayer::EndFire, 0.5f, false);
	}

	if (fire_montages.Num() > 1)
	{
		if (attachedWeapon->bSniperGun)
		{
			PlayAnimMontage(fire_montages[1]);
		}
		else
		{
			PlayAnimMontage(fire_montages[0]);
		}
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
	currentWeaponNumber = 0;
	ChangeGunType(currentWeaponNumber);
}

void ATPSPlayer::SetWeapon2(const FInputActionValue& value)
{
	currentWeaponNumber = 1;
	ChangeGunType(currentWeaponNumber);
}

void ATPSPlayer::SniperGunZoomInOut(const FInputActionValue& value)
{
	bool inputValue = value.Get<bool>();

	// 노멀 건일 때
	if (currentWeaponNumber == 0)
	{
		FVector dir = GetActorForwardVector() + GetActorUpVector();
		float throwPower = 2000;

		if (inputValue)
		{
			// 수류탄의 궤적을 그린다.
			TArray<FVector> results = CalculateThrowPoints(dir, throwPower, 0.1f, 3.0f);

			for (int32 i = 0; i < results.Num() - 1; i++)
			{
				DrawDebugLine(GetWorld(), results[i], results[i + 1], FColor::Black, true, 0, 0, 3);
			}
		}
		else
		{
			// 수류탄을 생성 및 발사한다.
			FActorSpawnParameters params;
			params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AGrenadeActor* bomb = GetWorld()->SpawnActor<AGrenadeActor>(grenade_bp, GetActorLocation() + GetActorForwardVector() * 100, FRotator::ZeroRotator, params);

			bomb->Throw(dir, throwPower);
		}
	}
	// 스나이퍼 건일 때
	else if (currentWeaponNumber == 1)
	{
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
}

void ATPSPlayer::ReleaseAction(const FInputActionValue& value)
{
	if (attachedWeapon == nullptr)
	{
		return;
	}
	attachedWeapon->Release();
	attachedWeapon = nullptr;
	currentWeaponNumber = 0;
}

void ATPSPlayer::EndFire()
{
	if (playerAnim != nullptr)
	{
		playerAnim->bFire = false;
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

TArray<FVector> ATPSPlayer::CalculateThrowPoints(const FVector& dir, float power, float interval, float simulTime)
{
	TArray<FVector> simulPoints;

	// 총 횟수 = 시뮬레이션 총 시간 / 시뮬레이션 간격
	int32 segment = simulTime / interval;
	FVector startLocation = GetActorLocation() + GetActorForwardVector() * 100;
	FVector gravityValue = FVector(0, 0, GetWorld()->GetDefaultGravityZ());
	float mass = 5;

	for (int32 i = 0; i < segment; i++)
	{
		// p = p0 + vt - 0.5*g*t*t * mass * mass
		float term = interval * i;
		FVector predictLocation = startLocation + dir * power * term + 0.5f * gravityValue * term * term * mass * mass;
		simulPoints.Add(predictLocation);
	}

	return simulPoints;
}

