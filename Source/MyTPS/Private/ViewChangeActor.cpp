// Fill out your copyright notice in the Description page of Project Settings.


#include "ViewChangeActor.h"
#include "Components/BoxComponent.h"
#include "TPSPlayer.h"


AViewChangeActor::AViewChangeActor()
{
	PrimaryActorTick.bCanEverTick = false;

	rootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	SetRootComponent(rootComp);

	triggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger Component"));
	triggerBox->SetupAttachment(RootComponent);
	triggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	triggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	triggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	triggerBox->SetSimulatePhysics(false);
	triggerBox->SetGenerateOverlapEvents(true);
	triggerBox->SetNotifyRigidBodyCollision(false);
}

void AViewChangeActor::BeginPlay()
{
	Super::BeginPlay();
	
	triggerBox->OnComponentBeginOverlap.AddDynamic(this, &AViewChangeActor::OnOverlap);
}

void AViewChangeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AViewChangeActor::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	player = Cast<ATPSPlayer>(OtherActor);

	if (player != nullptr && cameraActor != nullptr)
	{
		// Áï½Ã ¹Ù²ñ
		//player->GetController<APlayerController>()->SetViewTarget(cameraActor);

		// ¼­¼­È÷ ¹Ù²ñ
		player->GetController<APlayerController>()->SetViewTargetWithBlend(cameraActor, 1.5f, EViewTargetBlendFunction::VTBlend_EaseIn, 0.2f);
	}
}

