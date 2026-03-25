// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class OAuth : ModuleRules
{
	public OAuth(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
		);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
		);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UMG",
				"HTTP",
				"Json",
				"JsonUtilities"
				// ... add private dependencies that you statically link with here ...	
			}
		);

		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateDependencyModuleNames.Add("Launch");
			
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModuleDirectory, "SignInAndroid_UPL.xml"));
		}

		if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			PrivateDependencyModuleNames.Add("Launch");
			
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "iOS"));
			AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(ModuleDirectory, "SignInIOS_UPL.xml"));
			
			PublicFrameworks.AddRange(new[]
			{
				"Foundation",
				"UIKit",
				"AuthenticationServices",
				"Security"
			});
		}
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
}