// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "EngineUtils.h"
#include "TPSPlayer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemyAnimInstance.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnemyHealthWidget.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include <../../../../../../../Source/Runtime/NavigationSystem/Public/NavigationPath.h>


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

	// AI Controller의 자동 Possess 기능 실행을 월드에 배치되었을 때 또는 스폰 됐을 때 실행하는 것으로 설정한다.
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	// 기본 상태를 IDLE 상태로 초기화한다.
	enemyState = EEnemyState::IDLE;

	// 플레이할 Idle 애니메이션 번호를 선택해서 EnemyAnimInstance의 idleNumber 변수에 전달한다.
	anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance());


	// 월드에 있는 플레이어를 찾는다.
	for (TActorIterator<ATPSPlayer> player(GetWorld()); player; ++player)
	{
		target = *player;
	}

	// 시작 위치를 정한다.
	originLocation = GetActorLocation();
	originRotation = GetActorRotation();

	// 체력 변수를 초기화한다.
	currentHP = maxHP;

	// 메시의 머티리얼을 DynamicMaterial로 변경해준다.
	UMaterialInterface* currentMat = GetMesh()->GetMaterial(0);
	dynamicMat = UMaterialInstanceDynamic::Create(currentMat, nullptr);
	GetMesh()->SetMaterial(0, dynamicMat);

	// 위젯 컴포넌트에 할당되어 있는 위젯 인스턴스를 가져온다.
	healthWidget = Cast<UEnemyHealthWidget>(floatingWidgetComp->GetWidget());

	aiCon = GetController<AAIController>();
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (enemyState)
	{
	case EEnemyState::IDLE:
		Idle(DeltaTime);
		break;
	case EEnemyState::MOVE:
		MoveToTarget(DeltaTime);
		break;
	case EEnemyState::ATTACK:
		Attack();
		break;
	case EEnemyState::ATTACKDELAY:
		AttackDelay(DeltaTime);
		break;
	case EEnemyState::RETURN:
		ReturnHome(DeltaTime);
		break;
	case EEnemyState::DAMAGED:
		DamageProcess(DeltaTime);
		break;
	case EEnemyState::DIE:
		//Die();
		break;
	default:
		break;
	}

	DrawDebugSphere(GetWorld(), originLocation, limitDistance, 30, FColor::Green, false, 0, 0, 3);

	// 위젯 빌보드
	floatingWidgetComp->SetWorldRotation(BillboardWidgetComponent(target));
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::Idle(float deltaSeconds)
{
	if (target == nullptr)
	{
		return;
	}
	//UE_LOG(LogTemp, Warning, TEXT("Idle State"));

	// 5초가 지나면 상태를 MOVE 상태로 변경한다.
	//currentTime += deltaSeconds;

	//if (currentTime > 3.0f)
	//{
	//	currentTime = 0;
	//	enemyState = EEnemyState::MOVE;
	//	UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *UEnum::GetValueAsString<EEnemyState>(enemyState));
	//}

	// 자신의 전방을 기준으로 좌우 30도 이내, 거리 7미터의 범위에 플레이어 캐릭터가 접근하면 플레이어 캐릭터를 타겟으로 설정하고, 이동 상태로 전환한다.
	// 1. 찾은 플레이어가 거리가 7미터 이내인지 확인한다.
	float targetDistance = FVector::Distance(target->GetActorLocation(), GetActorLocation());

	// 2. 찾은 플레이어가 전방 좌우 30도 이내에 있는지 확인한다.
	FVector forwardVec = GetActorForwardVector();
	FVector directionVec = (target->GetActorLocation() - GetActorLocation()).GetSafeNormal();

	float cosTheta = FVector::DotProduct(forwardVec, directionVec);
	float theta_radian = FMath::Acos(cosTheta);
	float theta_degree = FMath::RadiansToDegrees(theta_radian);

#pragma region Debuging
	//if (cosTheta >= 0)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Target is located forward me"));
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Target is located back me"));
	//}

	//UE_LOG(LogTemp, Warning, TEXT("Degree: %.2f"), theta_degree);
#pragma endregion

	// 3. 두 조건을 모두 만족하면 이동 상태로 전환한다.
	if (targetDistance < sightDistance && cosTheta > 0 && theta_degree < sightAngle)
	{
		enemyState = EEnemyState::MOVE;
		UE_LOG(LogTemp, Warning, TEXT("I see target!"));

		// 2. 네비게이션 이동 방식
		// 만일, aiCon 변수의 값이 있다면...
		if (aiCon != nullptr)
		{
			// target 방향으로 네비게이션 경로대로 이동한다.
			EPathFollowingRequestResult::Type req = aiCon->MoveToActor(target);
			FString requestString = UEnum::GetValueAsString<EPathFollowingRequestResult::Type>(req);
			UE_LOG(LogTemp, Warning, TEXT("Nav Request: %s"), *requestString);

			//aiCon->MoveToLocation(target->GetActorLocation(), 10);
		}
	}
}

