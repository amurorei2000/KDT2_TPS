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

	// AI Controller�� �ڵ� Possess ��� ������ ���忡 ��ġ�Ǿ��� �� �Ǵ� ���� ���� �� �����ϴ� ������ �����Ѵ�.
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	// �⺻ ���¸� IDLE ���·� �ʱ�ȭ�Ѵ�.
	enemyState = EEnemyState::IDLE;

	// �÷����� Idle �ִϸ��̼� ��ȣ�� �����ؼ� EnemyAnimInstance�� idleNumber ������ �����Ѵ�.
	anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance());


	// ���忡 �ִ� �÷��̾ ã�´�.
	for (TActorIterator<ATPSPlayer> player(GetWorld()); player; ++player)
	{
		target = *player;
	}

	// ���� ��ġ�� ���Ѵ�.
	originLocation = GetActorLocation();
	originRotation = GetActorRotation();

	// ü�� ������ �ʱ�ȭ�Ѵ�.
	currentHP = maxHP;

	// �޽��� ��Ƽ������ DynamicMaterial�� �������ش�.
	UMaterialInterface* currentMat = GetMesh()->GetMaterial(0);
	dynamicMat = UMaterialInstanceDynamic::Create(currentMat, nullptr);
	GetMesh()->SetMaterial(0, dynamicMat);

	// ���� ������Ʈ�� �Ҵ�Ǿ� �ִ� ���� �ν��Ͻ��� �����´�.
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

	// ���� ������
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

	// 5�ʰ� ������ ���¸� MOVE ���·� �����Ѵ�.
	//currentTime += deltaSeconds;

	//if (currentTime > 3.0f)
	//{
	//	currentTime = 0;
	//	enemyState = EEnemyState::MOVE;
	//	UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *UEnum::GetValueAsString<EEnemyState>(enemyState));
	//}

	// �ڽ��� ������ �������� �¿� 30�� �̳�, �Ÿ� 7������ ������ �÷��̾� ĳ���Ͱ� �����ϸ� �÷��̾� ĳ���͸� Ÿ������ �����ϰ�, �̵� ���·� ��ȯ�Ѵ�.
	// 1. ã�� �÷��̾ �Ÿ��� 7���� �̳����� Ȯ���Ѵ�.
	float targetDistance = FVector::Distance(target->GetActorLocation(), GetActorLocation());

	// 2. ã�� �÷��̾ ���� �¿� 30�� �̳��� �ִ��� Ȯ���Ѵ�.
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

	// 3. �� ������ ��� �����ϸ� �̵� ���·� ��ȯ�Ѵ�.
	if (targetDistance < sightDistance && cosTheta > 0 && theta_degree < sightAngle)
	{
		enemyState = EEnemyState::MOVE;
		UE_LOG(LogTemp, Warning, TEXT("I see target!"));

		// 2. �׺���̼� �̵� ���
		// ����, aiCon ������ ���� �ִٸ�...
		if (aiCon != nullptr)
		{
			// target �������� �׺���̼� ��δ�� �̵��Ѵ�.
			EPathFollowingRequestResult::Type req = aiCon->MoveToActor(target);
			FString requestString = UEnum::GetValueAsString<EPathFollowingRequestResult::Type>(req);
			UE_LOG(LogTemp, Warning, TEXT("Nav Request: %s"), *requestString);

			//aiCon->MoveToLocation(target->GetActorLocation(), 10);
		}
	}
}

