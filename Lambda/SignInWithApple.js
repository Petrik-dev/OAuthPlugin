import{
  CognitoIdentityProviderClient,
  AdminGetUserCommand,
  AdminCreateUserCommand,
  AdminSetUserPasswordCommand,
  AdminInitiateAuthCommand
} from "@aws-sdk/client-cognito-identity-provider";

import{
  DynamoDBClient,
  GetItemCommand,
  PutItemCommand
} from "@aws-sdk/client-dynamodb";

import{
  createRemoteJWKSet,
  jwtVerify
} from "jose";

const appleJWKS = createRemoteJWKSet(new URL('https://appleid.apple.com/auth/keys'));

async function verifyAppleIdToken(idToken)
{
  const options = {
    issuer: "https://appleid.apple.com",
    audience: process.env.APPLE_BUNDLE_ID
  };

  const { payload } = await jwtVerify(idToken, appleJWKS, options);

  if(!payload?.sub)
  {
    throw new Error("Invalid Apple sub");
  }

  return payload;
}

export const handler = async (event) =>
{
  const region = process.env.REGION;
  const userPoolId = process.env.USER_POOL_ID;
  const clientId = process.env.CLIENT_ID;
  const playersTableName = process.env.PLAYERS_TABLE;

  const cognito = new CognitoIdentityProviderClient({region});
  const dynamo = new DynamoDBClient({ region });

  try
  {
    const idToken = event?.idToken;
    if(!idToken) return { error: "Missing idToken" };

    const appleData = await verifyAppleIdToken(idToken);
    const appleSub = appleData.sub;
    const email = appleData.email;
    const username = `apple_${appleSub}`;
    const tempPassword = `Tmp#${appleSub}#Pass`;

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

      let nicknameFromDB = "Player123";

      try
      {
        const getPlayerResp = await dynamo.send(new GetItemCommand({
          TableName: playersTableName,
          Key: { cognito_username: { S: username } },
          ConsistentRead: true,
        }));

        if(getPlayerResp.Item)
        {
          if(getPlayerResp.Item.nickname?.S)
          {
            nicknameFromDB = getPlayerResp.Item.nickname.S;
          }
        }
        else
        {
          await dynamo.send(
            new PutItemCommand({
              TableName: playersTableName,
              Item: {
                cognito_username: { S: username },
                nickname: { S: nicknameFromDB },
              },
              ConditionExpression: "attribute_not_exists(cognito_username)",
            }));
        }
      } catch
      {
        // ignore
      }
      
      return{
        AccessToken: authResp.AuthenticationResult.AccessToken,
        IdToken: authResp.AuthenticationResult.IdToken,
        RefreshToken: authResp.AuthenticationResult.RefreshToken,
        ExpiresIn: authResp.AuthenticationResult.ExpiresIn,
        Nickname: nicknameFromDB,
      };
  } catch (error)
  {
    return { error: error?.message || "Unknown error" };
  }
};