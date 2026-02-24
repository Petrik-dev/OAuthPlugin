// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerInfoWidget.h"

#include "Components/TextBlock.h"

void UPlayerInfoWidget::UpdatePlayerNickname(const FString& InPlayerNickname)
{
	Nickname_TextBlock->SetText(FText::FromString(InPlayerNickname));
}