void AEnemy::MoveToTarget(float deltaSeconds)
{
	// 기준 위치로부터 추격 한계 거리 이상 떨어져 있다면...
	if (FVector::Distance(originLocation, GetActorLocation()) > limitDistance)
	{
		//aiCon->MoveToLocation(originLocation, 5, false);
		enemyState = EEnemyState::RETURN;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EEnemyState>()->GetValueAsString(enemyState));
		return;
	}

	//UE_LOG(LogTemp, Warning, TEXT("MoveToTarget State"));
	// 타겟이 있다면 타겟을 쫓아간다.
	// 필요 요소: 방향, 추격 속력, 공격 가능 거리(최대 접근 거리)
	//FVector targetDir = target->GetActorLocation() - GetActorLocation();
	//targetDir.Z = 0;

	//if (targetDir.Length() > attackDistance)
	if(FVector::Distance(target->GetActorLocation(), GetActorLocation()) > attackDistance)
	{
		// 1. 기본 이동 방식
		//GetCharacterMovement()->MaxWalkSpeed = traceSpeed;
		//AddMovementInput(targetDir.GetSafeNormal());
		//
		//// 이동 방향으로 회전한다.
		//FRotator currentRot = GetActorRotation();
		//FRotator targetRot = targetDir.ToOrientationRotator();

		//FRotator calcRot = FMath::Lerp(currentRot, targetRot, deltaSeconds * rotSpeed);
		//SetActorRotation(calcRot);
		
		// 타겟까지의 이동 경로를 시각화한다.
		UWorld* currentWorld = GetWorld();
		UNavigationSystemV1* navSystem = UNavigationSystemV1::GetCurrent(currentWorld);
		if (navSystem != nullptr)
		{
			//UNavigationPath* calcPath = navSystem->FindPathToLocationSynchronously(currentWorld, GetActorLocation(), target->GetActorLocation());

			UNavigationPath* calcPath = navSystem->FindPathToActorSynchronously(currentWorld, GetActorLocation(), target);
			TArray<FVector> paths = calcPath->PathPoints;

			if (paths.Num() > 1)
			{
				for (int32 i = 0; i < paths.Num() - 1; i++)
				{
					DrawDebugLine(currentWorld, paths[i] + FVector(0, 0, 80), paths[i + 1] + FVector(0, 0, 80), FColor::Red, false, 0, 0, 2);
				}
			}
		}
		if (aiCon != nullptr)
		{
			aiCon->MoveToLocation(target->GetActorLocation(), 5, true);
		}
	}
	else
	{
		aiCon->StopMovement();
		enemyState = EEnemyState::ATTACK;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EEnemyState>()->GetValueAsString(enemyState));
	}
}

void AEnemy::Attack()
{
	if (FVector::Distance(GetActorLocation(), target->GetActorLocation()) < attackDistance + 15.0f)
	{
		
		enemyState = EEnemyState::ATTACKDELAY;
	}
	else
	{
		aiCon->MoveToActor(target);
		enemyState = EEnemyState::MOVE;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EEnemyState>()->GetValueAsString(enemyState));
	}
}

void AEnemy::AttackDelay(float deltaSeconds)
{

	// 공격 대기 시간이 경과되면 다시 ATTACK 상태로 되돌린다.
	currentTime += deltaSeconds;

	if (currentTime > attackDelayTime)
	{
		currentTime = 0;
		enemyState = EEnemyState::ATTACK;
	}

	if (FVector::Distance(GetActorLocation(), target->GetActorLocation()) > attackDistance + 15.0f)
	{
		if (currentTime > attackDelayTime * 0.65f)
		{
			aiCon->MoveToActor(target, 10);
			enemyState = EEnemyState::MOVE;
		}
	}
}

