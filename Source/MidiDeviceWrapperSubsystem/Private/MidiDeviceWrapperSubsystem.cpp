// Copyright Epic Games, Inc. All Rights Reserved.

#include "MidiDeviceWrapperSubsystem.h"
#include "UObject/UObjectArray.h"
#include "Serialization/JsonSerializer.h"
//#include "MetasoundUObjectRegistry.h"

#include "MetasoundDataTypeRegistrationMacro.h"
#include "Misc/EngineVersionComparison.h"
#include "VirtualMidiDevice.h"


#define LOCTEXT_NAMESPACE "MidiDeviceWrapperSubsystemModule"

void MidiDeviceWrapperSubsystem::StartupModule()
{

#if !(UE_VERSION_OLDER_THAN(5, 7, 0))

	Metasound::Frontend::FModuleInfo ModuleInfo{};
#if WITH_EDITORONLY_DATA
	ModuleInfo.PluginName = TEXT("MidiDeviceMetasoundWrapper");
	ModuleInfo.ModuleName = TEXT("MidiDeviceWrapperSubsystem");
#endif
	Metasound::Frontend::RegisterDataType<
		Metasound::FVirtualMidiDeviceAsset,
		Metasound::ELiteralType::UObjectProxy,
		UVirtualMidiDevice>(ModuleInfo);

#endif
}

void MidiDeviceWrapperSubsystem::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(MidiDeviceWrapperSubsystem, MidiDeviceWrapperSubsystem)