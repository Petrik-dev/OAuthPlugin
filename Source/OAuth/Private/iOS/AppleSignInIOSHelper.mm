#import "AppleSignInIOSHelper.h"
#import <UIKit/UIKit.h>

static NSString *LastAppleResultJson = nil;
static BOOL HasAppleResult = NO;
static AppleSignInIOSHelper *CurrentHelper = nil;

@implementation AppleSignInIOSHelper

+ (void)setResultFromDictionary:(NSDictionary *)payload
{
    NSError* jsonError = nil;
    NSData* jsonData =
    [NSJSONSerialization dataWithJSONObject:payload options:0 error:&jsonError];

    NSString* jsonString = nil;

    if (jsonError || !jsonData)
    {
        jsonString = @"{\"success\":false,\"error\":\"JsonSerializationFailed\"}";
    }
    else
    {
        jsonString =
        [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
    }

    if (LastAppleResultJson)
        [LastAppleResultJson release];

    LastAppleResultJson = [jsonString copy];
    HasAppleResult = YES;
}

+ (void)startSignIn
{
    if (LastAppleResultJson)
    {
        [LastAppleResultJson release];
        LastAppleResultJson = nil;
    }

    HasAppleResult = NO;

    if (@available(iOS 13.0, *))
    {
        if (CurrentHelper)
            [CurrentHelper release];

        CurrentHelper = [[AppleSignInIOSHelper alloc] init];

        ASAuthorizationAppleIDProvider* provider =
        [[ASAuthorizationAppleIDProvider alloc] init];

        ASAuthorizationAppleIDRequest* request = [provider createRequest];
        request.requestedScopes = @[];

        ASAuthorizationController* controller =
        [[ASAuthorizationController alloc]
         initWithAuthorizationRequests:@[request]];

        controller.delegate = CurrentHelper;
        controller.presentationContextProvider = CurrentHelper;

        [controller performRequests];
    }
}

- (ASPresentationAnchor)presentationAnchorForAuthorizationController:
(ASAuthorizationController *)controller
API_AVAILABLE(ios(13.0))
{
    for (UIScene* scene in [UIApplication sharedApplication].connectedScenes)
    {
        if (![scene isKindOfClass:[UIWindowScene class]])
            continue;

        if (scene.activationState != UISceneActivationStateForegroundActive)
            continue;

        UIWindowScene* windowScene = (UIWindowScene*)scene;

        for (UIWindow* window in windowScene.windows)
        {
            if (window.isKeyWindow)
                return window;
        }

        if (windowScene.windows.count > 0)
            return windowScene.windows.firstObject;
    }

    return nil;
}

- (void)authorizationController:(ASAuthorizationController *)controller
didCompleteWithAuthorization:(ASAuthorization *)authorization
API_AVAILABLE(ios(13.0))
{
    ASAuthorizationAppleIDCredential* credential = authorization.credential;

    NSMutableDictionary* payload = [NSMutableDictionary dictionary];

    payload[@"success"] = @YES;
    payload[@"error"] = @"";

    NSString* tokenString = @"";

    if (credential.identityToken)
    {
        tokenString = [[[NSString alloc] initWithData:credential.identityToken
            encoding:NSUTF8StringEncoding] autorelease] ?: @"";
    }

    payload[@"idToken"] = tokenString;

    [[self class] setResultFromDictionary:payload];
}

- (void)authorizationController:(ASAuthorizationController *)controller
didCompleteWithError:(NSError *)error
API_AVAILABLE(ios(13.0))
{
    NSDictionary* payload = @{
        @"success":@NO,
        @"error":error.localizedDescription ?: @"Apple Sign-In error"
    };

    [[self class] setResultFromDictionary:payload];
}

+ (BOOL)hasResult
{
    return HasAppleResult;
}

+ (NSString *)consumeLastResultJson
{
    NSString *result = LastAppleResultJson;

    LastAppleResultJson = nil;
    HasAppleResult = NO;

    return [result autorelease];
}

@end


extern "C" bool AppleSignInHasResult(void)
{
    return [AppleSignInIOSHelper hasResult];
}

extern "C" const char* AppleSignInConsumeLastResultJsonUTF8(void)
{
    NSString *json = [AppleSignInIOSHelper consumeLastResultJson];
    return json ? [json UTF8String] : "";
}