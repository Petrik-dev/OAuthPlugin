#pragma once

#if PLATFORM_IOS
#import <Foundation/Foundation.h>

@interface GoogleSignInIOSHelper : NSObject

+ (void)startSignInWithClientId:(NSString *)clientId;
+ (BOOL)hasResult;
+ (NSString *)consumeLastResultJson;

@end

#ifdef __cplusplus
extern "C" {
#endif
	bool GoogleSignInHasResult(void);
	const char* GoogleSignInConsumeLastResultJsonUTF8(void);
#ifdef __cplusplus
}
#endif

#endif