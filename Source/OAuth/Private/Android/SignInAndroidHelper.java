package com.Plugins.SignInAndroidHelper;

import android.app.Activity;
import android.content.Intent;
import android.util.Log;
import androidx.annotation.Keep;

import com.google.android.gms.auth.api.signin.GoogleSignIn;
import com.google.android.gms.auth.api.signin.GoogleSignInAccount;
import com.google.android.gms.auth.api.signin.GoogleSignInClient;
import com.google.android.gms.auth.api.signin.GoogleSignInOptions;

import com.google.android.gms.common.api.ApiException;
import com.google.android.gms.tasks.Task;

@Keep
public class SignInAndroidHelper
{
   private static final String TAG = "SignInAndroidHelper";
   
   private static final int RC_SIGN_IN = 9001;
   
   private static GoogleSignInClient sClient = null;
   
   private static String sLastResultJson = "";
   private static boolean sHasResult = false;
   
   @Keep
   public static void startSignIn(Activity activity, String serverClientId)
   {
        if(activity == null)
        {
            Log.e(TAG, "startSignIn: Activity is null");
            setResult(false, "Activity is null", "", "", "");
            return;
        }

        GoogleSignInOptions.Builder gsoBuilder = new GoogleSignInOptions.Builder(GoogleSignInOptions.DEFAULT_SIGN_IN).requestIdToken(serverClientId);
        
        //gsoBuilder.requestEmail();
        //gsoBuilder.requestProfile();
        //gsoBuilder.requestScopes(new Scope(DriveScopes.DRIVE_READONLY));
        
        GoogleSignInOptions gso = gsoBuilder.build();
        sClient = GoogleSignIn.getClient(activity, gso);
        Intent signInIntent = sClient.getSignInIntent();
        activity.startActivityForResult(signInIntent, RC_SIGN_IN);
        
   }
   
  @Keep
  public static void onActivityResult(Activity activity, int requestCode, int resultCode, Intent data)
  {
       if(requestCode != RC_SIGN_IN)
       {
           return;
       }
       Task<GoogleSignInAccount> task = GoogleSignIn.getSignedInAccountFromIntent(data);
       
       try
       {
           GoogleSignInAccount account = task.getResult(ApiException.class);
           String idToken = account != null ? account.getIdToken() : "";
           
           // String email = account != null ? account.getEmail() : "";
           // String displayName = account != null ? account.getDisplayName() : "";
           
           setResult(true,
            "",
            idToken,
            // email,
            // displayName,
            "",
            "");  
           Log.d(TAG, "Google sign-in success");
       } catch (ApiException e)
       {
           Log.e(TAG, "Google sign-in failed: " + e.getStatusCode() + " " + e.getMessage(), e);
           setResult(false, "ApiException code=" + e.getStatusCode() + " message=" + e.getMessage(), "", "", "");
       }
  }

  @Keep
  public static String consumeLastResultJson()
  {
		if(!sHasResult)
		{
			return "";
		}
		sHasResult = false;
		String result = sLastResultJson;
		sLastResultJson = "";
		return result;
  }
  
  private static void setResult(boolean success, String errorMessage, String idToken, String email, String displayName)
  {
       StringBuilder sb = new StringBuilder();
        
       sb.append("{");
       sb.append("\"success\":").append(success).append(",");
       sb.append("\"error\":\"").append(errorMessage).append("\",");
       sb.append("\"idToken\":\"").append(idToken).append("\",");
       sb.append("\"email\":\"").append(email).append("\",");
       sb.append("\"displayName\":\"").append(displayName).append("\"");
       sb.append("}");
        
       sLastResultJson = sb.toString();
       sHasResult = true;

  }
  

}

