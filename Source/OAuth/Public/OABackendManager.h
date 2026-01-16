// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "OABackendManager.generated.h"

/**
 * 
 */
UCLASS()
class OAUTH_API UOABackendManager : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "OAuth")
	void SignInWithGoogle();

private:

	void SignInWithGoogle_Internal();
	
};
