// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TimerManager.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"
#include "OABackendManager.generated.h"

class UOAuthLocalPlayerSubsystem;
class UGatewayAPI;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAPIRequestSucceeded, bool, Succeeded, const FString&, Log);

/**
 * 
 */
UCLASS(Blueprintable)
class OAUTH_API UOABackendManager : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "OAuth")
	void SignInWithGoogle();

	UOAuthLocalPlayerSubsystem* GetOAuthLocalPlayerSubsystem() const;

	FOnAPIRequestSucceeded OnSignInSucceeded;

protected:

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UGatewayAPI> GatewayAPIDataAsset;
	
private:

	bool HasErrors(const TSharedPtr<FJsonObject>& JsonObject) const;

	void SignInWithGoogle_Internal(const FString& ServerClientId);

	FString GetGoogleSignInJson_Internal();
	void TickGoogleSignInPolling();
	void SendGoogleSignInToBackend(const FString & GoogleResultJson);

	void Cognito_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessfull);

	FTimerHandle GoogleSignInPollTimerHandle;
	FTimerHandle GoogleSignInTimeotHandle;

	FString LastGoogleSignInResultJson;
	
};