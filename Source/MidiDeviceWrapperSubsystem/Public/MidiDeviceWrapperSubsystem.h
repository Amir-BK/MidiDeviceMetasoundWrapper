// Copyright FlyKick Studios.

#pragma once

#include "CoreMinimal.h"
//#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

class MidiDeviceWrapperSubsystem : public IModuleInterface
{
public:
	FString PluginContentDir;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
