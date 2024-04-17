// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TPSMainGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class MYTPS_API ATPSMainGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category="MySettings")
	TSubclassOf<class UMainWidget> mainWidget_bp;

	UPROPERTY()
	class UMainWidget* mainWidget_inst;

	void RespawnPlayer(AController* NewPlayer, APawn* previousPawn);

protected:
	virtual void BeginPlay() override;

};
