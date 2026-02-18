// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BackendRequestTypes.h"
#include "GatewayAPI.generated.h"

/**
 * 
 */
UCLASS()
class OAUTH_API UGatewayAPI : public UDataAsset
{
	GENERATED_BODY()

public:

	FString GetInvokeURL(EBackendRequestResources RequestResources) const;
	
protected:

	UPROPERTY(EditDefaultsOnly)
	FString InvokeURL;

	UPROPERTY(EditDefaultsOnly)
	TMap<EBackendRequestResources, FString> Resources;
	
};
