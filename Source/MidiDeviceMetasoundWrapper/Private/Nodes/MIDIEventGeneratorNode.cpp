// MIDIEventGeneratorNode.cpp
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
#include "HarmonixMetasound/DataTypes/MusicTransport.h"

#define LOCTEXT_NAMESPACE "MidiDeviceMetasoundwrapper_MIDIEventGeneratorNode"

namespace MidiDeviceMetasoundwrapper::MIDIEventGeneratorNode
{
    using namespace Metasound;
    using namespace HarmonixMetasound;

    namespace Inputs
    {
        DEFINE_INPUT_METASOUND_PARAM(Enable, "Enable", "Enable");
        DEFINE_INPUT_METASOUND_PARAM(Value, "Value", "Value to monitor for changes");
        DEFINE_INPUT_METASOUND_PARAM(MIDIValue, "MIDI Value", "MIDI CC number (0-127)");
    }

    namespace Outputs
    {
        DEFINE_OUTPUT_METASOUND_PARAM(MidiStream, "MidiStream", "MidiStream");
    }

    class FMIDIEventGeneratorOperator final : public TExecutableOperator<FMIDIEventGeneratorOperator>
    {
    public:
        static const FNodeClassMetadata& GetNodeInfo()
        {
            auto InitNodeInfo = []() -> FNodeClassMetadata
            {
                FNodeClassMetadata Info;
                Info.ClassName = { "unDAW", "FloatToMidiCC", "" };
                Info.MajorVersion = 1;
                Info.MinorVersion = 0;
                Info.DisplayName = INVTEXT("Float To MIDI CC");
                Info.Description = INVTEXT("Converts float value changes to MIDI CC messages");
                Info.Author = TEXT("Your Name");
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
                    TInputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::Value), 0.0f),
                    TInputDataVertex<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MIDIValue), 0)
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
            FFloatReadRef Value;
            FInt32ReadRef MIDIValue;
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
                InputData.GetOrCreateDefaultDataReadReference<float>(Inputs::ValueName, InParams.OperatorSettings),
                InputData.GetOrCreateDefaultDataReadReference<int32>(Inputs::MIDIValueName, InParams.OperatorSettings)
            };

            FOutputs Outputs{
                FMidiStreamWriteRef::CreateNew()
            };

            return MakeUnique<FMIDIEventGeneratorOperator>(InParams, MoveTemp(Inputs), MoveTemp(Outputs));
        }

        FMIDIEventGeneratorOperator(const FBuildOperatorParams& InParams, FInputs&& InInputs, FOutputs&& InOutputs)
            : Inputs(MoveTemp(InInputs))
            , Outputs(MoveTemp(InOutputs))
            , PreviousValue(*InInputs.Value)
        {
            Reset(InParams);
        }

        virtual void BindInputs(FInputVertexInterfaceData& InVertexData) override
        {
            InVertexData.BindReadVertex(Inputs::EnableName, Inputs.Enabled);
            InVertexData.BindReadVertex(Inputs::ValueName, Inputs.Value);
            InVertexData.BindReadVertex(Inputs::MIDIValueName, Inputs.MIDIValue);
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

            if (*Inputs.Enabled)
            {
                float CurrentValue = *Inputs.Value;
                int32 MIDIValue = FMath::Clamp(*Inputs.MIDIValue, 0, 127);

                if (CurrentValue != PreviousValue)
                {
                    // Create MIDI CC message
                    FMidiMsg MidiMessage = FMidiMsg::CreateControlChange(
                        0, // Channel
                        MIDIValue,
                        static_cast<uint8>(FMath::Clamp(CurrentValue * 127.0f, 0.0f, 127.0f))
                    );

                    // Add event to stream
                    const auto& BlockStartTick = 0;
                    HarmonixMetasound::FMidiStreamEvent Event = HarmonixMetasound::FMidiStreamEvent(BlockStartTick, MidiMessage);
                    Event.TrackIndex = 1;
                    Outputs.MidiStream->AddMidiEvent(Event);

                    PreviousValue = CurrentValue;
                }
            }
        }

    private:
        FInputs Inputs;
        FOutputs Outputs;
        float PreviousValue;
    };

    class FMIDIEventGeneratorNode final : public FNodeFacade
    {
    public:
        explicit FMIDIEventGeneratorNode(const FNodeInitData& InInitData)
            : FNodeFacade(InInitData.InstanceName, InInitData.InstanceID, TFacadeOperatorClass<FMIDIEventGeneratorOperator>())
        {
        }

        virtual ~FMIDIEventGeneratorNode() override = default;
    };

    METASOUND_REGISTER_NODE(FMIDIEventGeneratorNode)
}

#undef LOCTEXT_NAMESPACE