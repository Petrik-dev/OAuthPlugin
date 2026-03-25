
#import "GoogleSignInIOSHelper.h"
#import <UIKit/UIKit.h>
#import <AuthenticationServices/AuthenticationServices.h>
#import <CommonCrypto/CommonCrypto.h>
#import <Security/Security.h>

static NSString *LastResultJson = nil;
static BOOL HasResult = NO;
static ASWebAuthenticationSession *CurrentSession = nil;
static NSString *CodeVerifier = nil;

@interface WebAuthContextProvider : NSObject <ASWebAuthenticationPresentationContextProviding> {
@private
    UIWindow *_window;
}
- (instancetype)initWithWindow:(UIWindow *)window;
@end

@implementation WebAuthContextProvider

- (instancetype)initWithWindow:(UIWindow *)window
{
    self = [super init];
    if (self)
        _window = [window retain];
    return self;
}

- (void)dealloc
{
    [_window release];
    [super dealloc];
}

- (ASPresentationAnchor)presentationAnchorForWebAuthenticationSession:(ASWebAuthenticationSession *)session
{
    return _window;
}

@end

static WebAuthContextProvider *ContextProvider = nil;


static NSString *Base64URLEncode(NSData *data)
{
    if (!data) return @"";

    NSString *b64 = [data base64EncodedStringWithOptions:0];
    b64 = [b64 stringByReplacingOccurrencesOfString:@"+" withString:@"-"];
    b64 = [b64 stringByReplacingOccurrencesOfString:@"/" withString:@"_"];
    b64 = [b64 stringByTrimmingCharactersInSet:
           [NSCharacterSet characterSetWithCharactersInString:@"="]];

    return b64;
}


static NSString *GenerateCodeVerifier(void)
{
    uint8_t bytes[32];

    if (SecRandomCopyBytes(kSecRandomDefault, sizeof(bytes), bytes) != errSecSuccess)
    {
        for (NSUInteger i = 0; i < sizeof(bytes); i++)
            bytes[i] = arc4random_uniform(256);
    }

    return Base64URLEncode([NSData dataWithBytes:bytes length:sizeof(bytes)]);
}


static NSString *ComputeCodeChallenge(NSString *verifier)
{
    NSData *data = [verifier dataUsingEncoding:NSUTF8StringEncoding];

    uint8_t hash[CC_SHA256_DIGEST_LENGTH];
    CC_SHA256(data.bytes, (CC_LONG)data.length, hash);

    return Base64URLEncode([NSData dataWithBytes:hash length:sizeof(hash)]);
}


static void SetResult(BOOL success, NSString *error, NSString *token)
{
    if (!error) error = @"";
    if (!token) token = @"";

    error = [error stringByReplacingOccurrencesOfString:@"\"" withString:@"'"];
    token = [token stringByReplacingOccurrencesOfString:@"\"" withString:@"'"];

    NSString *json =
    [NSString stringWithFormat:
     @"{\"success\":%@,\"error\":\"%@\",\"idToken\":\"%@\"}",
     success ? @"true" : @"false",
     error,
     token];

    if (LastResultJson)
        [LastResultJson release];

    LastResultJson = [json copy];
    HasResult = YES;
}


static NSString *ExtractCodeFromURL(NSURL *url)
{
    NSURLComponents *components =
    [NSURLComponents componentsWithURL:url resolvingAgainstBaseURL:NO];

    for (NSURLQueryItem *item in components.queryItems)
    {
        if ([item.name isEqualToString:@"code"])
            return item.value;
    }

    return nil;
}


static void BuildRedirectInfo(
                               NSString *clientId,
                               NSString **outScheme,
                               NSString **outRedirectURI)
{
    if (outScheme) *outScheme = nil;
    if (outRedirectURI) *outRedirectURI = nil;

    if (!clientId.length)
        return;

    NSString *suffix = @".apps.googleusercontent.com";

    if (![clientId hasSuffix:suffix])
        return;

    NSString *prefix =
    [clientId substringToIndex:(clientId.length - suffix.length)];

    NSString *scheme =
    [NSString stringWithFormat:@"com.googleusercontent.apps.%@", prefix];

    NSString *uri =
    [scheme stringByAppendingString:@":/oauthredirect"];

    if (outScheme) *outScheme = scheme;
    if (outRedirectURI) *outRedirectURI = uri;
}


static UIWindow *GetPresentationWindow(void)
{
    UIApplication *app = UIApplication.sharedApplication;

    if (@available(iOS 13.0, *))
    {
        NSSet<UIScene *> *scenes = app.connectedScenes;

        for (UIScene *scene in scenes)
        {
            if (![scene isKindOfClass:[UIWindowScene class]])
                continue;

            UIWindowScene *windowScene = (UIWindowScene *)scene;

            if (windowScene.activationState != UISceneActivationStateForegroundActive)
                continue;

            for (UIWindow *window in windowScene.windows)
            {
                if (window.isKeyWindow)
                    return window;
            }

            if (windowScene.windows.count > 0)
                return windowScene.windows.firstObject;
        }
    }

    return nil;
}


