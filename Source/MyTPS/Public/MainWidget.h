// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainWidget.generated.h"

/**
 * 
 */
UCLASS()
class MYTPS_API UMainWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, Category="MySettings", meta=(BindWidget))
	class UImage* img_crosshair;

	UPROPERTY(VisibleAnywhere, Category = "MySettings", meta = (BindWidget))
	class UImage* img_weapon;
	
	UPROPERTY(VisibleAnywhere, Category = "MySettings", meta = (BindWidget))
	class UImage* img_zoom;

	UPROPERTY(EditAnywhere, Category = "MySettings")
	TArray<class UTexture2D*> weaponTextures;

	void SetWeaponTexture(int32 index);
	void SetSniperMode(bool modeOff);
};
