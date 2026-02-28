import{
  CognitoIdentityProviderClient,
  GlobalSignOutCommand
} from "@aws-sdk/client-cognito-identity-provider";

const cognitoIdentityProviderClient = new CognitoIdentityProviderClient({region: process.env.REGION});

export const handler = async (event) => {
  console.log(" Labda signout handler start ");

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
    const input = { AccessToken: accessToken };
    const globalSignOutCommand = new GlobalSignOutCommand(input);

    const globalSignOutResponse = await cognitoIdentityProviderClient.send(globalSignOutCommand);

    console.log(" Lambda signout handler Success");
    console.log(" GlobalSignOut raw response: ", globalSignOutResponse);
    return {  success: true };
  } catch(error)
  {
    console.log("Lambda signout handler Error");
    console.log("Error: ", error);

    return{
      errorType: error.name ?? "UnknownError",
      errorMessage: error.message ?? "Unknown error",
    };
  }
};