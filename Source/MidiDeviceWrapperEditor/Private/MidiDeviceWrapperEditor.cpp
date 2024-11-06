// Copyright Epic Games, Inc. All Rights Reserved.

#include "MidiDeviceWrapperEditor.h"
#include "UObject/UObjectArray.h"
#include "Serialization/JsonSerializer.h"
//#include "MetasoundUObjectRegistry.h"

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

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 4

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

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 4

class SMidiVirtualKeyboard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMidiVirtualKeyboard) {}
	SLATE_END_ARGS()

	TSharedPtr<SHorizontalBox> KeyboardWidget;
	UMetasoundEditorGraphNode* Node;
	FName DeviceName;
	int OctaveOffset = 0;

	void Tick(const FGeometry& AllottedGeometry,
		const double InCurrentTime,
		const float InDeltaTime
	) override
	{
		//if (Node->GetMidiDeviceName() != DeviceName)
		//{
		//	DeviceName = Node->GetMidiDeviceName();
		//	//recreate the keyboard
		//	KeyboardWidget->ClearChildren();
		//	Construct(SNew(SMidiVirtualKeyboard), Node);
		//}
		const auto& ThePin = Node->FindPin(TEXT("Midi Device Name"));

			const FName NewDeviceName = FName(ThePin->GetDefaultAsString());
			if (NewDeviceName != DeviceName)
			{
				DeviceName = NewDeviceName;
				//recreate the keyboard
				//KeyboardWidget->ClearChildren();
				//Construct(SNew(SMidiVirtualKeyboard), Node);
			}
	}

	void Construct(const FArguments& InArgs, const Metasound::Editor::FCreateGraphNodeVisualizationWidgetParams& InParams)
	{
		const int NumKeys = 36;
		const int FirstKey = 48;
		Node = InParams.MetaSoundNode;
	//ode->

		ChildSlot[
			SNew(SBox)
				.MinDesiredWidth(300)
				.MinDesiredHeight(75)
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
								//+ octave button
								+ SHorizontalBox::Slot()
								[
									SNew(SButton)
										.Text(FText::FromString("-"))
										.OnPressed_Lambda([this] {
										OctaveOffset = FMath::Max(OctaveOffset - 1 , -2);
											})
								]
								+ SHorizontalBox::Slot()
								[
									SNew(SButton)
										.Text(FText::FromString("+"))
										.OnPressed_Lambda([this] {
										OctaveOffset = FMath::Min(OctaveOffset+ 1, 2);
											})
								]

								+ SHorizontalBox::Slot()
								[
									SNew(STextBlock)
										.Text_Lambda([this] {
										return FText::FromString(FString::Printf(TEXT("%d"), OctaveOffset));
											})
								]

								+ SHorizontalBox::Slot()
								[
									SNew(STextBlock)
										.Text_Lambda([this] {
										return FText::FromString(FString::Printf(TEXT("%s"), *DeviceName.ToString()));
											})
								]

						]
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						[
							SAssignNew(KeyboardWidget, SHorizontalBox)

						]
				]
		];

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
								.OnPressed_Lambda([this, Key] {
								UMusicDeviceControllerSubsystem::TransmitNoteOnForDevice(DeviceName, Key + OctaveOffset * 12, 127);
									})
								.OnReleased_Lambda([this, Key] {
								UMusicDeviceControllerSubsystem::TransmitNoteOffForDevice(DeviceName, Key + OctaveOffset * 12, 0);
									})

								.ButtonStyle(FAppStyle::Get(), !bIsBlackKey ? "FlatButton.Dark" : "FlatButton")
						]

				];
		}
	}



};

TSharedRef<SWidget> MidiDevice::MetasoundWidget::CreateMidiVirtualKeyboardWidget(const Metasound::Editor::FCreateGraphNodeVisualizationWidgetParams& InParams)
{
	
	return SNew(SMidiVirtualKeyboard, InParams);

}

#endif