static void CleanupSession(void)
{
    if (CurrentSession)
    {
        [CurrentSession release];
        CurrentSession = nil;
    }

    if (ContextProvider)
    {
        [ContextProvider release];
        ContextProvider = nil;
    }
}


static void ExchangeCodeForTokens(
                                  NSString *code,
                                  NSString *clientId,
                                  NSString *redirectURI)
{
    NSString *verifier = [CodeVerifier copy];

    if (CodeVerifier)
    {
        [CodeVerifier release];
        CodeVerifier = nil;
    }

    NSURL *url =
    [NSURL URLWithString:@"https://oauth2.googleapis.com/token"];

    NSMutableURLRequest *request =
    [[[NSMutableURLRequest alloc] initWithURL:url] autorelease];

    [request setHTTPMethod:@"POST"];

    [request setValue:@"application/x-www-form-urlencoded"
   forHTTPHeaderField:@"Content-Type"];

    NSString *body =
    [NSString stringWithFormat:
     @"code=%@&client_id=%@&redirect_uri=%@&grant_type=authorization_code&code_verifier=%@",
     code,
     clientId,
     redirectURI,
     verifier];

    [request setHTTPBody:
     [body dataUsingEncoding:NSUTF8StringEncoding]];

    [[NSURLSession.sharedSession
      dataTaskWithRequest:request
      completionHandler:^(NSData *data,
                          NSURLResponse *response,
                          NSError *error)
      {

        if (error)
        {
            SetResult(NO, error.localizedDescription, @"");
            return;
        }

        NSDictionary *json =
        [NSJSONSerialization JSONObjectWithData:data
                                        options:0
                                          error:nil];

        NSString *token = json[@"id_token"];

        if (!token)
            SetResult(NO, @"no_id_token", @"");
        else
            SetResult(YES, nil, token);

    }] resume];

    [verifier release];
}


static void StartOAuthFlow(NSString *clientId)
{
    UIWindow *window = GetPresentationWindow();

    if (!window)
    {
        SetResult(NO, @"no_window", @"");
        return;
    }

    NSString *scheme = nil;
    NSString *redirectURI = nil;

    BuildRedirectInfo(clientId, &scheme, &redirectURI);

    if (!scheme || !redirectURI)
    {
        SetResult(NO, @"invalid_client_id", @"");
        return;
    }

    NSString *verifier = GenerateCodeVerifier();
    NSString *challenge = ComputeCodeChallenge(verifier);

    if (CodeVerifier)
        [CodeVerifier release];

    CodeVerifier = [verifier copy];

    NSURLComponents *components =
    [NSURLComponents componentsWithString:
     @"https://accounts.google.com/o/oauth2/v2/auth"];

    components.queryItems = @[
        [NSURLQueryItem queryItemWithName:@"client_id"
                                    value:clientId],

        [NSURLQueryItem queryItemWithName:@"redirect_uri"
                                    value:redirectURI],

        [NSURLQueryItem queryItemWithName:@"response_type"
                                    value:@"code"],

        [NSURLQueryItem queryItemWithName:@"scope"
                                    value:@"openid"],

        [NSURLQueryItem queryItemWithName:@"code_challenge"
                                    value:challenge],

        [NSURLQueryItem queryItemWithName:@"code_challenge_method"
                                    value:@"S256"],

        [NSURLQueryItem queryItemWithName:@"prompt"
                                    value:@"select_account"]
    ];

    CleanupSession();

    ContextProvider =
    [[WebAuthContextProvider alloc] initWithWindow:window];

    CurrentSession =
    [[ASWebAuthenticationSession alloc]
     initWithURL:components.URL
     callbackURLScheme:scheme
     completionHandler:^(NSURL *callbackURL, NSError *error)
     {

        if (error)
        {
            SetResult(NO, error.localizedDescription, @"");
            CleanupSession();
            return;
        }

        NSString *code = ExtractCodeFromURL(callbackURL);

        if (!code)
        {
            SetResult(NO, @"no_code", @"");
            CleanupSession();
            return;
        }

        CleanupSession();

        ExchangeCodeForTokens(code, clientId, redirectURI);

    }];

    CurrentSession.presentationContextProvider = ContextProvider;
    CurrentSession.prefersEphemeralWebBrowserSession = NO;

    [CurrentSession start];
}


@implementation GoogleSignInIOSHelper

+ (void)startSignInWithClientId:(NSString *)clientId
{
    if (LastResultJson)
    {
        [LastResultJson release];
        LastResultJson = nil;
    }

    HasResult = NO;

    StartOAuthFlow(clientId);
}

+ (BOOL)hasResult
{
    return HasResult;
}


+ (NSString *)consumeLastResultJson
{
    NSString *result = LastResultJson;

    LastResultJson = nil;
    HasResult = NO;

    return [result autorelease];
}

@end


extern "C" bool GoogleSignInHasResult(void)
{
    return [GoogleSignInIOSHelper hasResult];
}

extern "C" const char* GoogleSignInConsumeLastResultJsonUTF8(void)
{
    NSString *json = [GoogleSignInIOSHelper consumeLastResultJson];
    return json ? [json UTF8String] : "";
}