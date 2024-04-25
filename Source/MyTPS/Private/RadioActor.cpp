// Fill out your copyright notice in the Description page of Project Settings.


#include "RadioActor.h"
#include "Components/AudioComponent.h"


ARadioActor::ARadioActor()
{
	PrimaryActorTick.bCanEverTick = true;

	rootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene"));
	SetRootComponent(rootComp);

	soundComp = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio Component"));
	soundComp->SetupAttachment(RootComponent);

}

void ARadioActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARadioActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

