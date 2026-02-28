// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerInfoWidget.generated.h"

/**
 * 
 */
class UTextBlock;
class UButton;
UCLASS()
class OAUTH_API UPlayerInfoWidget : public UUserWidget
{
	GENERATED_BODY()


public:

	void UpdatePlayerNickname(const FString& InPlayerNickname);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SignOut_Button;
	
private:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Nickname_TextBlock;
};
