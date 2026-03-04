// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BackendRequestTypes.h"
#include "OAuthPopup.generated.h"

/**
 * 
 */

class UTextBlock;
class UButton;
class UEditableText;

UCLASS()
class OAUTH_API UOAuthPopup : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PopupMessage_TextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> Nickname_EditableText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Ok_Button;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Cancel_Button;

	FString NewNickname {};

	void UpdateStateOfPopup(EBackendRequestResources TypeOfPopup, const FString& InNickname = "");

protected:

	virtual void NativeConstruct() override;

private:

	UFUNCTION()
	void ChangeText(const FText& Text);
	
};
