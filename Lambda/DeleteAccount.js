import{
  CognitoIdentityProviderClient,
  GetUserCommand,
  AdminDeleteUserCommand
} from "@aws-sdk/client-cognito-identity-provider";

import{
  DynamoDBClient,
  DeleteItemCommand
} from "@aws-sdk/client-dynamodb";

const cognitoIdentityProviderClient = new CognitoIdentityProviderClient({region: process.env.REGION});
const dynamoDBClient = new DynamoDBClient({region: process.env.REGION});

const playersTableName = process.env.TABLE_NAME;
const userPoolId = process.env.USER_POOL_ID;

export const handler = async (event) => {
  console.log("Lambda DeleteAccount handler start");

  const accessToken = event?.accessToken;

  if(!accessToken)
  {
    console.log("[Error] Missing accessToken in event");
    return{
      errorType: "BadRequest",
      errorMessage: "Missing accessToken in event"
    };
  }

  try
  {
    const getUserInput = { AccessToken: accessToken };
    const getUserCommand = new GetUserCommand(getUserInput);

    const getUserResponse = await cognitoIdentityProviderClient.send(getUserCommand);
    console.log("GetUser raw response: ", getUserResponse);

    const username = getUserResponse?.Username;
    if(!username)
    {
      console.log("[Error] Cannot resolve username from accessToken");
      return{
        errorType: "NotAuthorizedException",
        errorMessage: "Cannot resolve username from accessToken"
      };
    }

    const deleteItemInput = {
      TableName: playersTableName,
      Key: { cognito_username: { S: username } },
      ConditionalExpression: "attribute_exists(cognito_username)"
    };

    const deleteItemCommand = new DeleteItemCommand(deleteItemInput);
    const deleteItemResponse = await dynamoDBClient.send(deleteItemCommand);

    console.log("DeleteItem raw response: ", deleteItemResponse);

    const adminDeleteUserInput = { UserPoolId: userPoolId, Username: username };
    const adminDeleteUserCommand = new AdminDeleteUserCommand(adminDeleteUserInput);
    const adminDeleteUserResponse = await cognitoIdentityProviderClient.send(adminDeleteUserCommand);

    console.log("AdminDeleteUser raw response: ", adminDeleteUserResponse);
    console.log("Lambda DeleteAccount handler success");

    return { success: true };

  } catch(error)
  {
    console.log("[Error] DeleteAccount handler error: ", error);

    if(error?.name === "NotAuthorizedException")
    {
      return{
        errorType: "NotAuthorizedException",
        errorMessage: "Invalid accessToken provided" 
      };
    }

    if(error?.name === "ConditionalCheckFailedException")
    {
      return{
        errorType: "NotFound",
        errorMessage: "Player not found"
      };
    }

    return{
      errorType: error?.name ?? "UnknownError",
      errorMessage: error?.message ?? "Unknown error"
    };
  };

}