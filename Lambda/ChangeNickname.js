import{
    CognitoIdentityProviderClient,
    GetUserCommand
} from "@aws-sdk/client-cognito-identity-provider";

import{
    DynamoDBClient,
    UpdateItemCommand
} from "@aws-sdk/client-dynamodb";

const cognitoIdentityProviderClient = new CognitoIdentityProviderClient({region: process.env.REGION});
const dynamoDBClient = new DynamoDBClient({region: process.env.REGION});

const playersTableName = process.env.TABLE_NAME;

export const handler = async (event) => {
    console.log("Lambda ChangeNickname handler start ");

    const accessToken = event?.accessToken;
    const newNickname = event?.newNickname;

    if(!accessToken)
    {
        console.log("[Error] Missing accessToken in event");
        return{
            errorType: "BadRequest",
            errorMessage: "Missing accessToken in event"
        };
    }
    if(!newNickname)
    {
        console.log("[Error] Missing newNickname in event");
        return{
            errorType: "BadRequest",
            errorMessage: "Missing newNickname in event"
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
            console.log("[Error] Cannot resolve Username from accessToken");
            return{
                errorType: "NotAuthorizedException",
                errorMessage: "Cannot resolve Username from accessToken"
            };
        }

        const updateItemInput = {
            TableName: playersTableName,
            Key: {
                cognito_username: { S: username },
            },
            UpdateExpression: "SET nickname = :n",
            ExpressionAttributeValues: {
                ":n": { S: newNickname },
            },
            ConditionalExpression: "attribute_exists(cognito_username)",
        };

        const updateItemCommand = new UpdateItemCommand(updateItemInput);
        const updateItemResponse = await dynamoDBClient.send(updateItemCommand);

        console.log("Lambda UpdateNickname handler succeess");
        console.log("ChangeNickname raw response: ", updateItemResponse);

        return { success: true };
    } catch(error)
    {
        console.log("Lambda ChangeNickname handler error");
        console.log("[Error]: ", error);

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
                errorMessage: "PlayerNotFound"
            };
        }

        return{
            errorType: error?.name ?? "UnknownError",
            errorMessage: error?.message ?? "unknown error"
        };
    };
}