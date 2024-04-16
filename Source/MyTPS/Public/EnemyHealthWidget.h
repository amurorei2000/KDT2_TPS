// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHealthWidget.generated.h"

/**
 * 
 */
UCLASS()
class MYTPS_API UEnemyHealthWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, Category="MySettings", meta=(BindWidget))
	class UProgressBar* pb_healthBar;

	void SetHealthBar(float value, FLinearColor lowHealthColor);
};
