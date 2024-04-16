// Fill out your copyright notice in the Description page of Project Settings.


#include "MainWidget.h"
#include "Components/Image.h"


void UMainWidget::SetWeaponTexture(int32 index)
{
	// 만일, 인덱스가 weaponTextures 배열의 인덱스 범위 이내라면...
	if (index >= 0 && index < weaponTextures.Num())
	{
		// UImage 컴포넌트의 Brush 속성에 지정된 텍스쳐를 할당한다.
		img_weapon->SetBrushFromTexture(weaponTextures[index]);
	}
}

void UMainWidget::SetSniperMode(bool modeOff)
{
	// 스나이퍼 모드가 off라면...
	if (modeOff)
	{
		img_crosshair->SetVisibility(ESlateVisibility::Visible);
		img_zoom->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		img_crosshair->SetVisibility(ESlateVisibility::Hidden);
		img_zoom->SetVisibility(ESlateVisibility::Visible);
	}
}

void UMainWidget::ShowHitBorder(bool bShow)
{
	if (bShow)
	{
		img_hitBorder->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		img_hitBorder->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMainWidget::PlayHitAnimation()
{
	StopAnimation(HitAnimation);
	PlayAnimationForward(HitAnimation);
}
