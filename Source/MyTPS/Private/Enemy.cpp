// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "TPSPlayer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemyAnimInstance.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "EnemyHealthWidget.h"
#include "NavigationInvokerComponent.h"
#include "GrenadeActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "FSMComponent.h"
#include "EngineUtils.h"


AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->SetCollisionProfileName(FName("EnemyPreset"));
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	floatingWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("Floating Widget Component"));
	floatingWidgetComp->SetupAttachment(GetMesh());
	floatingWidgetComp->SetRelativeLocation(FVector(0, 0, 210));
	floatingWidgetComp->SetRelativeRotation(FRotator(0, 90, 0));
	floatingWidgetComp->SetWidgetSpace(EWidgetSpace::World);
	floatingWidgetComp->SetDrawSize(FVector2D(150, 100));

	// 액터 컴포넌트들
	navInvokerComp = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavInvoker Component"));
	fsmComp = CreateDefaultSubobject<UFSMComponent>(TEXT("FSM Component"));

	// AI Controller의 자동 Possess 기능 실행을 월드에 배치되었을 때 또는 스폰 됐을 때 실행하는 것으로 설정한다.
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	// 월드에 있는 플레이어를 찾는다.
	for (TActorIterator<ATPSPlayer> findActor(GetWorld()); findActor; ++findActor)
	{
		player = *findActor;
	}

	// 플레이할 Idle 애니메이션 번호를 선택해서 EnemyAnimInstance의 idleNumber 변수에 전달한다.
	anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance());

	// 체력 변수를 초기화한다.
	currentHP = maxHP;

	// 메시의 머티리얼을 DynamicMaterial로 변경해준다.
	UMaterialInterface* currentMat = GetMesh()->GetMaterial(0);
	dynamicMat = UMaterialInstanceDynamic::Create(currentMat, nullptr);
	GetMesh()->SetMaterial(0, dynamicMat);

	// 위젯 컴포넌트에 할당되어 있는 위젯 인스턴스를 가져온다.
	healthWidget = Cast<UEnemyHealthWidget>(floatingWidgetComp->GetWidget());
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 위젯 빌보드
	floatingWidgetComp->SetWorldRotation(BillboardWidgetComponent(player));
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::OnDamaged(int32 dmg, AActor* attacker)
{
	if (fsmComp->enemyState == EEnemyState::DAMAGED)
	{
		return;
	}
	
	// 현재 체력 갱신
	currentHP = FMath::Clamp(currentHP - dmg, 0, maxHP);
	if (healthWidget != nullptr)
	{
		healthWidget->SetHealthBar((float)currentHP / (float)maxHP, FLinearColor(1.0f, 0.138f, 0.059f, 1.0f));
	}

	// 데미지 계산 결과 현재 체력이 0보다 크다면....
	if (currentHP > 0)
	{
		// 피격 상태로 전환한다.
		fsmComp->enemyState = EEnemyState::DAMAGED;
		fsmComp->SetNewTarget(attacker);
		hitLocation = GetActorLocation();
		hitDirection = GetActorLocation() - attacker->GetActorLocation();
		hitDirection.Z = 0;
		hitDirection.Normalize();
		fsmComp->targetLoc = hitLocation + hitDirection * 50.0f;

		// 머티리얼 색상에 붉은 색을 입힌다.
		dynamicMat->SetVectorParameterValue(FName("hitColor"), FVector4(1, 0, 0, 1));
	}
	// 그렇지 않으면...
	else
	{
		// 죽음 상태로 전환한다.
		fsmComp->enemyState = EEnemyState::DIE;
		Die();
	}
}

void AEnemy::Die()
{
	// 콜리젼을 NoCollision 상태로 전환한다.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// CharacterMovementComponent를 비활성화한다.
	GetCharacterMovement()->DisableMovement();

	// 죽음 애니메이션 몽타쥬를 실행한다.
	// 1~3 사이의 랜덤한 숫자를 하나 뽑는다.
	int32 num = FMath::RandRange(1, 3);
	// "Dead" + 숫자로 몽타주의 섹션 이름을 만든다.
	FString sectionName = FString("Dead") + FString::FromInt(num);
	// 섹션 이름을 이용해서 몽타주를 플레이한다.
	PlayAnimMontage(death_montage, 1, FName(sectionName));

	// 3초 뒤에 제거한다.
	/*FTimerHandle deadHandler;

	GetWorldTimerManager().SetTimer(deadHandler, FTimerDelegate::CreateLambda([&]() {
		Destroy();
		}), 5.6f, false);*/
}

// 카메라 컴포넌트를 가지고 있는 액터를 위젯 컴포넌트가 바라보도록 회전 값을 계산해주는 함수
FRotator AEnemy::BillboardWidgetComponent(class AActor* camActor)
{
	ATPSPlayer* camTarget = Cast<ATPSPlayer>(camActor);
	if (camTarget != nullptr)
	{
		FVector lookDir = (camTarget->cameraComp->GetComponentLocation() - floatingWidgetComp->GetComponentLocation()).GetSafeNormal();
		FRotator lookRot = UKismetMathLibrary::MakeRotFromX(lookDir);
		//FRotator lookRot = lookDir.ToOrientationRotator();

		return lookRot;
	}
	else
	{
		return FRotator::ZeroRotator;
	}
}

void AEnemy::HitBomb(int32 dmg, const FVector& attackDir, float maxRadius, float upPower)
{
	// 1. 데미지를 적용시킨다.
	currentHP = FMath::Clamp(currentHP - dmg, 0, maxHP);
	healthWidget->SetHealthBar((float)currentHP / (float)maxHP, FLinearColor(1.0f, 0.138f, 0.059f, 1.0f));

	if (currentHP > 0)
	{
		fsmComp->enemyState = EEnemyState::DAMAGED_BOMB;
	}
	else
	{
		fsmComp->enemyState = EEnemyState::DIE;
	}

	// 2. 랙돌 적용을 해준다.
	//GetMesh()->SetCollisionProfileName(FName("Ragdoll"));
	//GetMesh()->SetSimulatePhysics(true);

	// 3. 폭발의 중심지로부터 바깥쪽 방향으로 날려보낸다.
	GetCapsuleComponent()->SetSimulatePhysics(true);
	bombDir = attackDir;
	bombDist = maxRadius;
	bombUpPower = upPower;

	GetWorldTimerManager().SetTimerForNextTick(this, &AEnemy::BombImpact);
}

void AEnemy::BombImpact()
{
	float power = (bombDist - bombDir.Length()) * 500.0f;
	bombDir.Normalize();
	bombDir.Z = bombUpPower;

	GetCapsuleComponent()->AddImpulse(bombDir * power);
}

