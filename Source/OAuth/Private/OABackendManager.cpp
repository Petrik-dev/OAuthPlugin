// Fill out your copyright notice in the Description page of Project Settings.

#include "OABackendManager.h"
#include "GatewayAPI.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "OAuthLocalPlayerSubsystem.h"
#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#endif


#if PLATFORM_ANDROID
static constexpr const char* JAVA_HELPER_CLASS = "com/Plugins/SignInAndroidHelper/SignInAndroidHelper";
#endif


void UOABackendManager::SignInWithGoogle()
{
	SignInWithGoogle_Internal("922532317105-7aefn8h531ti512n4n35pce4nsfiu7l6.apps.googleusercontent.com");

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GoogleSignInPollTimerHandle);
		World->GetTimerManager().ClearTimer(GoogleSignInTimeotHandle);

		FTimerDelegate PollDelegate;
		PollDelegate.BindLambda([this]()
		{
			if (!IsValid(this)) return;
			TickGoogleSignInPolling();
		});

		World->GetTimerManager().SetTimer(GoogleSignInPollTimerHandle, PollDelegate, 0.3f, true);

		FTimerDelegate TimeoutDelegate;
		TimeoutDelegate.BindLambda([this]()
		{
			if (UWorld* W = GetWorld())
			{
				W->GetTimerManager().ClearTimer(GoogleSignInPollTimerHandle);
			}
			UE_LOG(LogTemp, Error, TEXT("Google Sign-in: Timer timed out"));
		});

		World->GetTimerManager().SetTimer(GoogleSignInTimeotHandle, TimeoutDelegate, 30.0f, false);
	}
}

UOAuthLocalPlayerSubsystem* UOABackendManager::GetOAuthLocalPlayerSubsystem() const
{
	if (const APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			return LP->GetSubsystem<UOAuthLocalPlayerSubsystem>();
		}
	}
	return nullptr;
}

bool UOABackendManager::HasErrors(const TSharedPtr<FJsonObject>& JsonObject) const
{
	if (!JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HasErrors: Invalid Json Object"));
		return true;
	}

	auto GetStringOr = [&](const TCHAR* Field, const TCHAR* DefaultValue) -> FString
	{
		FString Out;
		return JsonObject->TryGetStringField(Field, Out) ? Out : FString(DefaultValue);
	};

	const bool bHasErrorType = JsonObject->HasTypedField<EJson::String>(TEXT("errorType"));
	const bool bHasErrorMessage = JsonObject->HasTypedField<EJson::String>(TEXT("errorMessage"));
	const bool bHasFault = JsonObject->HasField(TEXT("$fault"));

	if (bHasErrorType || bHasErrorMessage)
	{
		const FString ErrorType = GetStringOr(TEXT("errorType"), TEXT("unknown error"));
		const FString ErrorMessage = GetStringOr(TEXT("errorMessage"), TEXT("unknown error"));

		UE_LOG(LogTemp, Error, TEXT("Backend error: type='%s', message='%s',"), *ErrorType, *ErrorMessage);
		return true;
	}

	if (bHasFault)
	{
		const FString ErrorName = GetStringOr(TEXT("name"), TEXT("unknown error"));
		const FString FaultType = GetStringOr(TEXT("$fault"), TEXT("fault"));

		UE_LOG(LogTemp, Error, TEXT("Backend fault: fault='%s', name='%s'"), *FaultType, *ErrorName);
		return true;
	}
	return false;
}

void UOABackendManager::SignInWithGoogle_Internal(const FString& ServerClientId)
{
#if PLATFORM_ANDROID

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (!Env)
	{
		UE_LOG(LogTemp, Error, TEXT("SignInWithGoogle_Internal: Env is NULL"));
		return;
	}
	
	jclass HelperClass = FAndroidApplication::FindJavaClass(JAVA_HELPER_CLASS);
	if (!HelperClass)
	{
		UE_LOG(LogTemp, Error, TEXT("SignInWithGoogle_Internal: HelperClass is NULL"));
		return;
	}

	jmethodID startSignInMethod = FJavaWrapper::FindStaticMethod(
		Env,
		HelperClass,
		"startSignIn",
		"(Landroid/app/Activity;Ljava/lang/String;)V",
		false);

	if (!startSignInMethod)
	{
		UE_LOG(LogTemp, Error, TEXT("SignInWithGoogle_Internal: startSignInMethod is NULL"));
		Env->DeleteLocalRef(HelperClass);
		return;
	}


	jobject Activity = FAndroidApplication::GetGameActivityThis();
	if (!Activity)
	{
		UE_LOG(LogTemp, Error, TEXT("SignInWithGoogle_Internal: Activity is NULL"));
		Env->DeleteLocalRef(HelperClass);
		return;
	}

	auto ClientIdUTF8 = StringCast<ANSICHAR>(*ServerClientId);
	jstring JClientId = Env->NewStringUTF(ClientIdUTF8.Get());

	Env->CallStaticVoidMethod(HelperClass, startSignInMethod, Activity, JClientId);

	Env->DeleteLocalRef(JClientId);
	Env->DeleteLocalRef(HelperClass);
#else
	UE_LOG(LogTemp, Warning, TEXT("Android-only code, nothing to do on this platform"));
	
	#endif
}

