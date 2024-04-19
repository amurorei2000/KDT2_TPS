// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Enemy.h"


AGrenadeActor::AGrenadeActor()
{
	PrimaryActorTick.bCanEverTick = false;

	sphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Component"));
	SetRootComponent(sphereComp);
	sphereComp->SetSphereRadius(10);
	sphereComp->SetCollisionProfileName(FName("Grenade"));
	sphereComp->SetNotifyRigidBodyCollision(true);

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	meshComp->SetupAttachment(RootComponent);
	meshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	meshComp->SetRelativeLocation(FVector(0, 0, -8));
	meshComp->SetRelativeScale3D(FVector(0.2f));

}

void AGrenadeActor::BeginPlay()
{
	Super::BeginPlay();
	
	// Block 충돌 이벤트에 함수를 바인딩하기
	sphereComp->OnComponentHit.AddDynamic(this, &AGrenadeActor::BombAction);

	SetLifeSpan(5.0f);
}

void AGrenadeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGrenadeActor::Throw(const FVector& throwDir, float throwPower)
{
	// 피직스가 off 상태라면 on 상태로 전환한다.
	if (!sphereComp->IsSimulatingPhysics())
	{
		sphereComp->SetSimulatePhysics(true);
	}

	sphereComp->AddImpulse(throwDir * throwPower);
}

// Block으로 부딪혔을 때 실행되는 함수
void AGrenadeActor::BombAction(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("Hit Event!!!"));

	// damageRadius 주변에 Enemy들을 모두 찾는다.
	TArray<FOverlapResult> hitInfos;
	FCollisionObjectQueryParams objectParams;
	objectParams.AddObjectTypesToQuery(affectType);
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this);

	bool result = GetWorld()->OverlapMultiByObjectType(hitInfos, GetActorLocation(), FQuat::Identity, objectParams, FCollisionShape::MakeSphere(damageRadius), queryParams);

	if (result)
	{
		// 각 Enemy들을 순회하면서
		for (const FOverlapResult& hitInfo : hitInfos)
		{
			// Enemy의 HitBomb 함수를 실행시킨다.
			AEnemy* enemy = Cast<AEnemy>(hitInfo.GetActor());

			if (enemy != nullptr)
			{
				//UE_LOG(LogTemp, Warning, TEXT("%s is damaged!"), *enemy->GetActorNameOrLabel());
				enemy->HitBomb(damagePower, GetActorLocation() - enemy->GetActorLocation(), damageRadius, UpForce);
			}
		}
	}

	Destroy();
}

