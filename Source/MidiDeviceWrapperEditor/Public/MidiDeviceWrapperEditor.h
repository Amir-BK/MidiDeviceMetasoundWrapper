// Copyright FlyKick Studios.

#pragma once

#include "CoreMinimal.h"
//#include "Interfaces/IPluginManager.h"
#include "MetasoundEditorModule.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SWidget.h"


namespace MidiDevice::MetasoundWidget
{
	
	TSharedRef<SWidget> CreateMidiVirtualKeyboardWidget(const Metasound::Editor::FCreateGraphNodeVisualizationWidgetParams& InParams);


}


class MidiDeviceWrapperEditor : public IModuleInterface
{
public:
	FString PluginContentDir;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
