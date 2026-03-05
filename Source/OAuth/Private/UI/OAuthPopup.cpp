// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OAuthPopup.h"

#include "Components/EditableText.h"
#include "Components/TextBlock.h"


void UOAuthPopup::NativeConstruct()
{
	Super::NativeConstruct();

	Nickname_EditableText->OnTextChanged.AddDynamic(this, &UOAuthPopup::ChangeText);
}

void UOAuthPopup::UpdateStateOfPopup(EBackendRequestResources TypeOfPopup, const FString& InNickname)
{
	TypeOfPopup == EBackendRequestResources::ChangePlayerNickname
		? Nickname_EditableText->SetVisibility(ESlateVisibility::Visible)
		: Nickname_EditableText->SetVisibility(ESlateVisibility::Collapsed);

	switch (TypeOfPopup)
	{
	case EBackendRequestResources::ChangePlayerNickname:
		{
			NewNickname = InNickname;
			FString InfoText = FString::Printf(TEXT("Right now your nickname is: %s%s"), LINE_TERMINATOR, *InNickname);
			PopupMessage_TextBlock->SetText(FText::FromString(InfoText));
			Nickname_EditableText->SetText(FText::FromString(NewNickname));
		}
		break;

	case EBackendRequestResources::SignOut:
		{
			PopupMessage_TextBlock->SetText(FText::FromString(TEXT("Are you sure you want to sign out?")));	
		}
		break;

	case EBackendRequestResources::DeleteAccount:
		{
			PopupMessage_TextBlock->SetText(FText::FromString(TEXT("Are you sure you want to delete the account?")));
		}

	default:
		break;
	}
}

void UOAuthPopup::ChangeText(const FText& Text)
{
	if (Text.ToString().Len() > 10)
	{
		Nickname_EditableText->SetText(FText::FromString(NewNickname));
	}
	NewNickname = Text.ToString();
}
