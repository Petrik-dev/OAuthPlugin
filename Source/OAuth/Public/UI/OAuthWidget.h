// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BackendRequestTypes.h"
#include "OAuthWidget.generated.h"

class UOAuthPopup;
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
	void SignOut();
	UFUNCTION()
	void ChangePlayerNickname();
	UFUNCTION()
	void DeleteAccount();

	UFUNCTION()
	void SignInSucceeded(bool IsSucceeded, const FString& LogMessage);
	UFUNCTION()
	void SignOutSucceeded(bool IsSucceded, const FString& LogMessage);
	UFUNCTION()
	void ChangePlayerNicknameSucceeded(bool IsSucceded, const FString& LogMessage);
	UFUNCTION()
	void DeleteAccountSucceeded(bool IsSucceded, const FString& LogMessage);

	UFUNCTION()
	void PopupOkClicked();
	UFUNCTION()
	void PopupCancelClicked();


	void ShowLogMessage(bool IsSucceeded, const FString& LogMessage);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCircularThrobber> LoadingScreen_CircularThrobber;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> SignIn_WidgetSwitcher;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPlayerInfoWidget> PlayerInfoWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> ButtonsWrapper_SizeBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOAuthPopup> PopupWidget;
	
	EBackendRequestResources CurrentPopupType = EBackendRequestResources::None;

	UPROPERTY()
	UOABackendManager* OAuthBackendManager;
	
};
