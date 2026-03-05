// Fill out your copyright notice in the Description page of Project Settings.
#include "UI/OAuthWidget.h"

#include "OABackendManager.h"
#include "OAuthLocalPlayerSubsystem.h"
#include "Components/Button.h"
#include "Components/CircularThrobber.h"
#include "Components/SizeBox.h"
#include "Components/WidgetSwitcher.h"
#include "UI/OAuthPopup.h"
#include "UI/PlayerInfoWidget.h"

void UOAuthWidget::NativeConstruct()
{
	Super::NativeConstruct();

	check(OAuthBackendManagerClass);

	OAuthBackendManager = NewObject<UOABackendManager>(this, OAuthBackendManagerClass);

	SignInWithGoogle_Button->OnClicked.AddDynamic(this, &UOAuthWidget::SignInWithGoogle);

	PlayerInfoWidget->SignOut_Button->OnClicked.AddDynamic(this, &UOAuthWidget::SignOut);
	PlayerInfoWidget->ChangePlayerNickname_Button->OnClicked.AddDynamic(this, &UOAuthWidget::ChangePlayerNickname);
	PlayerInfoWidget->DeleteAccount_Button->OnClicked.AddDynamic(this, &UOAuthWidget::DeleteAccount);

	PopupWidget->Ok_Button->OnClicked.AddDynamic(this, &UOAuthWidget::PopupOkClicked);
	PopupWidget->Cancel_Button->OnClicked.AddDynamic(this, &UOAuthWidget::PopupCancelClicked);

	OAuthBackendManager->OnSignInSucceeded.AddDynamic(this, &UOAuthWidget::SignInSucceeded);
	OAuthBackendManager->OnSignOutSucceeded.AddDynamic(this, &UOAuthWidget::SignOutSucceeded);
	OAuthBackendManager->OnChangePlayerNicknameSucceeded.AddDynamic(this, &UOAuthWidget::ChangePlayerNicknameSucceeded);
	OAuthBackendManager->OnDeleteAccountSucceeded.AddDynamic(this, &UOAuthWidget::DeleteAccountSucceeded);
}

void UOAuthWidget::SignInWithGoogle()
{
	OAuthBackendManager->SignInWithGoogle();
	SignIn_WidgetSwitcher->SetActiveWidget(LoadingScreen_CircularThrobber);
}

void UOAuthWidget::SignInSucceeded(bool IsSucceeded, const FString& LogMessage)
{
	ShowLogMessage(IsSucceeded, LogMessage);

	if (IsSucceeded)
	{
		UOAuthLocalPlayerSubsystem* LocalPlayerSubsystem = OAuthBackendManager->GetOAuthLocalPlayerSubsystem();
		PlayerInfoWidget->UpdatePlayerNickname(LocalPlayerSubsystem->Nickname);
		SignIn_WidgetSwitcher->SetActiveWidget(PlayerInfoWidget);
	}
	else
	{
		SignIn_WidgetSwitcher->SetActiveWidget(ButtonsWrapper_SizeBox);
	}
}

void UOAuthWidget::SignOut()
{
	CurrentPopupType = EBackendRequestResources::SignOut;
	SignIn_WidgetSwitcher->SetActiveWidget(PopupWidget);
	PopupWidget->UpdateStateOfPopup(CurrentPopupType);
}

void UOAuthWidget::SignOutSucceeded(bool IsSucceded, const FString& LogMessage)
{
	ShowLogMessage(IsSucceded, LogMessage);
	SignIn_WidgetSwitcher->SetActiveWidget(ButtonsWrapper_SizeBox);
}

void UOAuthWidget::ShowLogMessage(bool IsSucceeded, const FString& LogMessage)
{
	const FString Msg = FString::Printf(TEXT("SignInSucceeded called. ISucceeded=%s, Message=%s"), IsSucceeded ? TEXT("true") : TEXT("false"), *LogMessage);
	UE_LOG(LogTemp, Log, TEXT("%s"), *Msg);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, IsSucceeded ? FColor::Green : FColor::Red, *Msg);
	}
}

void UOAuthWidget::ChangePlayerNickname()
{
	CurrentPopupType = EBackendRequestResources::ChangePlayerNickname;
	SignIn_WidgetSwitcher->SetActiveWidget(PopupWidget);
	UOAuthLocalPlayerSubsystem* LocalPlayerSubsystem = OAuthBackendManager->GetOAuthLocalPlayerSubsystem();
	if (!IsValid(LocalPlayerSubsystem)) return;
		
	PopupWidget->UpdateStateOfPopup(CurrentPopupType, LocalPlayerSubsystem->Nickname);
}

void UOAuthWidget::ChangePlayerNicknameSucceeded(bool IsSucceded, const FString& LogMessage)
{
	ShowLogMessage(IsSucceded, LogMessage);

	PlayerInfoWidget->UpdatePlayerNickname(PopupWidget->NewNickname);
	SignIn_WidgetSwitcher->SetActiveWidget(PlayerInfoWidget);
	UOAuthLocalPlayerSubsystem* LocalPlayerSubsystem = OAuthBackendManager->GetOAuthLocalPlayerSubsystem();
	if (!IsValid(LocalPlayerSubsystem)) return;

	LocalPlayerSubsystem->Nickname = PopupWidget->NewNickname;
}

void UOAuthWidget::PopupOkClicked()
{
	switch (CurrentPopupType)
	{
		case EBackendRequestResources::ChangePlayerNickname:
			{
				UOAuthLocalPlayerSubsystem* LocalPlayerSubsystem = OAuthBackendManager->GetOAuthLocalPlayerSubsystem();
				if (!IsValid(LocalPlayerSubsystem)) return;

				const FString CurrentNickname = LocalPlayerSubsystem->Nickname;
				const FString NewNickname = PopupWidget->NewNickname;

				if (NewNickname.IsEmpty() || NewNickname.Len() > 10) return;
				if (NewNickname == CurrentNickname)
				{
					PopupCancelClicked();
					return;
				}
				SignIn_WidgetSwitcher->SetActiveWidget(LoadingScreen_CircularThrobber);
				OAuthBackendManager->ChangePlayerNickname(NewNickname);
			}
			break;

	case EBackendRequestResources::SignOut:
		{
			SignIn_WidgetSwitcher->SetActiveWidget(LoadingScreen_CircularThrobber);
			OAuthBackendManager->SignOut();
		}
		break;

	case EBackendRequestResources::DeleteAccount:
		{
			SignIn_WidgetSwitcher->SetActiveWidget(LoadingScreen_CircularThrobber);
			OAuthBackendManager->DeleteAccount();
		}
		break;
		
	default:
		break;
	}
	
}

void UOAuthWidget::PopupCancelClicked()
{
	SignIn_WidgetSwitcher->SetActiveWidget(PlayerInfoWidget);
}

void UOAuthWidget::DeleteAccount()
{
	CurrentPopupType = EBackendRequestResources::DeleteAccount;
	SignIn_WidgetSwitcher->SetActiveWidget(PopupWidget);

	PopupWidget->UpdateStateOfPopup(CurrentPopupType);
}

void UOAuthWidget::DeleteAccountSucceeded(bool IsSucceded, const FString& LogMessage)
{
	ShowLogMessage(IsSucceded, LogMessage);

	if (IsSucceded)
	{
		SignIn_WidgetSwitcher->SetActiveWidget(ButtonsWrapper_SizeBox);
	}
	else
	{
		SignIn_WidgetSwitcher->SetActiveWidget(PlayerInfoWidget);
	}
}