// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OAuthWidget.generated.h"

class UPlayerInfoWidget;
class UOABackendManager;
class UButton;
class UCircularThrobber;
class UWidgetSwitcher;
class USizeBox;
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
	TObjectPtr<UButton> SignInWithGoogle_Button;

private:

	UFUNCTION()
	void SignInWithGoogle();

	UFUNCTION()
	void SignInSucceeded(bool IsSucceeded, const FString& LogMessage);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCircularThrobber> LoadingScreen_CircularThrobber;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> SignIn_WidgetSwitcher;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPlayerInfoWidget> PlayerInfoWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> ButtonsWrapper_SizeBox;

	UPROPERTY()
	UOABackendManager* OAuthBackendManager;
	
};
