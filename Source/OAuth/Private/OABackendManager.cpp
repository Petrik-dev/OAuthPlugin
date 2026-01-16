// Fill out your copyright notice in the Description page of Project Settings.


#include "OABackendManager.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#endif

void UOABackendManager::SignInWithGoogle()
{
	SignInWithGoogle_Internal();
}

void UOABackendManager::SignInWithGoogle_Internal()
{
#if PLATFORM_ANDROID

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (!Env)
	{
		UE_LOG(LogTemp, Error, TEXT("SignInWithGoogle_Internal: Env is NULL"));
		return;
	}

	const char* ClassName = "com/Plugins/SignInAndroidHelper/SignInAndroidHelper";

	jclass HelperClass = FAndroidApplication::FindJavaClass(ClassName);
	if (!HelperClass)
	{
		UE_LOG(LogTemp, Error, TEXT("SignInWithGoogle_Internal: HelperClass is NULL"));
		return;
	}

	jmethodID HelloMethod = FJavaWrapper::FindsStaticMethod(
		Env,
		HelperClass,
		"helloFromAndroid",
		"()Ljava/lang/String;",
		false);

	if (!HelloMethod)
	{
		UE_LOG(LogTemp, Error, TEXT("SignInWithGoogle_Internal: HelloMethod is NULL"));
		Env->DeleteLocalRef(HelperClass);
		return;
	}

	jstring ResultJString = (jstring)FJavaWrapper::CallStaticObjectMethod(Env, HelperClass, HelloMethod);

	FString Result;
	if (ResultJString)
	{
		Result = FJavaHelper::FStringFromParam(Env, ResultJString);
		Env->DeleteLocalRef(ResultJString);
	}
	else
	{
		Result = TEXT("NULL");		
	}
	Env->DeleteLocalRef(HelperClass);
	UE_LOG(LogTemp, Log, TEXT("Java says: %s"), Result);
#else
	UE_LOG(LogTemp, Warning, TEXT("Android-only code, nothing to do on this platform"));
	
	#endif
}
