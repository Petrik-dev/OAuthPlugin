import{
  CognitoIdentityProviderClient,
  AdminGetUserCommand,
  AdminCreateUserCommand,
  AdminSetUserPasswordCommand,
  AdminInitiateAuthCommand
} from "@aws-sdk/client-cognito-identity-provider";

async function verifyGoogleIdToken(idToken)
{
  const url = `https://oauth2.googleapis.com/tokeninfo?id_token=${encodeURIComponent(idToken)}`;

  const res = await fetch(url);
  if(!res.ok)
  {
    const text = await res.text().catch(() => "");
    throw new Error(`Google tokeninfo errpr: HTTP ${res.status} ${text}`);
  }

  const data = await res.json();

  if(process.env.GOOGLE_CLIENT_ID)
  {
    if(data.aud !== process.env.GOOGLE_CLIENT_ID)
    {
      throw new Error("Invalid Google client ID (aud mismatch)");
    }
  }
  return data;
}

export const handler = async (event) =>
{
  const region = process.env.REGION;
  const userPoolId = process.env.USER_POOL_ID;
  const clientId = process.env.CLIENT_ID;

  const cognito = new CognitoIdentityProviderClient({region});

  try
  {
    const idToken = event?.idToken;
    if(!idToken) return { error: "Missing idToken" };

    const googleData = await verifyGoogleIdToken(idToken);
    const googleSub = googleData.sub;
    const email = googleData.email;
    const username = `google_${googleSub}`;
    const tempPassword = `Tmp#${googleSub}#Pass`;

    let userExists = true;
    let userStatus = "";

    try
    {
      const getUserResp = await cognito.send(new AdminGetUserCommand({
        UserPoolId: userPoolId,
        Username: username
      }));
      userStatus = getUserResp.UserStatus || "";
    } catch
    {
      userExists = false;
    }

    if(!userExists)
    {
      await cognito.send(new AdminCreateUserCommand({
        UserPoolId: userPoolId,
        Username: username,
        TemporaryPassword: tempPassword,
        MessageAction: "SUPPRESS",
        UserAttributes: email ? [{ Name: "email", Value: email }] : []
    }));
      userStatus = "FORCE_CHANGE_PASSWORD";
    }

    if(userStatus === "FORCE_CHANGE_PASSWORD")
    {
      await cognito.send(new AdminSetUserPasswordCommand({
        UserPoolId: userPoolId,
        Username: username,
        Password: tempPassword,
        Permanent: true
      }));
    }

    const authResp = await cognito.send(new AdminInitiateAuthCommand({
      UserPoolId: userPoolId,
      ClientId: clientId,
      AuthFlow: "ADMIN_USER_PASSWORD_AUTH",
      AuthParameters: {
        USERNAME: username,
        PASSWORD: tempPassword
      },
      }));

      if(!authResp.AuthenticationResult)
      {
        throw new Error("No AuthenticationResult from Cognito");
      }
      
      return{
        AccessToken: authResp.AuthenticationResult.AccessToken,
        IdToken: authResp.AuthenticationResult.IdToken,
        RefreshToken: authResp.AuthenticationResult.RefreshToken,
        ExpiresIn: authResp.AuthenticationResult.ExpiresIn
      };
  } catch (error)
  {
    return { error: error?.message || "Unknown error" };
  }
};