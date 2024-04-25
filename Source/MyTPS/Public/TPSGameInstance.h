// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "TPSGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class MYTPS_API UTPSGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category="MySettings")
	UDataTable* dt_playerStatus;


	UFUNCTION(BlueprintCallable)
	FPlayerStatus GetPlayerTableData(const FString& rowName);
};
