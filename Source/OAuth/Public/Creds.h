// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "BackendRequestTypes.h"
#include "Creds.generated.h"

/**
 * 
 */
UCLASS()
class OAUTH_API UCreds : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY(SaveGame)
	FString Nickname;
	UPROPERTY(SaveGame)
	FCognitoAuthenticationResult AuthenticationResult{};

};
