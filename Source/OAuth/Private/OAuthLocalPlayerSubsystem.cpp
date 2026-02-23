// Fill out your copyright notice in the Description page of Project Settings.


#include "OAuthLocalPlayerSubsystem.h"
#include "Creds.h"
#include "Kismet/GameplayStatics.h"

void UOAuthLocalPlayerSubsystem::UpdateTokens(const FCognitoAuthenticationResult& InAuthenticationResult)
{
	AuthenticationResult = InAuthenticationResult;
	SaveTokenToSaveGame();
}

void UOAuthLocalPlayerSubsystem::ClearTokens()
{
	Nickname = FString();
	AuthenticationResult = FCognitoAuthenticationResult();

	if (UGameplayStatics::DoesSaveGameExist(TEXT("Creds"), 0))
	{
		UGameplayStatics::DeleteGameInSlot(TEXT("Creds"), 0);
	}
}

void UOAuthLocalPlayerSubsystem::SaveNickname(const FString& InNickname)
{
	Nickname = InNickname;
	UCreds* Creds = nullptr;
	if (UGameplayStatics::DoesSaveGameExist(TEXT("Creds"), 0))
	{
		if (USaveGame* SaveGame = UGameplayStatics::LoadGameFromSlot(TEXT("Creds"), 0))
		{
			Creds = Cast<UCreds>(SaveGame);
		}
	}

	if (!Creds)
	{
		Creds = Cast<UCreds>(UGameplayStatics::CreateSaveGameObject(UCreds::StaticClass()));
	}

	if (Creds)
	{
		Creds->Nickname = InNickname;
		UGameplayStatics::SaveGameToSlot(Creds, TEXT("Creds"), 0);
	}
	
}

void UOAuthLocalPlayerSubsystem::SaveTokenToSaveGame()
{
	UCreds* Creds = nullptr;
	if (UGameplayStatics::DoesSaveGameExist(TEXT("Creds"), 0))
	{
		if (USaveGame* SavedGame = UGameplayStatics::LoadGameFromSlot(TEXT("Creds"), 0))
		{
			Creds = Cast<UCreds>(SavedGame);
		}
	}

	if (!Creds)
	{
		Creds = Cast<UCreds>(UGameplayStatics::CreateSaveGameObject(UCreds::StaticClass()));
	}

	if (Creds)
	{
		Creds->Nickname = Nickname;
		Creds->AuthenticationResult = AuthenticationResult;

		UGameplayStatics::SaveGameToSlot(Creds, TEXT("Creds"), 0);
	}
	
}
