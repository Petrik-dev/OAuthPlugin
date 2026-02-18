#include "BackendRequestTypes.h"

void FNewDeviceMetadata::ShowResult() const
{
	UE_LOG(LogTemp, Log, TEXT("NewDeviceMetadata: "));
	UE_LOG(LogTemp, Log, TEXT("DeviceGroupKey: %s"), *DeviceGroupKey);
	UE_LOG(LogTemp, Log, TEXT("DeviceKey: %s"), *DeviceKey);
}

void FCognitoAuthenticationResult::ShowResult() const
{
	UE_LOG(LogTemp, Log, TEXT("CognitoAuthenticationResult: "));
	UE_LOG(LogTemp, Log, TEXT("AccessToken: %s"), *AccessToken);
	UE_LOG(LogTemp, Log, TEXT("ExpiresIn: %d"), ExpiresIn);
	UE_LOG(LogTemp, Log, TEXT("IdToken: %s"), *IdToken);
	NewDeviceMetadata.ShowResult();
	UE_LOG(LogTemp, Log, TEXT("RefreshToken: %s"), *RefreshToken);
	UE_LOG(LogTemp, Log, TEXT("TokenType: %s"), *TokenType);
}
