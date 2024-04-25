// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayer.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TPSMainGameModeBase.h"
#include "MainWidget.h"
#include "PlayerAnimInstance.h"
#include "Enemy.h"
#include "EnemyHealthWidget.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MoveComponent.h"
#include "WeaponComponent.h"
#include "FSMComponent.h"
#include "TPSGameInstance.h"


ATPSPlayer::ATPSPlayer()
{
	PrimaryActorTick.bCanEverTick = false;

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

	

	// 0번 플레이어(로컬 플레이어)로 등록한다.
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// 사용자 액터 컴포넌트들
	moveComp = CreateDefaultSubobject<UMoveComponent>(TEXT("Move Component"));
	weaponComp = CreateDefaultSubobject<UWeaponComponent>(TEXT("Weapon Component"));
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

	gm = Cast<ATPSMainGameModeBase>(GetWorld()->GetAuthGameMode());
	playerAnim = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());

	currentHP = playerStatus.maxHP;
	playerHealthWidget = Cast<UEnemyHealthWidget>(floatingWidgetComp->GetWidget());

	tpsPlayerState = EPlayerState::PLAYING;

	// 연속 점프 가능 수 설정
	JumpMaxCount = playerStatus.jumpCount;

	// 기본 이동 속도 설정
	GetCharacterMovement()->MaxWalkSpeed = playerStatus.baseSpeed;

	// 게임 인스턴스 가져오기
	UTPSGameInstance* gi = GetGameInstance<UTPSGameInstance>();
	
	// 테이블의 모든 행 이름(Row Name)을 읽어오기
	TArray<FName> rowNameList = gi->dt_playerStatus->GetRowNames();
	for (int32 i = 0; i < rowNameList.Num(); i++)
	{
		UE_LOG(LogTemp, Warning, TEXT("%d: %s"), i, *rowNameList[i].ToString());
	}

	// 테이블 데이터를 행(Row) 단위로 읽기
	FPlayerStatus readData = gi->GetPlayerTableData("HeavyMan");
	UE_LOG(LogTemp, Warning, TEXT("Base Speed: %f, DashSpeed: %f, Max Jump Count: %d, Max HP: %d"), readData.baseSpeed, readData.dashSpeed, readData.jumpCount, readData.maxHP);
}

void ATPSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 카메라와 캐릭터 사이의 방해물을 검사하는 함수
	//CheckObstacles();
}

void ATPSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* enhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (enhancedInputComponent != nullptr)
	{
		moveComp->SetupPlayerInputComponent(enhancedInputComponent);
		weaponComp->SetupPlayerInputComponent(enhancedInputComponent);
	}
}

void ATPSPlayer::SetGunAnimType(bool sniper)
{
	if (playerAnim != nullptr)
	{
		playerAnim->bUseSniper = sniper;
	}
}

void ATPSPlayer::OnDamaged(int32 dmg, AEnemy* attacker)
{
	currentHP = FMath::Clamp(currentHP - dmg, 0, playerStatus.maxHP);

	if (playerHealthWidget != nullptr)
	{
		playerHealthWidget->SetHealthBar((float)currentHP / (float)playerStatus.maxHP, FLinearColor(1.0f, 0.138f, 0.059f, 1.0f));
	}

	if (currentHP <= 0)
	{
		// 자신의 상태를 죽음 상태로 전환한다.
		tpsPlayerState = EPlayerState::DEATH;
		playerAnim->bDead = true;
		GetCharacterMovement()->DisableMovement();

		attacker->fsmComp->RemoveTarget();
		attacker->fsmComp->enemyState = EEnemyState::RETURN;

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





