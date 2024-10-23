// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class FrustumChecker : ModuleRules
{
	public FrustumChecker(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateIncludePaths.AddRange(
            new string[] {
                "FrustumChecker/Private"
            })
        ;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
        PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
