// Fill out your copyright notice in the Description page of Project Settings.


#include "OABackendManager.h"



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

	UE_LOG(LogTemp, Log, TEXT("Google Sign-in result: %s"), *ResultJson);
}
