// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSMainGameModeBase.h"
#include "MainWidget.h"


void ATPSMainGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// 메인 UI를 화면에 출력한다.
	if (mainWidget_bp != nullptr)
	{
		mainWidget_inst = CreateWidget<UMainWidget>(GetWorld(), mainWidget_bp);

		if (mainWidget_inst)
		{
			mainWidget_inst->AddToViewport(0);
		}
	}
}

void ATPSMainGameModeBase::RespawnPlayer(AController* NewPlayer, APawn* previousPawn)
{
	// 기존 폰에서 플레이어 컨트롤러를 떼어낸다.
	NewPlayer->UnPossess();

	// 게임 모드 베이스에 있는 재생성 함수를 실행한다.
	RestartPlayer(NewPlayer);

	// 기존의 폰을 제거한다.
	previousPawn->Destroy();
}



