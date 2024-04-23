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

	// �⺻ ���¸� IDLE ���·� �ʱ�ȭ�Ѵ�.
	enemyState = EEnemyState::IDLE;

	// ���� ��ġ�� ���Ѵ�.
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
	// �ڽ��� ������ �������� �¿� 30�� �̳�, �Ÿ� 7������ ������ �÷��̾� ĳ���Ͱ� �����ϸ� �÷��̾� ĳ���͸� Ÿ������ �����ϰ�, �̵� ���·� ��ȯ�Ѵ�.

	// <1. Ÿ�� ����>
	// sightDistance�� �ݰ����� �ֺ��� �ִ� ATPSPlayer Ŭ������ �˻��Ѵ�. -> TArray<ATPSPlayer>
	// OverlapMulti ����� �̿��Ѵ�.
	TArray<AActor*> foundActors = UTPSFunctionLibrary::SearchAroundActor(enemy, sightDistance, ECC_Pawn, enemy->GetWorld());

	if (foundActors.Num() > 0)
	{
		for (AActor* fActor : foundActors)
		{
			ATPSPlayer* player = Cast<ATPSPlayer>(fActor);
			if (player != nullptr && player->tpsPlayerState == EPlayerState::PLAYING)
			{
				// <2. �þ߰� üũ>
				// ã�� �÷��̾ ���� �¿� 30�� �̳��� �ִ��� Ȯ���Ѵ�.
				float degree = UTPSFunctionLibrary::CheckSight(enemy, player);

				if (degree < sightAngle)
				{
					enemyState = EEnemyState::MOVE;
					UE_LOG(LogTemp, Warning, TEXT("I see target!"));

					target = player;

					// <�׺���̼� �̵� ���>
					// ����, aiCon ������ ���� �ִٸ�...
					if (aiCon != nullptr)
					{
						// target �������� �׺���̼� ��δ�� �̵��Ѵ�.
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
	// ���� ��ġ�κ��� �߰� �Ѱ� �Ÿ� �̻� ������ �ִٸ�...
	if (FVector::Distance(originLocation, enemy->GetActorLocation()) > limitDistance)
	{
		//aiCon->MoveToLocation(originLocation, 5, false);
		enemyState = EEnemyState::RETURN;
		//UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EEnemyState>()->GetValueAsString(enemyState));
		return;
	}

	//UE_LOG(LogTemp, Warning, TEXT("MoveToTarget State"));
	// Ÿ���� �ִٸ� Ÿ���� �Ѿư���.
	// �ʿ� ���: ����, �߰� �ӷ�, ���� ���� �Ÿ�(�ִ� ���� �Ÿ�)
	//FVector targetDir = target->GetActorLocation() - GetActorLocation();
	//targetDir.Z = 0;

	//if (targetDir.Length() > attackDistance)
	if (FVector::Distance(target->GetActorLocation(), enemy->GetActorLocation()) > attackDistance)
	{
		// 1. �⺻ �̵� ���
		//GetCharacterMovement()->MaxWalkSpeed = traceSpeed;
		//AddMovementInput(targetDir.GetSafeNormal());
		//
		//// �̵� �������� ȸ���Ѵ�.
		//FRotator currentRot = GetActorRotation();
		//FRotator targetRot = targetDir.ToOrientationRotator();

		//FRotator calcRot = FMath::Lerp(currentRot, targetRot, deltaSeconds * rotSpeed);
		//SetActorRotation(calcRot);

		// Ÿ�ٱ����� �̵� ��θ� �ð�ȭ�Ѵ�.
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
	// Ÿ���� �̹� ��� ������ ��� ��� Return ���·� ��ȯ�Ѵ�.
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
	// Ÿ���� �̹� ��� ������ ��� ��� Return ���·� ��ȯ�Ѵ�.
	if (Cast<ATPSPlayer>(target)->tpsPlayerState == EPlayerState::DEATH)
	{
		target = nullptr;
		enemyState = EEnemyState::RETURN;
		return;
	}

	// ���� ��� �ð��� ����Ǹ� �ٽ� ATTACK ���·� �ǵ�����.
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
	// ���� ��ġ�� ���ư���.
	// �ʿ� ��� : ���� ��ġ, �Ѱ� �Ÿ�, ���� �ӷ�

	FVector dir = originLocation - enemy->GetActorLocation();

	// ����, 10��Ƽ���� �̳��� �����ߴٸ�...
	if (dir.Length() < 20)
	{
		aiCon->StopMovement();

		// ������ ���� ��ġ�� �̵���Ų��.
		enemy->SetActorLocation(originLocation);
		enemy->SetActorRotation(originRotation);

		// ������ Idle �ִϸ��̼��� �����Ѵ�.
		if (enemy->GetEnemyAnim() != nullptr)
		{
			enemy->GetEnemyAnim()->idleNumber = SelectIdleAnimation();
		}

		enemyState = EEnemyState::IDLE;
	}
	else
	{
		// ���� ��ġ ������ �̵��Ѵ�.
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

		// ���ư� �� �ٸ� �̵� ���� Enemy�� �� ȿ���� �߻��Ǵ� ����
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

	// �ǰ� ȿ���� �ش�(�˹� ȿ�� �ο�).
	//FVector backVec = GetActorForwardVector() * -1.0f;
	FVector knockBackLocation = FMath::Lerp(enemy->GetActorLocation(), targetLoc, deltaSeconds * 7.0f);

	if (FVector::Distance(enemy->GetActorLocation(), targetLoc) > 10)
	{
		enemy->SetActorLocation(knockBackLocation, true);
	}
	else
	{
		// ��Ƽ���� ���� ����� ������.
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
