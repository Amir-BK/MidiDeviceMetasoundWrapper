// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MidiDeviceMetasoundWrapper : ModuleRules
{
    private bool bStrictIncludesCheck = true;

    public MidiDeviceMetasoundWrapper(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = false;

        // This next flag is needed as there are Metasound node derivatives
        // that are implemented in .cpp files only... no header files.
        IWYUSupport = IWYUSupport.None;

        // This is to emulate engine installation and verify includes during development
        // Gives effect similar to BuildPlugin with -StrictIncludes
        if (bStrictIncludesCheck)
        {
            bUseUnity = false;
            PCHUsage = PCHUsageMode.NoPCHs;
            // Enable additional checks used for Engine modules
            bTreatAsEngineModule = true;
        }

        PublicIncludePaths.AddRange(
            new string[] {
				// ... add public include paths required here ...
				//engine_path + "Plugins/Runtime/Metasound/Source/MetasoundEngine/",
				//engine_path + "Plugins/Runtime/Metasound/Source/MetasoundEngine/Private/",
			}
            );

        PrivateIncludePaths.AddRange(
            new string[] {
				// ... add other private include paths required here ...
			}
            );

        //PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "../ThirdParty/Sfizz/"));

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Engine",
                "Core",
                "CoreUObject",
                "MetasoundStandardNodes",
                "HarmonixDsp",
                "HarmonixMidi",
                "Harmonix",
                "HarmonixMetasound",
                "AudioExtensions",
                "SignalProcessing",
                "MetasoundEngine",
                "MetasoundGraphCore",
                "MetasoundFrontend",
                "MetasoundGenerator",
                "AudioMixer",
				"MidiDeviceWrapperSubsystem",
				"MIDIDevice",
                //MidiDevice should be Mac PC only // WE DON'T WANT THIS // this module should be wholly ignorant of MIDIDevce...

				// ... add other public dependencies that you statically link with here ...
			}

            );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                 "SignalProcessing"
                // ... add private dependencies that you statically link with here ...
			}
            );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );
    }
}