// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "MetasoundExecutableOperator.h"
#include "MetasoundFacade.h"
#include "MetasoundNodeInterface.h"
#include "MetasoundParamHelper.h"
#include "MetasoundSampleCounter.h"
#include "MetasoundStandardNodesCategories.h"
#include "MetasoundVertex.h"

#include "HarmonixMetasound/DataTypes/MidiStream.h"
#include "HarmonixMetasound/DataTypes/MusicTransport.h"
#include "MidiDeviceManager.h"

#include "MidiDeviceAndWidgetReceiverNode.h"
#include "MusicDeviceControllerSubsystem.h"


#define LOCTEXT_NAMESPACE "MidiDeviceMetasoundwrapper_MidiDeviceAndWidgetReceiverNode"

namespace MidiDeviceMetasoundwrapper::MidiStreamEventTrackMergeOp
{
	/**
	Adds notes from a TArray<TTupe<uint32, FMidiMsg>> to a FMidiStream

	*/
	class MIDIDEVICEMETASOUNDWRAPPER_API FMidiStreamEventTrackMerge
	{
	public:
		void Process(const HarmonixMetasound::FMidiStream& InStream, TArray<TTuple<int32, FMidiMsg>> InEvents, HarmonixMetasound::FMidiStream& OutStream) {
			const auto& BlockStartTick = 0;
			for (const auto& [tick, msg] : InEvents)
			{
				HarmonixMetasound::FMidiStreamEvent Event = HarmonixMetasound::FMidiStreamEvent(BlockStartTick, msg);
				//Event.MidiMessage = msg;
				Event.AuthoredMidiTick = tick;
				Event.TrackIndex = 1;
				OutStream.AddMidiEvent(Event);
			}
		}

	};


};

namespace MidiDeviceMetasoundwrapper::MidiDeviceAndWidgetReceiverNode
{
	using namespace Metasound;
	using namespace HarmonixMetasound;

	const FNodeClassName& GetClassName()
	{
		static FNodeClassName ClassName
		{
			"unDAW",
			"MidiStreamInput",
			""
		};
		return ClassName;
	}

	int32 GetCurrentMajorVersion()
	{
		return 1;
	}

	namespace Inputs
	{
		DEFINE_INPUT_METASOUND_PARAM(Enable, "Enable", "Enable");
		DEFINE_INPUT_METASOUND_PARAM(MidiStream, "MidiStream", "MidiStream");
		DEFINE_INPUT_METASOUND_PARAM(MinTrackIndex, "Track Index", "Track");
		DEFINE_INPUT_METASOUND_PARAM(MaxTrackIndex, "Channel Index", "Channel");
		DEFINE_INPUT_METASOUND_PARAM(MidiDeviceName, "Midi Device Name", "The name of the midi input device we want to receive with this node")
		//trigger, pitch, velocity for single note triggers
		DEFINE_INPUT_METASOUND_PARAM(NoteOn, "NoteOn", "Trigger for generating single notes without a midi device");
		DEFINE_INPUT_METASOUND_PARAM(NoteOff, "NoteOff", "Trigger for generating single notes without a midi device");
		DEFINE_INPUT_METASOUND_PARAM(Pitch, "Pitch", "the pitch to use when generating single notes using the trigger");
		DEFINE_INPUT_METASOUND_PARAM(Velocity, "Velocity", "the velocity to use when generating single notes using the trigger");
		//DEFINE_INPUT_METASOUND_PARAM(IncludeConductorTrack, "Include Conductor Track", "Enable to include the conductor track (AKA track 0)");
	}

	namespace Outputs
	{
		DEFINE_OUTPUT_METASOUND_PARAM(MidiStream, "MidiStream", "MidiStream");
	}

	class FMidiDeviceAndWidgetReceiverOperator final : public TExecutableOperator<FMidiDeviceAndWidgetReceiverOperator>
	{
	public:
		static const FNodeClassMetadata& GetNodeInfo()
		{
			auto InitNodeInfo = []() -> FNodeClassMetadata
				{
					FNodeClassMetadata Info;
					Info.ClassName = GetClassName();
					Info.MajorVersion = 1;
					Info.MinorVersion = 0;
					Info.DisplayName = INVTEXT("MIDI Stream Input Node");
					Info.Description = INVTEXT("merge midi inputs into an existing midi stream");
					Info.Author = TEXT("Amir Ben-Kiki");
					Info.PromptIfMissing = PluginNodeMissingPrompt;
					Info.DefaultInterface = GetVertexInterface();
					Info.CategoryHierarchy = { INVTEXT("unDAW"), NodeCategories::Music };
					return Info;
				};

			static const FNodeClassMetadata Info = InitNodeInfo();

			return Info;
		}

