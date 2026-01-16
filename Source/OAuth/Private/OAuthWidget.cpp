// Fill out your copyright notice in the Description page of Project Settings.


#include "OAuthWidget.h"

#include "OABackendManager.h"
#include "Components/Button.h"

void UOAuthWidget::NativeConstruct()
{
	Super::NativeConstruct();

	check(OAuthBackendManagerClass);

	OAuthBackendManager = NewObject<UOABackendManager>(this, OAuthBackendManagerClass);

	SignInWithGoogle_Button->OnClicked.AddDynamic(this, &UOAuthWidget::SignInWithGoogle);
}

void UOAuthWidget::SignInWithGoogle()
{
	OAuthBackendManager->SignInWithGoogle();
}
