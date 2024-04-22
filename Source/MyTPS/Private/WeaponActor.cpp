// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "TPSPlayer.h"
#include "WeaponComponent.h"


AWeaponActor::AWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;

	boxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Component"));
	SetRootComponent(boxComp);
	boxComp->SetCollisionProfileName(FName("WeaponPreset"));
	boxComp->SetSimulatePhysics(true);
	boxComp->SetEnableGravity(true);


	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun Mesh Component"));
	meshComp->SetupAttachment(RootComponent);
	meshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeaponActor::BeginPlay()
{
	Super::BeginPlay();

	boxComp->OnComponentBeginOverlap.AddDynamic(this, &AWeaponActor::OnOverlapPlayer);
}

void AWeaponActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponActor::Release()
{
	FDetachmentTransformRules rules = FDetachmentTransformRules::KeepWorldTransform;
	DetachFromActor(rules);
	boxComp->SetSimulatePhysics(true);
}

void AWeaponActor::OnOverlapPlayer(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("OtherBodyIndex: %d"), OtherBodyIndex);

	// 충돌한 대상이 플레이어라면...
	ATPSPlayer* player = Cast<ATPSPlayer>(OtherActor);

	if (player != nullptr)
	{
		// 만일, 플레이어가 현재 총을 들고 있지 않은 상태라면...
		if (player->attachedWeapon == nullptr)
		{
			// 자신의 Physics 옵션을 꺼준다.
			boxComp->SetSimulatePhysics(false);

			// 플레이어의 bodyMesh쪽에 있는 "GunSocket"이라는 이름의 소켓에 자기 자신을 부착시킨다.
			FAttachmentTransformRules attachRules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
			// 부착될 당시의 플레이어와 자기 자신과의 상대적 거리, 회전, 크기를 Relative Transform 값에 적용시킨다.
			//FAttachmentTransformRules attachRules = FAttachmentTransformRules::KeepWorldTransform;
			// 부착될 당시의 자기 자신의 world transform 값을 relative transform 값으로 적용시킨다.
			//FAttachmentTransformRules attachRules = FAttachmentTransformRules::KeepRelativeTransform;

			AttachToComponent(player->GetMesh(), attachRules, FName("GunSocket"));
			SetActorRelativeLocation(snapOffset);

			// 플레이어의 attachedWeapon 변수에 자기 자신을 등록한다.
			player->attachedWeapon = this;

			// 플레이어 애니메이션 인스턴스의 useSniper 변수 값을 변경한다.
			player->SetGunAnimType(bSniperGun);

			player->weaponComp->SetCurrentWeaponNumber(bSniperGun);
		}
	}
}

