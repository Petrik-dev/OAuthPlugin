// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OAuthWidget.h"

#include "OABackendManager.h"
#include "OAuthLocalPlayerSubsystem.h"
#include "Components/Button.h"
#include "Components/CircularThrobber.h"
#include "Components/SizeBox.h"
#include "Components/WidgetSwitcher.h"
#include "UI/PlayerInfoWidget.h"

void UOAuthWidget::NativeConstruct()
{
	Super::NativeConstruct();

	check(OAuthBackendManagerClass);

	OAuthBackendManager = NewObject<UOABackendManager>(this, OAuthBackendManagerClass);

	SignInWithGoogle_Button->OnClicked.AddDynamic(this, &UOAuthWidget::SignInWithGoogle);

	OAuthBackendManager->OnSignInSucceeded.AddDynamic(this, &UOAuthWidget::SignInSucceeded);
}

void UOAuthWidget::SignInWithGoogle()
{
	OAuthBackendManager->SignInWithGoogle();
	SignIn_WidgetSwitcher->SetActiveWidget(LoadingScreen_CircularThrobber);
}

void UOAuthWidget::SignInSucceeded(bool IsSucceeded, const FString& LogMessage)
{
	const FString Msg = FString::Printf(TEXT("SignInSucceeded called. ISucceeded=%s, Message=%s"), IsSucceeded ? TEXT("true") : TEXT("false"), *LogMessage);
	UE_LOG(LogTemp, Log, TEXT("%s"), *Msg);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, IsSucceeded ? FColor::Green : FColor::Red, *Msg);
	}

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
