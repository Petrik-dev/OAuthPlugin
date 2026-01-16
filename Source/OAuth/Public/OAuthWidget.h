// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OAuthWidget.generated.h"

class UOABackendManager;
class UButton;
/**
 * 
 */
UCLASS()
class OAUTH_API UOAuthWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeConstruct() override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UOABackendManager> OAuthBackendManagerClass;

	UPROPERTY(meta = (BindWidget))
	UButton* SignInWithGoogle_Button;

private:

	UFUNCTION()
	void SignInWithGoogle();

	UPROPERTY()
	UOABackendManager* OAuthBackendManager;
	
};
