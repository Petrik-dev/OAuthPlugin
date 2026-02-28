#pragma once

#include "CoreMinimal.h"
#include "BackendRequestTypes.generated.h"

USTRUCT()
struct FNewDeviceMetadata
{
	GENERATED_BODY()

	UPROPERTY()
	FString DeviceGroupKey;

	UPROPERTY()
	FString DeviceKey;

	void ShowResult() const;
};

USTRUCT()
struct FCognitoAuthenticationResult
{
	GENERATED_BODY()

	UPROPERTY()
	FString AccessToken;

	UPROPERTY()
	int32 ExpiresIn = 0;

	UPROPERTY()
	FString IdToken;

	UPROPERTY()
	FNewDeviceMetadata NewDeviceMetadata;

	UPROPERTY()
	FString RefreshToken;

	UPROPERTY()
	FString TokenType;

	void ShowResult() const;
};

UENUM(BlueprintType)
enum class EBackendRequestResources : uint8
{
	None,
	GoogleSignIn,
	SignOut
};