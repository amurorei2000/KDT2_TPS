// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletFXActor.h"
#include "Particles/ParticleSystemComponent.h"

ABulletFXActor::ABulletFXActor()
{
	PrimaryActorTick.bCanEverTick = false;

	rootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	SetRootComponent(rootComp);

	particleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particle Component"));
	particleComp->SetupAttachment(RootComponent);
	particleComp->SetWorldScale3D(FVector(0.5f));
	particleComp->SetAutoActivate(false);
}

void ABulletFXActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABulletFXActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABulletFXActor::PlayFX()
{
	particleComp->Activate(true);
}

