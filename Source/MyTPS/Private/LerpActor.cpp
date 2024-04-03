// Fill out your copyright notice in the Description page of Project Settings.


#include "LerpActor.h"

ALerpActor::ALerpActor()
{
	PrimaryActorTick.bCanEverTick = true;

	rootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(rootComp);

	mesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh 1"));
	mesh1->SetupAttachment(RootComponent);

	mesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh 2"));
	mesh2->SetupAttachment(RootComponent);
}

void ALerpActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALerpActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (positions.Num() > 1)
	{
		// 선형 보간
		FVector resultVector = FMath::Lerp(positions[0], positions[1], alpha);
		alpha += DeltaTime * 0.5f;
		alpha = FMath::Min(alpha, 1.0f);

		// 선 그리기(디버깅용)
		DrawDebugLine(GetWorld(), positions[0], resultVector, FColor::Red, false, 0, 0, 2);
		
		// 메시 위치 설정하기(World 좌표)
		mesh1->SetWorldLocation(positions[0]);
		mesh2->SetWorldLocation(positions[1]);
	}
}