FString UOABackendManager::GetGoogleSignInJson_Internal()
{
#if PLATFORM_ANDROID
	FString Result;

	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jclass HelperClass = FAndroidApplication::FindJavaClass(JAVA_HELPER_CLASS);
		if (!HelperClass)
		{
			UE_LOG(LogTemp, Error, TEXT("GetGoogleSignInJson_Internal: HelperClass is NULL"));
			return Result;
		}
		jmethodID consumeLastResultJsonMethod = Env->GetStaticMethodID(HelperClass, "consumeLastResultJson", "()Ljava/lang/String;");
		if (!consumeLastResultJsonMethod)
		{
			UE_LOG(LogTemp, Error, TEXT("GetGoogleSignInJson_Internal: consumeLastResultJsonMethod not found"));
			Env->DeleteLocalRef(HelperClass);
			return Result;
		}

		jstring JResult = (jstring)Env->CallStaticObjectMethod(HelperClass, consumeLastResultJsonMethod);
		if (JResult)
		{
			const char* Utf8 = Env->GetStringUTFChars(JResult, nullptr);
			Result = UTF8_TO_TCHAR(Utf8);
			Env->ReleaseStringUTFChars(JResult, Utf8);
			Env->DeleteLocalRef(JResult);
		}
		Env->DeleteLocalRef(HelperClass);
	}
	return Result;
#endif
	return FString();
}

void UOABackendManager::TickGoogleSignInPolling()
{
	const FString ResultJson = GetGoogleSignInJson_Internal();
	if (ResultJson.IsEmpty()) return;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GoogleSignInPollTimerHandle);
		World->GetTimerManager().ClearTimer(GoogleSignInTimeotHandle);
	}

	LastGoogleSignInResultJson = ResultJson;

	TSharedPtr<FJsonObject> GoogleJson;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResultJson);
	if (!FJsonSerializer::Deserialize(Reader, GoogleJson) || !GoogleJson.IsValid())
	{
		OnSignInSucceeded.Broadcast(false, TEXT("Parse json failed"));
		return;
	}

	bool bSuccess = false;
	if (!GoogleJson->TryGetBoolField(TEXT("success"), bSuccess))
	{
		UE_LOG(LogTemp, Warning, TEXT("GoogleSignIn ResultJson: %s"), *ResultJson);
		OnSignInSucceeded.Broadcast(false, TEXT("Json doesnt have success field"));
		return;
	}

	if (!bSuccess)
	{
		FString Error;
		GoogleJson->TryGetStringField(TEXT("error"), Error);
		OnSignInSucceeded.Broadcast(false, FString::Printf(TEXT("GoogleSignIn failed: %s"), *Error));
		return;
	}

	FString IdToken;
	if (!GoogleJson->TryGetStringField(TEXT("idToken"), IdToken))
	{
		OnSignInSucceeded.Broadcast(false, TEXT("Json doesnt have idtoken field"));
		return;
	}

	TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
	Payload->SetStringField(TEXT("idToken"), IdToken);

	FString PayloadString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadString);
	FJsonSerializer::Serialize(Payload.ToSharedRef(), Writer);

	SendGoogleSignInToBackend(PayloadString);
}

void UOABackendManager::SendGoogleSignInToBackend(const FString& GoogleResultJson)
{
	check(GatewayAPIDataAsset);
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UOABackendManager::Cognito_Response);

	const FString APIUrl = GatewayAPIDataAsset->GetInvokeURL(EBackendRequestResources::GoogleSignIn);
	Request->SetURL(APIUrl);
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");

	Request->SetContentAsString(GoogleResultJson);
	Request->ProcessRequest();
}

