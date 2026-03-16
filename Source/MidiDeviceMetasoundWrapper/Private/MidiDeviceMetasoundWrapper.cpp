// Copyright Epic Games, Inc. All Rights Reserved.

#include "MidiDeviceMetasoundWrapper.h"
#include "MetasoundNodeRegistrationMacro.h"
#include "Misc/EngineVersionComparison.h"



#define LOCTEXT_NAMESPACE "MidiDeviceMetasoundWrapper"



void MidiDeviceMetasoundWrapper::StartupModule()
{
#if !(UE_VERSION_OLDER_THAN(5, 7, 0))
	METASOUND_REGISTER_ITEMS_IN_MODULE
#else
	FMetasoundFrontendRegistryContainer::Get()->RegisterPendingNodes();
#endif
}

void MidiDeviceMetasoundWrapper::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

#if !(UE_VERSION_OLDER_THAN(5, 7, 0))

	METASOUND_UNREGISTER_ITEMS_IN_MODULE
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(MidiDeviceMetasoundWrapper, MidiDeviceMetasoundWrapper)


