#pragma once

#if PLATFORM_IOS
#import <Foundation/Foundation.h>
#import  <AuthenticationServices/AuthenticationServices.h>

@interface AppleSignInIOSHelper : NSObject
<ASAuthorizationControllerDelegate, ASAuthorizationControllerPresentationContextProviding>

+ (void)startSignIn;
+ (BOOL)hasResult;
+ (NSString *)consumeLastResultJson;

@end

#ifdef __cplusplus
extern "C"{
#endif
	bool AppleSignInHasResult(void);
	const char* AppleSignInConsumeLastResultJsonUTF8(void);
#ifdef __cplusplus
}
#endif
#endif