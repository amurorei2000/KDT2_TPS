// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSGameInstance.h"
#include "TPSPlayer.h"


FPlayerStatus UTPSGameInstance::GetPlayerTableData(const FString& rowName)
{
	// 데이터 Parsing(분석) 하기
	FString errorText;
	FPlayerStatus* playerStatusParse = dt_playerStatus->FindRow<FPlayerStatus>(FName(rowName), errorText);

	return *playerStatusParse;
}