void AEnemy::ReturnHome(float deltaSeconds)
{
	// 기준 위치로 돌아간다.
	// 필요 요소 : 기준 위치, 한계 거리, 복귀 속력
	
	FVector dir = originLocation - GetActorLocation();
	
	// 만일, 10센티미터 이내로 접근했다면...
	if (dir.Length() < 20)
	{
		aiCon->StopMovement();

		// 강제로 시작 위치로 이동시킨다.
		SetActorLocation(originLocation);
		SetActorRotation(originRotation);

		// 실행할 Idle 애니메이션을 결정한다.
		if (anim != nullptr)
		{
			anim->idleNumber = SelectIdleAnimation();
		}

		enemyState = EEnemyState::IDLE;
	}
	else
	{
		// 시작 위치 쪽으로 이동한다.
		//GetCharacterMovement()->MaxWalkSpeed = returnSpeed;
		//AddMovementInput(dir.GetSafeNormal());

		//FRotator lookRotation = FMath::Lerp(GetActorRotation(), dir.ToOrientationRotator(), deltaSeconds * rotSpeed);
		//SetActorRotation(lookRotation);

		UWorld* currentWorld = GetWorld();
		UNavigationSystemV1* navSystem = UNavigationSystemV1::GetCurrent(currentWorld);
		if (navSystem != nullptr)
		{
			//UNavigationPath* calcPath = navSystem->FindPathToLocationSynchronously(currentWorld, GetActorLocation(), target->GetActorLocation());

			UNavigationPath* calcPath = navSystem->FindPathToLocationSynchronously(currentWorld, GetActorLocation(), originLocation);
			TArray<FVector> paths = calcPath->PathPoints;

			if (paths.Num() > 1)
			{
				for (int32 i = 0; i < paths.Num() - 1; i++)
				{
					DrawDebugLine(currentWorld, paths[i] + FVector(0, 0, 80), paths[i + 1] + FVector(0, 0, 80), FColor::Red, false, 0, 0, 2);
				}
			}
		}

		// 돌아갈 때 다른 이동 중인 Enemy와 블럭 효과가 발생되는 문제
		aiCon->MoveToLocation(originLocation, 5, false, true, true);
	}

}

void AEnemy::OnDamaged(int32 dmg, AActor* attacker)
{
	if (enemyState == EEnemyState::DAMAGED)
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
		enemyState = EEnemyState::DAMAGED;
		hitLocation = GetActorLocation();
		hitDirection = GetActorLocation() - attacker->GetActorLocation();
		hitDirection.Z = 0;
		hitDirection.Normalize();
		
		// 머티리얼 색상에 붉은 색을 입힌다.
		dynamicMat->SetVectorParameterValue(FName("hitColor"), FVector4(1, 0, 0, 1));
	}
	// 그렇지 않으면...
	else
	{
		// 죽음 상태로 전환한다.
		enemyState = EEnemyState::DIE;
		Die();
	}
}


void AEnemy::DamageProcess(float deltaSeconds)
{
	currentTime += deltaSeconds;
	if (currentTime > 1.0f)
	{
		aiCon->MoveToActor(target);
		dynamicMat->SetVectorParameterValue(FName("hitColor"), FVector4(1, 1, 1, 1));
		enemyState = EEnemyState::MOVE;
		return;
	}

	// 피격 효과를 준다(넉백 효과 부여).
	//FVector backVec = GetActorForwardVector() * -1.0f;
	FVector targetLoc = hitLocation + hitDirection * 50.0f;
	FVector knockBackLocation = FMath::Lerp(GetActorLocation(), targetLoc, deltaSeconds * 7.0f);

	if (FVector::Distance(GetActorLocation(), targetLoc) > 10)
	{
		SetActorLocation(knockBackLocation, true);
	}
	else
	{
		// 머티리얼 색상에 흰색을 입힌다.
		aiCon->MoveToActor(target);
		dynamicMat->SetVectorParameterValue(FName("hitColor"), FVector4(1, 1, 1, 1));
		enemyState = EEnemyState::MOVE;
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
	ATPSPlayer* player = Cast<ATPSPlayer>(target);
	if (player != nullptr)
	{
		FVector lookDir = (player->cameraComp->GetComponentLocation() - floatingWidgetComp->GetComponentLocation()).GetSafeNormal();
		FRotator lookRot = UKismetMathLibrary::MakeRotFromX(lookDir);
		//FRotator lookRot = lookDir.ToOrientationRotator();

		return lookRot;
	}
	else
	{
		return FRotator::ZeroRotator;
	}
}

int32 AEnemy::SelectIdleAnimation()
{
	return FMath::RandRange(1, 4);
}

