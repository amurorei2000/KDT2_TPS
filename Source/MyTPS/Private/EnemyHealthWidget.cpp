// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHealthWidget.h"
#include "Components/ProgressBar.h"


void UEnemyHealthWidget::SetHealthBar(float value, FLinearColor lowHealthColor)
{
	pb_healthBar->SetPercent(value);

	// 만일, 플레이어의 남은 체력이 20% 미만일 경우 체력바의 fill 색상을 주황색으로 표시한다.
	if (value < 0.2f)
	{
		pb_healthBar->SetFillColorAndOpacity(lowHealthColor);
	}
}
