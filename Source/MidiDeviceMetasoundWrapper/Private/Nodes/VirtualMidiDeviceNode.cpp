// Copyright Amir Ben-Kiki and unDAW org.

#include "MetasoundNodeInterface.h"

#include "HarmonixMetasound/Common.h"
#include "CoreMinimal.h"
#include "MetasoundExecutableOperator.h"
#include "MetasoundFacade.h"
#include "MetasoundNodeInterface.h"
#include "MetasoundParamHelper.h"
#include "MetasoundSampleCounter.h"
#include "MetasoundStandardNodesCategories.h"
#include "MetasoundVertex.h"

#include "HarmonixMetasound/DataTypes/MidiStream.h"
#include "VirtualMidiDevice.h"

#define LOCTEXT_NAMESPACE "MidiDeviceMetasoundwrapper_VirtualMidiDeviceNode"

namespace MidiDeviceMetasoundwrapper::VirtualMidiDeviceNode
{
	using namespace Metasound;
	using namespace HarmonixMetasound;

	namespace Inputs
	{
		DEFINE_INPUT_METASOUND_PARAM(Enable, "Enable", "Enable the node");
		DEFINE_INPUT_METASOUND_PARAM(VirtualMidiDevice, "Virtual MIDI Device", "The Virtual MIDI Device UObject to consume events from");
		DEFINE_INPUT_METASOUND_PARAM(MidiStream, "MidiStream", "Optional input MidiStream to merge with virtual device events");
	}

	namespace Outputs
	{
		DEFINE_OUTPUT_METASOUND_PARAM(MidiStream, "MidiStream", "Output MidiStream containing events from the virtual device");
	}

	class FVirtualMidiDeviceOperator final : public TExecutableOperator<FVirtualMidiDeviceOperator>
	{
	public:
		static const FNodeClassMetadata& GetNodeInfo()
		{
			auto InitNodeInfo = []() -> FNodeClassMetadata
			{
				FNodeClassMetadata Info;
				Info.ClassName = { "unDAW", "VirtualMidiDevice", "" };
				Info.MajorVersion = 1;
				Info.MinorVersion = 0;
				Info.DisplayName = INVTEXT("Virtual MIDI Device");
				Info.Description = INVTEXT("Outputs MIDI events sent from a Blueprint-driven Virtual MIDI Device UObject");
				Info.Author = TEXT("Amir Ben-Kiki");
				Info.PromptIfMissing = PluginNodeMissingPrompt;
				Info.DefaultInterface = GetVertexInterface();
				Info.CategoryHierarchy = { NodeCategories::Music };
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
					TInputDataVertex<FVirtualMidiDeviceAsset>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::VirtualMidiDevice)),
					TInputDataVertex<FMidiStream>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MidiStream))
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
			FVirtualMidiDeviceAssetReadRef VirtualMidiDevice;
			FMidiStreamReadRef MidiStream;
		};

		struct FOutputs
		{
			FMidiStreamWriteRef MidiStream;
		};

		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			const FInputVertexInterfaceData& InputData = InParams.InputData;

			FInputs Inputs{
				InputData.GetOrCreateDefaultDataReadReference<bool>(Inputs::EnableName, InParams.OperatorSettings),
				InputData.GetOrCreateDefaultDataReadReference<FVirtualMidiDeviceAsset>(Inputs::VirtualMidiDeviceName, InParams.OperatorSettings),
				InputData.GetOrCreateDefaultDataReadReference<FMidiStream>(Inputs::MidiStreamName, InParams.OperatorSettings)
			};

			FOutputs Outputs{
				FMidiStreamWriteRef::CreateNew()
			};

			return MakeUnique<FVirtualMidiDeviceOperator>(InParams, MoveTemp(Inputs), MoveTemp(Outputs));
		}

		FVirtualMidiDeviceOperator(const FBuildOperatorParams& InParams, FInputs&& InInputs, FOutputs&& InOutputs)
			: Inputs(MoveTemp(InInputs))
			, Outputs(MoveTemp(InOutputs))
		{
			Reset(InParams);
		}

		virtual void BindInputs(FInputVertexInterfaceData& InVertexData) override
		{
			InVertexData.BindReadVertex(Inputs::EnableName, Inputs.Enabled);
			InVertexData.BindReadVertex(Inputs::VirtualMidiDeviceName, Inputs.VirtualMidiDevice);
			InVertexData.BindReadVertex(Inputs::MidiStreamName, Inputs.MidiStream);
		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InVertexData) override
		{
			InVertexData.BindReadVertex(Outputs::MidiStreamName, Outputs.MidiStream);
		}

		void Reset(const FResetParams&)
		{
		}

		void Execute()
		{
			Outputs.MidiStream->PrepareBlock();

			if (!*Inputs.Enabled)
			{
				return;
			}

			// Merge any events from the pass-through input stream
			for (const FMidiStreamEvent& Event : Inputs.MidiStream->GetEventsInBlock())
			{
				Outputs.MidiStream->AddMidiEvent(Event);
			}

			// Dequeue events from the virtual MIDI device's lock-free queue
			if (Inputs.VirtualMidiDevice->IsInitialized())
			{
				const FVirtualMidiDeviceProxy* Proxy = Inputs.VirtualMidiDevice->GetProxy();
				if (Proxy && Proxy->MidiQueue.IsValid())
				{
					FMidiMsg Msg;
					while (Proxy->MidiQueue->Dequeue(Msg))
					{
						const int32 BlockStartTick = 0;
						FMidiStreamEvent Event(BlockStartTick, Msg);
						Event.TrackIndex = 1;
						Event.AuthoredMidiTick = 0;
						Outputs.MidiStream->AddMidiEvent(Event);
					}
				}
			}
		}

	private:
		FInputs Inputs;
		FOutputs Outputs;
	};

	class FVirtualMidiDeviceNode final : public FNodeFacade
	{
	public:
		explicit FVirtualMidiDeviceNode(const FNodeInitData& InInitData)
			: FNodeFacade(InInitData.InstanceName, InInitData.InstanceID, TFacadeOperatorClass<FVirtualMidiDeviceOperator>())
		{
		}

		virtual ~FVirtualMidiDeviceNode() override = default;
	};

	METASOUND_REGISTER_NODE(FVirtualMidiDeviceNode)
}

#undef LOCTEXT_NAMESPACE
