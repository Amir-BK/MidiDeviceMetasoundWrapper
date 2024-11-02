// Copyright Epic Games, Inc. All Rights Reserved.

#include "MidiDeviceWrapperEditor.h"
#include "UObject/UObjectArray.h"
#include "Serialization/JsonSerializer.h"
//#include "MetasoundUObjectRegistry.h"
#include "Misc/EngineVersionComparison.h"
#include "Modules/ModuleManager.h"

#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#include "MetasoundDataTypeRegistrationMacro.h"
#include "MusicDeviceControllerSubsystem.h"
#include "MetasoundEditorGraphNode.h"


#define LOCTEXT_NAMESPACE "MidiDeviceWrapperEditorModule"

void MidiDeviceWrapperEditor::StartupModule()
{

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 4

#else
	//get metasound editor module
	using namespace Metasound::Editor;

	//get IMetasoundEditorModule
	IMetasoundEditorModule& MetasoundEditorModule = FModuleManager::LoadModuleChecked<IMetasoundEditorModule>("MetasoundEditor");

	MetasoundEditorModule.RegisterGraphNodeVisualization(FName("unDAW.MidiStreamInput"), Metasound::Editor::FOnCreateGraphNodeVisualizationWidget::CreateStatic(&MidiDevice::MetasoundWidget::CreateMidiVirtualKeyboardWidget));

#endif

}

void MidiDeviceWrapperEditor::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(MidiDeviceWrapperEditor, MidiDeviceWrapperEditor)

TSharedRef<SWidget> MidiDevice::MetasoundWidget::CreateMidiVirtualKeyboardWidget(const Metasound::Editor::FCreateGraphNodeVisualizationWidgetParams& InParams)
{
	TSharedPtr<SHorizontalBox> KeyboardWidget;
	auto Node = InParams.MetaSoundNode;
	//Node->GetOp
	auto ThePin = Node->FindPin(TEXT("Midi Device Name"));
	//auto DataInfo = Node->GetPinDataTypeInfo(*ThePin);
	auto LiteralValue = ThePin->GetDefaultAsText();
	

	auto MainBox = SNew(SBox)
		.MinDesiredWidth(300)
		.MinDesiredHeight(75)
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
						.Text_Lambda([Node, ThePin] {
						const FName DeviceName(TEXT("Midi Device Name"));
						
					//	return FText::FromString(Node->GetPinVa GetPinVisualizationValue<FString>(DeviceName).GetValue()); })
						return ThePin->GetDefaultAsText(); })
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SAssignNew(KeyboardWidget, SHorizontalBox)

				]

		];

	const int NumKeys = 36;
	const int FirstKey = 48;

	for (int i = 0; i < NumKeys; i++)
	{
		const int Key = FirstKey + i;
		const bool bIsBlackKey = (Key % 12 == 1) || (Key % 12 == 3) || (Key % 12 == 6) || (Key % 12 == 8) || (Key % 12 == 10);

		KeyboardWidget->AddSlot()
			.AutoWidth()
			[
				SNew(SBox)
					.WidthOverride(10)
					[
						SNew(SButton)
							.Text(FText::FromString(FString::Printf(TEXT("%d"), Key)))
							.OnPressed_Lambda([Key, Node, ThePin] {
							UMusicDeviceControllerSubsystem::TransmitNoteOnForDevice(FName(ThePin->GetDefaultAsString()), Key, 127);
								})
							.OnReleased_Lambda([Key, Node, ThePin] {
							UMusicDeviceControllerSubsystem::TransmitNoteOffForDevice(FName(ThePin->GetDefaultAsString()), Key, 0);
								})

							.ButtonStyle(FAppStyle::Get(), !bIsBlackKey ? "FlatButton.Dark" : "FlatButton")
					]

			];
	}

	return MainBox;

}