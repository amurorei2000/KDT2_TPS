// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSMainGameModeBase.h"
#include "MainWidget.h"


void ATPSMainGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// 메인 UI를 화면에 출력한다.
	if (mainWidget_bp != nullptr)
	{
		UMainWidget* mainWidget_inst = CreateWidget<UMainWidget>(GetWorld(), mainWidget_bp);
		if (mainWidget_inst)
		{
			mainWidget_inst->AddToViewport(0);
		}
	}
}
