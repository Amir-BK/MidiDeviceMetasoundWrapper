// Copyright FlyKick Studios.

#pragma once

#include "CoreMinimal.h"
//#include "Interfaces/IPluginManager.h"
#include "MetasoundEditorModule.h"
#include "Misc/EngineVersionComparison.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SWidget.h"

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 4


namespace MidiDevice::MetasoundWidget
{
	
	TSharedRef<SWidget> CreateMidiVirtualKeyboardWidget(const Metasound::Editor::FCreateGraphNodeVisualizationWidgetParams& InParams);


}

#endif

class MidiDeviceWrapperEditor : public IModuleInterface
{
public:
	FString PluginContentDir;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
