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

	bool AutoSignIn();

	UFUNCTION(BlueprintCallable, Category = "OAuth")
	void SignInWithGoogle();
	UFUNCTION(BlueprintCallable, Category = "OAuth")
	void SignInWithApple();

	void SignOut();
	void ChangePlayerNickname(const FString& InNickname);
	void DeleteAccount();
	void RefreshToken(const FString& InRefreshToken);

	UOAuthLocalPlayerSubsystem* GetOAuthLocalPlayerSubsystem() const;

	FOnAPIRequestSucceeded OnSignInSucceeded;
	FOnAPIRequestSucceeded OnSignOutSucceeded;
	FOnAPIRequestSucceeded OnChangePlayerNicknameSucceeded;
	FOnAPIRequestSucceeded OnDeleteAccountSucceeded;

protected:

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UGatewayAPI> GatewayAPIDataAsset;
	
private:

	bool HasErrors(const TSharedPtr<FJsonObject>& JsonObject) const;
	FString SerializeJsonData(const TMap<FString, FString>& Params);

	void SignInWithGoogle_Internal(const FString& ServerClientId);
	void SignInWithApple_Internal();
	void SignOut_Internal();
	
	FString GetGoogleSignInJson_Internal();
	void TickGoogleSignInPolling();
	void SendGoogleSignInToBackend(const FString & GoogleResultJson);

	FString GetAppleSignInJson_Internal();
	void TickAppleSignInPolling();
	void SendAppleSignInToBackend(const FString & AppleResultJson);

	void Cognito_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessfull);
	void SignOut_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessfull);
	void ChangePlayerNickname_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessfull);
	void DeleteAccount_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessfull);
	void RefreshToken_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessfull);

	FTimerHandle GoogleSignInPollTimerHandle;
	FTimerHandle GoogleSignInTimeoutHandle;

	FTimerHandle AppleSignInPollTimerHandle;
	FTimerHandle AppleSignInTimeoutHandle;

	FString LastGoogleSignInResultJson;
	
};