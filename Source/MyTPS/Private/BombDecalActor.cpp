// Fill out your copyright notice in the Description page of Project Settings.


#include "BombDecalActor.h"
#include "Components/DecalComponent.h"



void ABombDecalActor::BeginPlay()
{
	Super::BeginPlay();
	
	GetDecal()->DecalSize = FVector(decalRadius);
}

void ABombDecalActor::SetShowDecal(bool visible)
{
	GetDecal()->SetVisibility(visible);
}
