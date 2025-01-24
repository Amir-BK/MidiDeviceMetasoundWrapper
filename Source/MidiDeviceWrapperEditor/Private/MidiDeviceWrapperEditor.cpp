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
#include "HarmonixMetasound/DataTypes/MidiStream.h"
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
	UMIDIDeviceInputController* MidiDeviceController = nullptr;
	TArray<TTuple<int32, FMidiMsg>> PendingMessages;
	TMap<int32, TSharedPtr<SButton>> KeyButtons;
	TMap<int32, TSharedPtr<SBorder>> KeyBorders;

	//set up styles for regular, black key and active key
	FButtonStyle RegularKeyStyle;
	FButtonStyle AccidentalKeyStyle;
	FButtonStyle ActiveKeyStyle;
	FButtonStyle ActiveAccidentalKeyStyle;


	FName DeviceName;
	int OctaveOffset = 0;

	void Tick(const FGeometry& AllottedGeometry,
		const double InCurrentTime,
		const float InDeltaTime
	) override
	{

		const auto& ThePin = Node->FindPin(TEXT("Midi Device Name"));

		const FName NewDeviceName = FName(ThePin->GetDefaultAsString());
		if (NewDeviceName != DeviceName)
		{
			DeviceName = NewDeviceName;
				
			//get the device and bind the delegate
			MidiDeviceController = UMusicDeviceControllerSubsystem::GetOrCreateMidiInputDeviceController(DeviceName.ToString());
			if (MidiDeviceController)
			{
				MidiDeviceController->OnMIDIRawEvent.AddSP(this, &SMidiVirtualKeyboard::OnReceiveRawMidiMessage);
			}
		}

		//process pending messages, if any, we only update the color of the borders
		for (const auto& [tick, msg] : PendingMessages)
		{
			const int Key = msg.GetStdData1();
			const bool bIsBlackKey = (Key % 12 == 1) || (Key % 12 == 3) || (Key % 12 == 6) || (Key % 12 == 8) || (Key % 12 == 10);
			if (msg.IsNoteOn())
			{

				KeyButtons[Key]->SetButtonStyle(!bIsBlackKey ? &ActiveKeyStyle : &ActiveAccidentalKeyStyle);

				
			}
			else if (msg.IsNoteOff())
			{
			
				KeyButtons[Key]->SetButtonStyle(!bIsBlackKey ? &AccidentalKeyStyle : &RegularKeyStyle);


			}
		}
		
		PendingMessages.Empty();
	}

	void OnReceiveRawMidiMessage(UMIDIDeviceInputController* MIDIDeviceController, int32 Timestamp, int32 Type, int32 Channel, int32 MessageData1, int32 MessageData2)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Midi Raw Event: %d %d %d %d %d"), Timestamp, Type, Channel, MessageData1, MessageData2);
		const EMIDIEventType MIDIEventType = static_cast<EMIDIEventType>(Type);

		switch (MIDIEventType)
		{
		case EMIDIEventType::NoteOn:
			UE_LOG(LogTemp, Warning, TEXT("Note On"));
			PendingMessages.Add(TTuple<int32, FMidiMsg>(Timestamp, FMidiMsg::CreateNoteOn(Channel, MessageData1, MessageData2)));
			break;
		case EMIDIEventType::NoteOff:
			UE_LOG(LogTemp, Warning, TEXT("Note Off"));
			PendingMessages.Add(TTuple<int32, FMidiMsg>(Timestamp, FMidiMsg::CreateNoteOff(Channel, MessageData1)));
			break;
		case EMIDIEventType::ControlChange:
			UE_LOG(LogTemp, Warning, TEXT("Control Change"));
			PendingMessages.Add(TTuple<int32, FMidiMsg>(Timestamp, FMidiMsg::CreateControlChange(Channel, MessageData1, MessageData2)));
			break;
		case EMIDIEventType::ProgramChange:
			UE_LOG(LogTemp, Warning, TEXT("Program Change"));
			//PendingMessages.Add(TTuple<int32, FMidiMsg>(Timestamp, FMidiMsg::CreateProgramChange(Channel, MessageData1)));
			break;
		case EMIDIEventType::PitchBend:
			UE_LOG(LogTemp, Warning, TEXT("Pitch Bend"));
			//PendingMessages.Add(TTuple<int32, FMidiMsg>(Timestamp, FMidiMsg(uint8(0b1110), MessageData1, MessageData2)));
			break;
		case EMIDIEventType::NoteAfterTouch:
			UE_LOG(LogTemp, Warning, TEXT("Aftertouch"));
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("Unknown Event"));
			break;
		}

	}

	void Construct(const FArguments& InArgs, const Metasound::Editor::FCreateGraphNodeVisualizationWidgetParams& InParams)
	{
		//set up styles for regular, black key and active key
		auto ActiveAccidentalNormal = FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton.Warning").Normal;
		//dark orange
		ActiveAccidentalNormal.TintColor = FLinearColor(0.5f, 0.2f, 0.0f, 1.0f);

        RegularKeyStyle = FButtonStyle()
        .SetNormal(FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton").Normal)
        .SetHovered(FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton").Hovered)
        .SetPressed(FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton").Pressed)
        .SetDisabled(FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton").Disabled);

		AccidentalKeyStyle = FButtonStyle()
			.SetNormal(FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton.Dark").Normal)
			.SetHovered(FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton.Dark").Hovered)
			.SetPressed(ActiveAccidentalNormal)
			.SetDisabled(FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton.Dark").Disabled);

		ActiveKeyStyle = FButtonStyle()
			.SetNormal(FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton.Warning").Normal)
			.SetHovered(FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton.Warning").Hovered)
			.SetPressed(FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton.Warning").Pressed)
			.SetDisabled(FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton.Warning").Disabled);



		//needs to be slightly darker
		ActiveAccidentalKeyStyle = FButtonStyle()
			.SetNormal(ActiveAccidentalNormal)
			.SetHovered(FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton.Warning").Hovered)
			.SetPressed(FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton.Warning").Pressed)
			.SetDisabled(FAppStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton.Warning").Disabled);



		
		const int NumKeys = 36;
		const int FirstKey = 48;
		Node = InParams.MetaSoundNode;


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

			TSharedPtr<SButton> NewButton;
			TSharedPtr<SBorder> ContentBorder;

			KeyboardWidget->AddSlot()
				.AutoWidth()
				[
					SNew(SBox)
						.WidthOverride(10)
						[
							SAssignNew(NewButton, SButton)
								//.Text(FText::FromString(FString::Printf(TEXT("%d"), Key)))
								.OnPressed_Lambda([this, Key] {
								UMusicDeviceControllerSubsystem::TransmitNoteOnForDevice(DeviceName, Key + OctaveOffset * 12, 127);
									})
								.OnReleased_Lambda([this, Key] {
								UMusicDeviceControllerSubsystem::TransmitNoteOffForDevice(DeviceName, Key + OctaveOffset * 12, 0);
									})

								.ButtonStyle(FAppStyle::Get(), !bIsBlackKey ? "FlatButton.Dark" : "FlatButton")

						]

				];

			KeyButtons.Add(Key, NewButton);
			//KeyBorders.Add(Key, ContentBorder);
		}
	}



};

TSharedRef<SWidget> MidiDevice::MetasoundWidget::CreateMidiVirtualKeyboardWidget(const Metasound::Editor::FCreateGraphNodeVisualizationWidgetParams& InParams)
{
	
	return SNew(SMidiVirtualKeyboard, InParams);

}

#endif