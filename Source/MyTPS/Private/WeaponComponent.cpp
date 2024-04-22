// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComponent.h"
#include "EnhancedInputComponent.h"
#include "TPSPlayer.h"
#include "WeaponActor.h"
#include "Camera/CameraComponent.h"
#include "BulletFXActor.h"
#include "Enemy.h"
#include "PlayerAnimInstance.h"
#include "KismetTraceUtils.h"
#include "TPSFunctionLibrary.h"
#include "GrenadeActor.h"
#include "BombDecalActor.h"
#include "GameFramework/SpringArmComponent.h"
#include "TPSMainGameModeBase.h"
#include "MainWidget.h"
#include "EngineUtils.h"


UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	player = GetOwner<ATPSPlayer>();
	gm = Cast<ATPSMainGameModeBase>(GetWorld()->GetAuthGameMode());

	// 월드에 있는 총알 피격 효과 액터를 찾아서 변수에 참조시킨다.
	for (TActorIterator<ABulletFXActor> iter(GetWorld()); iter; ++iter)
	{
		bulletFX = *iter;
	}

	// 폭발 범위 표시 데칼 액터 생성하기
	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	bombDecal_inst = GetWorld()->SpawnActor<ABombDecalActor>(bombDecal_bp, player->GetActorLocation(), FRotator::ZeroRotator, params);
	if (bombDecal_inst != nullptr)
	{
		bombDecal_inst->SetShowDecal(false);
	}
}


void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 카메라 줌 인 아웃 처리
	float direction = bZoomIn ? 1.0f : -1.0f;
	alpha += DeltaTime * direction * 5.0f;
	alpha = FMath::Clamp(alpha, 0.0f, 1.0f);

	float result = FMath::Lerp(90, 40, alpha);
	player->cameraComp->SetFieldOfView(result);

	// 수류탄의 궤적을 그린다.
	if (bShowLine)
	{
		throwDir = player->GetActorForwardVector() + player->GetActorUpVector();
		FVector modifiedDir = player->cameraComp->GetComponentTransform().TransformVector(throwDir);
		
#pragma region UGameplayStatics Style
		// 1. UGameplayStatics 클래스의 함수를 이용한 방법

		//FPredictProjectilePathParams params;
		//params.StartLocation = GetActorLocation() + GetActorForwardVector() * 100;
		//params.LaunchVelocity = modifiedDir * throwPower;
		//params.bTraceWithChannel = true;
		//params.TraceChannel = ECC_Visibility;
		//params.DrawDebugTime = 0;
		//params.DrawDebugType = EDrawDebugTrace::Type::ForOneFrame;
		//params.MaxSimTime = 3.0f;
		//params.SimFrequency = 50;
		//params.OverrideGravityZ = -980;

		//FPredictProjectilePathResult pathResults;
		//UGameplayStatics::PredictProjectilePath(GetWorld(), params, pathResults);

		//for (int32 i = 0; i < pathResults.PathData.Num() - 1; i++)
		//{
		//	DrawDebugLine(GetWorld(), pathResults.PathData[i].Location, pathResults.PathData[i + 1].Location, FColor::Red, false, 0, 0, 3);
		//}
#pragma endregion

		// 2. 직접 계산하는 방식
		TArray<FVector> results = UTPSFunctionLibrary::CalculateThrowPoints(player, modifiedDir, throwPower, 0.1f, 3.0f, GetWorld()->GetDefaultGravityZ());

		FVector finalLoc;

		for (int32 i = 0; i < results.Num() - 1; i++)
		{
			FHitResult hitInfo;

			if (GetWorld()->LineTraceSingleByChannel(hitInfo, results[i], results[i + 1], ECC_Visibility))
			{
				DrawDebugLine(GetWorld(), results[i], hitInfo.ImpactPoint, FColor::Black, false, 0, 0, 3);
				finalLoc = hitInfo.ImpactPoint;
				break;
			}
			else
			{
				finalLoc = results[i + 1];
				DrawDebugLine(GetWorld(), results[i], results[i + 1], FColor::Black, false, 0, 0, 3);
			}
		}

		if (bombDecal_inst != nullptr)
		{
			bombDecal_inst->SetActorLocation(finalLoc);
			bombDecal_inst->SetShowDecal(true);
		}
	}
	else
	{
		bombDecal_inst->SetShowDecal(false);
	}
}

void UWeaponComponent::SetupPlayerInputComponent(UEnhancedInputComponent* enhancedInputComponent)
{
	enhancedInputComponent->BindAction(ia_fire, ETriggerEvent::Started, this, &UWeaponComponent::PlayerFire);
	enhancedInputComponent->BindAction(ia_alpha1, ETriggerEvent::Started, this, &UWeaponComponent::SetWeapon1);
	enhancedInputComponent->BindAction(ia_alpha2, ETriggerEvent::Started, this, &UWeaponComponent::SetWeapon2);
	enhancedInputComponent->BindAction(ia_aimFocusing, ETriggerEvent::Started, this, &UWeaponComponent::SniperGunZoomInOut);
	enhancedInputComponent->BindAction(ia_aimFocusing, ETriggerEvent::Completed, this, &UWeaponComponent::SniperGunZoomInOut);
	enhancedInputComponent->BindAction(ia_releaseWeapon, ETriggerEvent::Started, this, &UWeaponComponent::ReleaseAction);
}

