// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "BackendRequestTypes.h"
#include "OAuthLocalPlayerSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class OAUTH_API UOAuthLocalPlayerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	void UpdateTokens(const FCognitoAuthenticationResult& InAuthenticationResult);
	void ClearTokens();
	void SaveNickname(const FString& InNickname);

	FString Nickname;
	FCognitoAuthenticationResult AuthenticationResult;

private:

	void SaveTokenToSaveGame();
	
};