void AEnemy::MoveToTarget(float deltaSeconds)
{
	// ���� ��ġ�κ��� �߰� �Ѱ� �Ÿ� �̻� ������ �ִٸ�...
	if (FVector::Distance(originLocation, GetActorLocation()) > limitDistance)
	{
		//aiCon->MoveToLocation(originLocation, 5, false);
		enemyState = EEnemyState::RETURN;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EEnemyState>()->GetValueAsString(enemyState));
		return;
	}

	//UE_LOG(LogTemp, Warning, TEXT("MoveToTarget State"));
	// Ÿ���� �ִٸ� Ÿ���� �Ѿư���.
	// �ʿ� ���: ����, �߰� �ӷ�, ���� ���� �Ÿ�(�ִ� ���� �Ÿ�)
	//FVector targetDir = target->GetActorLocation() - GetActorLocation();
	//targetDir.Z = 0;

	//if (targetDir.Length() > attackDistance)
	if(FVector::Distance(target->GetActorLocation(), GetActorLocation()) > attackDistance)
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

	// ���� ��� �ð��� ����Ǹ� �ٽ� ATTACK ���·� �ǵ�����.
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
	// ���� ��ġ�� ���ư���.
	// �ʿ� ��� : ���� ��ġ, �Ѱ� �Ÿ�, ���� �ӷ�
	
	FVector dir = originLocation - GetActorLocation();
	
	// ����, 10��Ƽ���� �̳��� �����ߴٸ�...
	if (dir.Length() < 20)
	{
		aiCon->StopMovement();

		// ������ ���� ��ġ�� �̵���Ų��.
		SetActorLocation(originLocation);
		SetActorRotation(originRotation);

		// ������ Idle �ִϸ��̼��� �����Ѵ�.
		if (anim != nullptr)
		{
			anim->idleNumber = SelectIdleAnimation();
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

		// ���ư� �� �ٸ� �̵� ���� Enemy�� �� ȿ���� �߻��Ǵ� ����
		aiCon->MoveToLocation(originLocation, 5, false, true, true);
	}

}

void AEnemy::OnDamaged(int32 dmg, AActor* attacker)
{
	if (enemyState == EEnemyState::DAMAGED)
	{
		return;
	}
	
	// ���� ü�� ����
	currentHP = FMath::Clamp(currentHP - dmg, 0, maxHP);
	if (healthWidget != nullptr)
	{
		healthWidget->SetHealthBar((float)currentHP / (float)maxHP, FLinearColor(1.0f, 0.138f, 0.059f, 1.0f));
	}

	// ������ ��� ��� ���� ü���� 0���� ũ�ٸ�....
	if (currentHP > 0)
	{
		// �ǰ� ���·� ��ȯ�Ѵ�.
		enemyState = EEnemyState::DAMAGED;
		hitLocation = GetActorLocation();
		hitDirection = GetActorLocation() - attacker->GetActorLocation();
		hitDirection.Z = 0;
		hitDirection.Normalize();
		
		// ��Ƽ���� ���� ���� ���� ������.
		dynamicMat->SetVectorParameterValue(FName("hitColor"), FVector4(1, 0, 0, 1));
	}
	// �׷��� ������...
	else
	{
		// ���� ���·� ��ȯ�Ѵ�.
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

	// �ǰ� ȿ���� �ش�(�˹� ȿ�� �ο�).
	//FVector backVec = GetActorForwardVector() * -1.0f;
	FVector targetLoc = hitLocation + hitDirection * 50.0f;
	FVector knockBackLocation = FMath::Lerp(GetActorLocation(), targetLoc, deltaSeconds * 7.0f);

	if (FVector::Distance(GetActorLocation(), targetLoc) > 10)
	{
		SetActorLocation(knockBackLocation, true);
	}
	else
	{
		// ��Ƽ���� ���� ����� ������.
		aiCon->MoveToActor(target);
		dynamicMat->SetVectorParameterValue(FName("hitColor"), FVector4(1, 1, 1, 1));
		enemyState = EEnemyState::MOVE;
	}
}


void AEnemy::Die()
{
	// �ݸ����� NoCollision ���·� ��ȯ�Ѵ�.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// CharacterMovementComponent�� ��Ȱ��ȭ�Ѵ�.
	GetCharacterMovement()->DisableMovement();

	// ���� �ִϸ��̼� ��Ÿ�긦 �����Ѵ�.
	// 1~3 ������ ������ ���ڸ� �ϳ� �̴´�.
	int32 num = FMath::RandRange(1, 3);
	// "Dead" + ���ڷ� ��Ÿ���� ���� �̸��� �����.
	FString sectionName = FString("Dead") + FString::FromInt(num);
	// ���� �̸��� �̿��ؼ� ��Ÿ�ָ� �÷����Ѵ�.
	PlayAnimMontage(death_montage, 1, FName(sectionName));

	// 3�� �ڿ� �����Ѵ�.
	/*FTimerHandle deadHandler;

	GetWorldTimerManager().SetTimer(deadHandler, FTimerDelegate::CreateLambda([&]() {
		Destroy();
		}), 5.6f, false);*/
}

// ī�޶� ������Ʈ�� ������ �ִ� ���͸� ���� ������Ʈ�� �ٶ󺸵��� ȸ�� ���� ������ִ� �Լ�
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

