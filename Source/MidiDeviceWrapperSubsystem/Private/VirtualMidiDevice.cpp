// Copyright Amir Ben-Kiki and unDAW org.

#include "VirtualMidiDevice.h"
#include "MetasoundDataTypeRegistrationMacro.h"

DEFINE_METASOUND_DATA_TYPE(Metasound::FVirtualMidiDeviceAsset, "Virtual MIDI Device")

// ─── UVirtualMidiDevice ────────────────────────────────────────────────────────

UVirtualMidiDevice::UVirtualMidiDevice()
{
	MidiQueue = MakeShared<TSpscQueue<FMidiMsg>>();
}

TSharedPtr<Audio::IProxyData> UVirtualMidiDevice::CreateProxyData(const Audio::FProxyDataInitParams& InitParams)
{
	return MakeShared<FVirtualMidiDeviceProxy>(MidiQueue);
}

void UVirtualMidiDevice::SendNoteOn(int32 Channel, int32 NoteNumber, int32 Velocity)
{
	if (MidiQueue.IsValid())
	{
		MidiQueue->Enqueue(FMidiMsg::CreateNoteOn(
			FMath::Clamp(Channel, 0, 15),
			FMath::Clamp(NoteNumber, 0, 127),
			FMath::Clamp(Velocity, 0, 127)));
	}
}

void UVirtualMidiDevice::SendNoteOff(int32 Channel, int32 NoteNumber)
{
	if (MidiQueue.IsValid())
	{
		MidiQueue->Enqueue(FMidiMsg::CreateNoteOff(
			FMath::Clamp(Channel, 0, 15),
			FMath::Clamp(NoteNumber, 0, 127)));
	}
}

void UVirtualMidiDevice::SendControlChange(int32 Channel, int32 CCNumber, int32 Value)
{
	if (MidiQueue.IsValid())
	{
		MidiQueue->Enqueue(FMidiMsg::CreateControlChange(
			FMath::Clamp(Channel, 0, 15),
			FMath::Clamp(CCNumber, 0, 127),
			FMath::Clamp(Value, 0, 127)));
	}
}

void UVirtualMidiDevice::SendPitchBend(int32 Channel, int32 Value)
{
	if (MidiQueue.IsValid())
	{
		// Pitch bend is 14-bit: 0-16383, center at 8192
		const int32 Clamped = FMath::Clamp(Value, 0, 16383);
		const uint8 LSB = static_cast<uint8>(Clamped & 0x7F);
		const uint8 MSB = static_cast<uint8>((Clamped >> 7) & 0x7F);
		// FMidiMsg pitch bend: status byte 0xE0 | Channel, data1 = LSB, data2 = MSB
		MidiQueue->Enqueue(FMidiMsg(static_cast<uint8>(0xE0 | FMath::Clamp(Channel, 0, 15)), LSB, MSB));
	}
}
