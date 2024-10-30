// Copyright Epic Games, Inc. All Rights Reserved.

#include "MidiDeviceWrapperSubsystem.h"
#include "UObject/UObjectArray.h"
#include "Serialization/JsonSerializer.h"
//#include "MetasoundUObjectRegistry.h"

#include "MetasoundDataTypeRegistrationMacro.h"


#define LOCTEXT_NAMESPACE "MidiDeviceWrapperSubsystemModule"

void MidiDeviceWrapperSubsystem::StartupModule()
{
#ifdef WITH_CHUNREAL_PLUGIN
	UE_LOG(LogTemp, Warning, TEXT("WE SEE CHUNREAL!"));
#endif 


}

void MidiDeviceWrapperSubsystem::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(MidiDeviceWrapperSubsystem, MidiDeviceWrapperSubsystem)