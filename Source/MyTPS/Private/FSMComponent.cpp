// Fill out your copyright notice in the Description page of Project Settings.


#include "FSMComponent.h"
#include "Enemy.h"
#include "TPSPlayer.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "EnemyAnimInstance.h"
#include "TPSFunctionLibrary.h"


UFSMComponent::UFSMComponent()
{
	PrimaryComponentTick.bCanEverTick = true;


}


void UFSMComponent::BeginPlay()
{
	Super::BeginPlay();

	enemy = GetOwner<AEnemy>();
	aiCon = enemy->GetController<AAIController>();

	// 기본 상태를 IDLE 상태로 초기화한다.
	enemyState = EEnemyState::IDLE;

	// 시작 위치를 정한다.
	originLocation = enemy->GetActorLocation();
	originRotation = enemy->GetActorRotation();
}


void UFSMComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

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
	case EEnemyState::DAMAGED_BOMB:

		break;
	case EEnemyState::DIE:
		//Die();
		break;
	default:
		break;
	}

	DrawDebugSphere(GetWorld(), originLocation, limitDistance, 30, FColor::Green, false, 0, 0, 3);
}

void UFSMComponent::Idle(float deltaSeconds)
{
	// 자신의 전방을 기준으로 좌우 30도 이내, 거리 7미터의 범위에 플레이어 캐릭터가 접근하면 플레이어 캐릭터를 타겟으로 설정하고, 이동 상태로 전환한다.

	// <1. 타겟 감지>
	// sightDistance를 반경으로 주변에 있는 ATPSPlayer 클래스를 검색한다. -> TArray<ATPSPlayer>
	// OverlapMulti 방식을 이용한다.
	TArray<AActor*> foundActors = UTPSFunctionLibrary::SearchAroundActor(enemy, sightDistance, ECC_Pawn, enemy->GetWorld());

	if (foundActors.Num() > 0)
	{
		for (AActor* fActor : foundActors)
		{
			ATPSPlayer* player = Cast<ATPSPlayer>(fActor);
			if (player != nullptr && player->tpsPlayerState == EPlayerState::PLAYING)
			{
				// <2. 시야각 체크>
				// 찾은 플레이어가 전방 좌우 30도 이내에 있는지 확인한다.
				float degree = UTPSFunctionLibrary::CheckSight(enemy, player);

				if (degree < sightAngle)
				{
					enemyState = EEnemyState::MOVE;
					UE_LOG(LogTemp, Warning, TEXT("I see target!"));

					target = player;

					// <네비게이션 이동 방식>
					// 만일, aiCon 변수의 값이 있다면...
					if (aiCon != nullptr)
					{
						// target 방향으로 네비게이션 경로대로 이동한다.
						EPathFollowingRequestResult::Type req = aiCon->MoveToActor(target);
						FString requestString = UEnum::GetValueAsString<EPathFollowingRequestResult::Type>(req);
						UE_LOG(LogTemp, Warning, TEXT("Nav Request: %s"), *requestString);
					}

					break;
				}
			}
		}
	}
	else
	{
		target = nullptr;
	}
}