		static const FVertexInterface& GetVertexInterface()
		{
			static const FVertexInterface Interface(
				FInputVertexInterface(
					TInputDataVertex<bool>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::Enable), true),
					TInputDataVertex<FMidiStream>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MidiStream)),
					TInputDataVertex<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MinTrackIndex), 1),
					TInputDataVertex<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MaxTrackIndex), 0),
					TInputDataVertex<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MidiDeviceName)),
					TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::NoteOn)),
					TInputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::NoteOff)),
					TInputDataVertex<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::Pitch), 0),
					TInputDataVertex<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::Velocity), 0)

					//TInputDataVertex<bool>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::IncludeConductorTrack), false)
				),
				FOutputVertexInterface(
					TOutputDataVertex<FMidiStream>(METASOUND_GET_PARAM_NAME_AND_METADATA(Outputs::MidiStream))
				)
			);

			return Interface;
		}

		struct FInputs
		{
			FBoolReadRef Enabled;
			FMidiStreamReadRef MidiStream;
			FInt32ReadRef MinTrackIndex;
			FInt32ReadRef MaxTrackIndex;
			FStringReadRef MidiDeviceName;
			FTriggerReadRef NoteOn;
			FTriggerReadRef NoteOff;
			FInt32ReadRef Pitch;
			FInt32ReadRef Velocity;
			//FBoolReadRef IncludeConductorTrack;
		};

		struct FOutputs
		{
			FMidiStreamWriteRef MidiStream;
		};

		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			const FInputVertexInterfaceData& InputData = InParams.InputData;

			FInputs Inputs
			{
				InputData.GetOrCreateDefaultDataReadReference<bool>(Inputs::EnableName, InParams.OperatorSettings),
				InputData.GetOrConstructDataReadReference<FMidiStream>(Inputs::MidiStreamName),
				InputData.GetOrCreateDefaultDataReadReference<int32>(Inputs::MinTrackIndexName, InParams.OperatorSettings),
				InputData.GetOrCreateDefaultDataReadReference<int32>(Inputs::MaxTrackIndexName, InParams.OperatorSettings),
				InputData.GetOrCreateDefaultDataReadReference<FString>(Inputs::MidiDeviceNameName, InParams.OperatorSettings),
				InputData.GetOrCreateDefaultDataReadReference<FTrigger>(Inputs::NoteOnName, InParams.OperatorSettings),
				InputData.GetOrCreateDefaultDataReadReference<FTrigger>(Inputs::NoteOffName, InParams.OperatorSettings),
				InputData.GetOrCreateDefaultDataReadReference<int32>(Inputs::PitchName, InParams.OperatorSettings),
				InputData.GetOrCreateDefaultDataReadReference<int32>(Inputs::VelocityName, InParams.OperatorSettings)
				//InputData.GetOrCreateDefaultDataReadReference<bool>(Inputs::IncludeConductorTrackName, InParams.OperatorSettings)
			};

			FOutputs Outputs
			{
				FMidiStreamWriteRef::CreateNew()
			};

			return MakeUnique<FMidiDeviceAndWidgetReceiverOperator>(InParams, MoveTemp(Inputs), MoveTemp(Outputs));
		}

		FMidiDeviceAndWidgetReceiverOperator(const FBuildOperatorParams& InParams, FInputs&& InInputs, FOutputs&& InOutputs)
			: Inputs(MoveTemp(InInputs))
			, Outputs(MoveTemp(InOutputs))
		{
			Reset(InParams);
		}

		virtual void BindInputs(FInputVertexInterfaceData& InVertexData) override
		{
			InVertexData.BindReadVertex(Inputs::EnableName, Inputs.Enabled);
			InVertexData.BindReadVertex(Inputs::MidiStreamName, Inputs.MidiStream);
			InVertexData.BindReadVertex(Inputs::MinTrackIndexName, Inputs.MinTrackIndex);
			InVertexData.BindReadVertex(Inputs::MaxTrackIndexName, Inputs.MaxTrackIndex);
			InVertexData.BindReadVertex(Inputs::MidiDeviceNameName, Inputs.MidiDeviceName);
			InVertexData.BindReadVertex(Inputs::NoteOnName, Inputs.NoteOn);
			InVertexData.BindReadVertex(Inputs::NoteOffName, Inputs.NoteOff);
			InVertexData.BindReadVertex(Inputs::PitchName, Inputs.Pitch);
			InVertexData.BindReadVertex(Inputs::VelocityName, Inputs.Velocity);
			//InVertexData.BindReadVertex(Inputs::IncludeConductorTrackName, Inputs.IncludeConductorTrack);
		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InVertexData) override
		{
			InVertexData.BindReadVertex(Outputs::MidiStreamName, Outputs.MidiStream);
		}

		void Reset(const FResetParams&)
		{
			OnMidiInputDeviceChanged(*Inputs.MidiDeviceName.Get());
		}


		void OnMidiInputDeviceChanged(FString NewSelection)
		{
			//int32 DeviceID;
				//if we have a device controller, we need to remove the delegate
				if (MidiDeviceController != nullptr)
				{
					MidiDeviceController->OnMIDIRawEvent.Remove(RawEventDelegateHandle);
					MidiDeviceController = nullptr;
				}

			
				MidiDeviceController = UMusicDeviceControllerSubsystem::GetOrCreateMidiInputDeviceController(NewSelection);

				if (MidiDeviceController != nullptr)
				{
					RawEventDelegateHandle = MidiDeviceController->OnMIDIRawEvent.AddRaw(this, &FMidiDeviceAndWidgetReceiverOperator::OnReceiveRawMidiMessage);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Could not create MIDI device controller"));
				}

			
				//TickOffset = Inputs.MidiStream->GetClock()->GetCurrentMidiTick();
			


		}

		void OnReceiveRawMidiMessage(UMIDIDeviceInputController* MIDIDeviceController, int32 Timestamp, int32 Type, int32 Channel, int32 MessageData1, int32 MessageData2)
		{
			const EMIDIEventType MIDIEventType = static_cast<EMIDIEventType>(Type);

			switch (MIDIEventType)
			{
			case EMIDIEventType::NoteOn:
				PendingMessages.Add(TTuple<int32, FMidiMsg>(Timestamp, FMidiMsg::CreateNoteOn(Channel, MessageData1, MessageData2)));
				break;
			case EMIDIEventType::NoteOff:
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
			//construct new midi message from data
			//we'll probably need a switch here... I hope not
			//FMidiMsg NewMidiMsg(Type, MessageData1, MessageData2);
			//PendingMessages.Add(TTuple<int32, FMidiMsg>(Timestamp, NewMidiMsg));

		}

		//destructor
		virtual ~FMidiDeviceAndWidgetReceiverOperator()
		{
			//UE_LOG(LogTemp, Log, TEXT("MidiDeviceAndWidgetReceiverOperator Destructor"));
			if (MidiDeviceController != nullptr)
			{
				MidiDeviceController->OnMIDIRawEvent.Remove(RawEventDelegateHandle);
				//UMIDIDeviceManager::ShutDownAllMIDIDevices();
				//MidiDeviceController->
				//UMIDIDeviceManager::MidiIn
				//MidiDeviceController->ShutdownDevice();
				MidiDeviceController = nullptr;
			}
		}


		void Execute()
		{
			//Filter.SetFilterValues(*Inputs.MinTrackIndex, *Inputs.MaxTrackIndex, false);

			Outputs.MidiStream->PrepareBlock();




			if (*Inputs.Enabled)
			{
				//stream current tick?
				//int32 CurrentTick = Inputs.MidiStream->GetClock()->GetCurrentMidiTick();
				//Inputs.MidiStream->Add
				if (Inputs.NoteOn->IsTriggeredInBlock())
				{
					//add a message to the stream
					PendingMessages.Add(TTuple<int32, FMidiMsg>(0, FMidiMsg::CreateNoteOn(0, *Inputs.Pitch, *Inputs.Velocity)));

				}

				if (Inputs.NoteOff->IsTriggeredInBlock())
				{
					//add a message to the stream
					PendingMessages.Add(TTuple<int32, FMidiMsg>(0, FMidiMsg::CreateNoteOff(0, *Inputs.Pitch)));
				}


				
				MergeOp.Process(*Inputs.MidiStream, PendingMessages, *Outputs.MidiStream);
				PendingMessages.Empty();
				//Filter.Process(*Inputs.MidiStream, *Outputs.MidiStream);
			}

		}
	private:
		FInputs Inputs;
		FOutputs Outputs;
		UMIDIDeviceInputController* MidiDeviceController = nullptr;
		FDelegateHandle RawEventDelegateHandle;
		int32 TickOffset = 0; //in theory we can start when the stream tick is different from the device tick, we'll see
		TArray<TTuple<int32, FMidiMsg>> PendingMessages;
		MidiDeviceMetasoundwrapper::MidiStreamEventTrackMergeOp::FMidiStreamEventTrackMerge MergeOp;
		//unDAWMetasounds::TrackIsolatorOP::FMidiTrackIsolator Filter;
	};

	class FunDAWMidiInputNode final : public FNodeFacade
	{
	public:
		explicit FunDAWMidiInputNode(const FNodeInitData& InInitData)
			: FNodeFacade(InInitData.InstanceName, InInitData.InstanceID, TFacadeOperatorClass<FMidiDeviceAndWidgetReceiverOperator>())
		{

		}
		virtual ~FunDAWMidiInputNode() override = default;

		FMidiDeviceAndWidgetReceiverOperator* MyOperator = nullptr;
	};

	METASOUND_REGISTER_NODE(FunDAWMidiInputNode)
}

#undef LOCTEXT_NAMESPACE // "HarmonixMetaSound"