void UWeaponComponent::PlayerFire(const FInputActionValue& value)
{
	// 총을 들고 있지 않거나 또는 총을 발사하는 애니메이션 중일 때에는 이 함수를 종료한다.
	if (player->attachedWeapon == nullptr || GetWorld()->GetTimerManager().IsTimerActive(endFireTimer))
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon: %s"), player->attachedWeapon == nullptr ? *FString("Null") : *FString("Has"));
		return;
	}

	// 라인 트레이스 방식
	FHitResult hitInfo;
	FVector startLoc = player->cameraComp->GetComponentLocation();
	FVector endLoc = startLoc + player->cameraComp->GetForwardVector() * player->attachedWeapon->fireDistance;
	// 충돌 체크에 포함할 오브젝트 타입
	FCollisionObjectQueryParams objQueryParams;
	objQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	objQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	// 충돌 체크에서 제외할 액터 또는 컴포넌트
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(player);

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
			enemy->OnDamaged(player->attachedWeapon->damage, player);
			
		}
	}
	//else
	//{
		//DrawDebugLine(GetWorld(), startLoc, endLoc, FColor(255, 0, 0), false, 2.0f, 0, 1.0f);
	//}

	if (player->GetPlayerAnim() != nullptr)
	{
		player->GetPlayerAnim()->bFire = true;

		GetWorld()->GetTimerManager().SetTimer(endFireTimer, this, &UWeaponComponent::EndFire, 0.5f, false);
	}

	if (fire_montages.Num() > 1)
	{
		if (player->attachedWeapon->bSniperGun)
		{
			player->PlayAnimMontage(fire_montages[1]);
		}
		else
		{
			player->PlayAnimMontage(fire_montages[0]);
		}
	}
}

void UWeaponComponent::PlayerFire2(const FInputActionValue& value)
{
	FHitResult hitInfo;
	FVector startLoc = player->GetActorLocation();
	FVector endLoc = startLoc + player->GetActorForwardVector() * 1000.0f;
	FQuat startRot = FRotator(0, 0, 45).Quaternion();
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(player);

	// 정육면체를 45도 회전시킨 상태로 발사한다.
	bool bResult = GetWorld()->SweepSingleByChannel(hitInfo, startLoc, endLoc, startRot, ECC_Visibility, FCollisionShape::MakeBox(FVector(10)), queryParams);

	if (bResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit Actor name: %s"), *hitInfo.GetActor()->GetActorNameOrLabel());
		FVector centerPos = (startLoc + endLoc) * 0.5f;
	}

	DrawDebugBoxTraceSingle(GetWorld(), startLoc, endLoc, FVector(10), FRotator(0, 0, 45), EDrawDebugTrace::ForDuration, true, hitInfo, FLinearColor::Green, FLinearColor::Red, 2.0f);
}

void UWeaponComponent::SetWeapon1(const FInputActionValue& value)
{
	currentWeaponNumber = 0;
	ChangeGunType(currentWeaponNumber);
}

void UWeaponComponent::SetWeapon2(const FInputActionValue& value)
{
	currentWeaponNumber = 1;
	ChangeGunType(currentWeaponNumber);
}

void UWeaponComponent::SniperGunZoomInOut(const FInputActionValue& value)
{
	bool inputValue = value.Get<bool>();

	// 노멀 건일 때
	if (currentWeaponNumber == 0)
	{
		bShowLine = inputValue;

		if (!inputValue)
		{
			// 수류탄을 생성 및 발사한다.
			FActorSpawnParameters params;
			params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AGrenadeActor* bomb = GetWorld()->SpawnActor<AGrenadeActor>(grenade_bp, player->GetActorLocation() + player->GetActorForwardVector() * 100, FRotator::ZeroRotator, params);

			FVector modifiedDir = player->cameraComp->GetComponentTransform().TransformVector(throwDir);
			bomb->Throw(modifiedDir, throwPower);
		}
	}
	// 스나이퍼 건일 때
	else if (currentWeaponNumber == 1)
	{
		if (inputValue)
		{
			player->springArmComp->TargetArmLength = -100;
		}
		else
		{
			player->springArmComp->TargetArmLength = 300;
		}

		// zoom in 효과 on/off 설정
		bZoomIn = inputValue;

		if (gm != nullptr && gm->mainWidget_inst != nullptr)
		{
			gm->mainWidget_inst->SetSniperMode(!inputValue);
		}
	}
}

void UWeaponComponent::ReleaseAction(const FInputActionValue& value)
{
	if (player->attachedWeapon == nullptr)
	{
		return;
	}
	player->attachedWeapon->Release();
	player->attachedWeapon = nullptr;
	currentWeaponNumber = 0;
}

void UWeaponComponent::EndFire()
{
	if (player->GetPlayerAnim() != nullptr)
	{
		player->GetPlayerAnim()->bFire = false;
	}
}

void UWeaponComponent::ChangeGunType(int32 number)
{
	// 메시 변경
	player->gunMeshComp->SetStaticMesh(player->gunTypes[number]);
	player->gunMeshComp->SetRelativeLocation(player->gunOffset[number]);

	// UI 텍스쳐 변경
	if (gm != nullptr)
	{
		gm->mainWidget_inst->SetWeaponTexture(number);
	}
}

void UWeaponComponent::SetCurrentWeaponNumber(bool bSniper)
{
	currentWeaponNumber = (int32)bSniper;
}