void UFSMComponent::MoveToTarget(float deltaSeconds)
{
	// 기준 위치로부터 추격 한계 거리 이상 떨어져 있다면...
	if (FVector::Distance(originLocation, enemy->GetActorLocation()) > limitDistance)
	{
		//aiCon->MoveToLocation(originLocation, 5, false);
		enemyState = EEnemyState::RETURN;
		//UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EEnemyState>()->GetValueAsString(enemyState));
		return;
	}

	//UE_LOG(LogTemp, Warning, TEXT("MoveToTarget State"));
	// 타겟이 있다면 타겟을 쫓아간다.
	// 필요 요소: 방향, 추격 속력, 공격 가능 거리(최대 접근 거리)
	//FVector targetDir = target->GetActorLocation() - GetActorLocation();
	//targetDir.Z = 0;

	//if (targetDir.Length() > attackDistance)
	if (FVector::Distance(target->GetActorLocation(), enemy->GetActorLocation()) > attackDistance)
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
			UNavigationPath* calcPath = navSystem->FindPathToActorSynchronously(currentWorld, enemy->GetActorLocation(), target);
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

void UFSMComponent::Attack()
{
	// 타겟이 이미 사망 상태일 경우 즉시 Return 상태로 전환한다.
	if (Cast<ATPSPlayer>(target)->tpsPlayerState == EPlayerState::DEATH)
	{
		target = nullptr;
		enemyState = EEnemyState::RETURN;
		return;
	}

	if (FVector::Distance(enemy->GetActorLocation(), target->GetActorLocation()) < attackDistance + 15.0f)
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

void UFSMComponent::AttackDelay(float deltaSeconds)
{
	// 타겟이 이미 사망 상태일 경우 즉시 Return 상태로 전환한다.
	if (Cast<ATPSPlayer>(target)->tpsPlayerState == EPlayerState::DEATH)
	{
		target = nullptr;
		enemyState = EEnemyState::RETURN;
		return;
	}

	// 공격 대기 시간이 경과되면 다시 ATTACK 상태로 되돌린다.
	currentTime += deltaSeconds;

	if (currentTime > attackDelayTime)
	{
		currentTime = 0;
		enemyState = EEnemyState::ATTACK;
	}

	if (FVector::Distance(enemy->GetActorLocation(), target->GetActorLocation()) > attackDistance + 15.0f)
	{
		if (currentTime > attackDelayTime * 0.65f)
		{
			aiCon->MoveToActor(target, 10);
			enemyState = EEnemyState::MOVE;
		}
	}
}

void UFSMComponent::ReturnHome(float deltaSeconds)
{
	// 기준 위치로 돌아간다.
	// 필요 요소 : 기준 위치, 한계 거리, 복귀 속력

	FVector dir = originLocation - enemy->GetActorLocation();

	// 만일, 10센티미터 이내로 접근했다면...
	if (dir.Length() < 20)
	{
		aiCon->StopMovement();

		// 강제로 시작 위치로 이동시킨다.
		enemy->SetActorLocation(originLocation);
		enemy->SetActorRotation(originRotation);

		// 실행할 Idle 애니메이션을 결정한다.
		if (enemy->GetEnemyAnim() != nullptr)
		{
			enemy->GetEnemyAnim()->idleNumber = SelectIdleAnimation();
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

			UNavigationPath* calcPath = navSystem->FindPathToLocationSynchronously(currentWorld, enemy->GetActorLocation(), originLocation);
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

void UFSMComponent::DamageProcess(float deltaSeconds)
{
	currentTime += deltaSeconds;
	if (currentTime > 1.0f)
	{
		aiCon->MoveToActor(target);
		enemy->dynamicMat->SetVectorParameterValue(FName("hitColor"), FVector4(1, 1, 1, 1));
		enemyState = EEnemyState::MOVE;
		return;
	}

	// 피격 효과를 준다(넉백 효과 부여).
	//FVector backVec = GetActorForwardVector() * -1.0f;
	FVector knockBackLocation = FMath::Lerp(enemy->GetActorLocation(), targetLoc, deltaSeconds * 7.0f);

	if (FVector::Distance(enemy->GetActorLocation(), targetLoc) > 10)
	{
		enemy->SetActorLocation(knockBackLocation, true);
	}
	else
	{
		// 머티리얼 색상에 흰색을 입힌다.
		aiCon->MoveToActor(target);
		enemy->dynamicMat->SetVectorParameterValue(FName("hitColor"), FVector4(1, 1, 1, 1));
		enemyState = EEnemyState::MOVE;
	}
}

void UFSMComponent::SetNewTarget(AActor* newActor)
{
	target = newActor;
}

int32 UFSMComponent::SelectIdleAnimation()
{
	return FMath::RandRange(1, 4);
}
