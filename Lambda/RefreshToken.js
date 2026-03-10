import{
  CognitoIdentityProviderClient,
  AdminInitiateAuthCommand
} from "@aws-sdk/client-cognito-identity-provider"

const cognitoIdentityProviderClient = new CognitoIdentityProviderClient({ region: process.env.REGION });

const userPoolId = process.env.USER_POOL_ID;
const clientId = process.env.CLIENT_ID;

export const handler = async (event) => {
  console.log("Lambda RefreshToken handler start");

  const refreshToken = event?.refreshToken;
  if(!refreshToken)
  {
    console.log("[Error] Missing refreshToken in event");
    return{
      forceLogout: false,
      errorType: "BadRequest",
      errorMessage: "Missing refreshToken in event"
    };
  }

  try
  {
    const adminInitiateAuthInput = {
      UserPoolId: userPoolId,
      ClientId: clientId,
      AuthFlow: "REFRESH_TOKEN_AUTH",
      AuthParameters: {
        REFRESH_TOKEN: refreshToken
      }
    };

    const adminInitiateAuthCommand = new AdminInitiateAuthCommand(adminInitiateAuthInput);
    const adminInitiateAuthResponse = await cognitoIdentityProviderClient.send(adminInitiateAuthCommand);

    console.log("AdminInitiateAuth raw response: ", adminInitiateAuthResponse);
    if(!adminInitiateAuthResponse.AuthenticationResult)
    {
      console.log("[Error] Missing AuthenticationResult in AdminInitiateAuth response");
      return{
        forceLogout: false,
        errorType: "UnknownError",
        errorMessage: "Missing AuthenticationResult in AdminInitiateAuth response"
      };
    }

    console.log("Lambda refreshtoken handler success");

    return{
      AccessToken: adminInitiateAuthResponse.AuthenticationResult.AccessToken,
      IdToken: adminInitiateAuthResponse.AuthenticationResult.IdToken,
      RefreshToken: refreshToken,
      ExpiresIn: adminInitiateAuthResponse.AuthenticationResult.ExpiresIn,
      TokenType: adminInitiateAuthResponse.AuthenticationResult.TokenType
    };
  } catch(error)
  {
    console.log("[Error] Exception in RefreshToken handler: ", error);
    
    const errorMap = {
      NotAuthorizedException: "Invalid refreshToken",
      InvalidParameterException: "Invalid refreshToken",
      UserNotFoundException: "User does not exist"
    };

    const errorType = error?.name;
    const errorMessage = errorMap[errorType];

    return{
      forceLogout: Boolean(errorMessage),
      errorType: errorType ?? "UnknownError",
      errorMessage: errorMessage ?? error?.message ?? "Unknown error"
    };
  }

}