void UOABackendManager::Cognito_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessfull)
{
	if (!bWasSuccessfull || !Request.IsValid())
	{
		OnSignInSucceeded.Broadcast(false, TEXT("Cognito_Response failed"));
		return;
	}

	const FString ResponseString = Response->GetContentAsString();
	UE_LOG(LogTemp, Log, TEXT("Cognito_Response: %s"), *ResponseString);

	if (ResponseString.Contains(TEXT("Missing Authentication Token")) || ResponseString.Contains(TEXT("\"message\"")))
	{
		OnSignInSucceeded.Broadcast(false, TEXT("API Gateway error (route/method/stage)"));
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(ResponseString);
	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
	{
		OnSignInSucceeded.Broadcast(false, TEXT("Cognito response wrong json"));
		return;
	}
	if (HasErrors(JsonObject))
	{
		OnSignInSucceeded.Broadcast(false, TEXT("Cognito response HasErrors"));
		return;
	}

	FCognitoAuthenticationResult AuthResult;
	FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &AuthResult);
	AuthResult.ShowResult();

	FString Nickname;
	if (JsonObject -> HasField(TEXT("nickname")))
	{
		Nickname = JsonObject->GetStringField(TEXT("nickname"));
	}

	UOAuthLocalPlayerSubsystem* LocalPlayerSubsystem = GetOAuthLocalPlayerSubsystem();
	if (IsValid(LocalPlayerSubsystem))
	{
		if (!Nickname.IsEmpty())
		{
			LocalPlayerSubsystem->Nickname = Nickname;
		}
		LocalPlayerSubsystem->UpdateTokens(AuthResult);
	}

	OnSignInSucceeded.Broadcast(true, TEXT("Cognito authentication successful"));

}

FString UOABackendManager::SerializeJsonData(const TMap<FString, FString>& Params)
{
	TSharedPtr<FJsonObject> JsonObjectData = MakeShareable(new FJsonObject());
	for (const auto& Param : Params)
	{
		JsonObjectData->SetStringField(Param.Key, Param.Value);
	}
	FString Data;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Data);
	FJsonSerializer::Serialize(JsonObjectData.ToSharedRef(), Writer);
	return Data;
}

void UOABackendManager::SignOut_Internal()
{
#if PLATFORM_ANDROID

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (!Env)
	{
		UE_LOG(LogTemp, Error, TEXT("SignOut_Internal: Env is null"));
		return;
	}

	jclass HelperClass = FAndroidApplication::FindJavaClass(JAVA_HELPER_CLASS);
	if (!HelperClass)
	{
		UE_LOG(LogTemp, Error, TEXT("SignOut_Internal: HelperClass is null"));
		return;
	}

	jmethodID SignOutMethod = FJavaWrapper::FindStaticMethod(Env, HelperClass, "signOut", "(Landroid/app/Activity;)V", false);
	if (!SignOutMethod)
	{
		UE_LOG(LogTemp, Error, TEXT("SignOut_Internal: signout method not found"));
		Env->DeleteLocalRef(HelperClass);
		return;
	}

	jobject Activity = FAndroidApplication::GetGameActivityThis();
	if (!Activity)
	{
		UE_LOG(LogTemp, Error, TEXT("SignOut_Internal: Activity is null"));
		Env->DeleteLocalRef(HelperClass);
		return;
	}

	Env->CallStaticVoidMethod(HelperClass, SignOutMethod, Activity);
	Env->DeleteLocalRef(HelperClass);
	
#endif
}

void UOABackendManager::SignOut()
{
	check(GatewayAPIDataAsset);
	SignOut_Internal();
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UOABackendManager::SignOutResponse);
	const FString APIUrl = GatewayAPIDataAsset->GetInvokeURL(EBackendRequestResources::SignOut);
	Request->SetURL(APIUrl);
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");

	UOAuthLocalPlayerSubsystem* LocalPlayerSubsystem = GetOAuthLocalPlayerSubsystem();
	if (!IsValid(LocalPlayerSubsystem)) return;

	TMap<FString, FString> Params = {{ TEXT("accessToken"), LocalPlayerSubsystem->AuthenticationResult.AccessToken }};
	const FString Content = SerializeJsonData(Params);
	Request->SetContentAsString(Content);
	Request->ProcessRequest();
}

void UOABackendManager::SignOutResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessfull)
{
	if (!bWasSuccessfull || !Response.IsValid())
	{
		OnSignOutSucceeded.Broadcast(false, TEXT("SognOut_Response: !bWasSuccessfull || !Response.IsValid()"));
		return;
	}
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
	{
		if (HasErrors(JsonObject))
		{
			OnSignOutSucceeded.Broadcast(false, TEXT("SognOut_Response: HasErrors"));
			return;
		}
	}
	if (UOAuthLocalPlayerSubsystem* LocalPlayerSubsystem = GetOAuthLocalPlayerSubsystem(); IsValid(LocalPlayerSubsystem))
	{
		LocalPlayerSubsystem->ClearTokens();
	}
	OnSignOutSucceeded.Broadcast(true, TEXT("Successfully Signed Out"));
}