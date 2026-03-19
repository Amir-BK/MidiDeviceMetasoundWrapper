// Copyright Amir Ben-Kiki and unDAW org.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "IAudioProxyInitializer.h"
#include "HarmonixMidi/MidiMsg.h"
#include "MetasoundDataReferenceMacro.h"
#include "Containers/SpscQueue.h"
#include "VirtualMidiDevice.generated.h"

// Forward declare
class UVirtualMidiDevice;

/**
 * Audio proxy that carries the shared MIDI event queue to the audio thread.
 * Does NOT hold a raw pointer to the UObject — only the lock-free queue.
 */
class MIDIDEVICEWRAPPERSUBSYSTEM_API FVirtualMidiDeviceProxy : public Audio::TProxyData<FVirtualMidiDeviceProxy>
{
public:
	IMPL_AUDIOPROXY_CLASS(FVirtualMidiDeviceProxy);

	explicit FVirtualMidiDeviceProxy(const TSharedPtr<TSpscQueue<FMidiMsg>>& InQueue)
		: MidiQueue(InQueue)
	{
	}

	FVirtualMidiDeviceProxy(const FVirtualMidiDeviceProxy& Other) = default;

	TSharedPtr<TSpscQueue<FMidiMsg>> MidiQueue;
};

/**
 * A virtual MIDI device that can be driven from Blueprints (game thread)
 * and consumed by a MetaSound node (audio thread) via a lock-free queue.
 */
UCLASS(BlueprintType, Category = "MIDI")
class MIDIDEVICEWRAPPERSUBSYSTEM_API UVirtualMidiDevice : public UObject, public IAudioProxyDataFactory
{
	GENERATED_BODY()

public:
	UVirtualMidiDevice();

	// IAudioProxyDataFactory
	virtual TSharedPtr<Audio::IProxyData> CreateProxyData(const Audio::FProxyDataInitParams& InitParams) override;

	/** Send a Note On event. Channel 0-15, NoteNumber 0-127, Velocity 0-127. */
	UFUNCTION(BlueprintCallable, Category = "Virtual MIDI Device")
	void SendNoteOn(int32 Channel, int32 NoteNumber, int32 Velocity);

	/** Send a Note Off event. Channel 0-15, NoteNumber 0-127. */
	UFUNCTION(BlueprintCallable, Category = "Virtual MIDI Device")
	void SendNoteOff(int32 Channel, int32 NoteNumber);

	/** Send a Control Change (CC) event. Channel 0-15, CCNumber 0-127, Value 0-127. */
	UFUNCTION(BlueprintCallable, Category = "Virtual MIDI Device")
	void SendControlChange(int32 Channel, int32 CCNumber, int32 Value);

	/** Send a Pitch Bend event. Channel 0-15, Value 0-16383 (8192 = center). */
	UFUNCTION(BlueprintCallable, Category = "Virtual MIDI Device")
	void SendPitchBend(int32 Channel, int32 Value);

	/** Get the shared queue — used internally by the proxy. */
	TSharedPtr<TSpscQueue<FMidiMsg>> GetMidiQueue() const { return MidiQueue; }

private:
	/** Lock-free single-producer (game thread) single-consumer (audio thread) queue. */
	TSharedPtr<TSpscQueue<FMidiMsg>> MidiQueue;
};

// ─── MetaSound datatype wrapper ────────────────────────────────────────────────

namespace Metasound
{
	/** MetaSound-compatible wrapper for the VirtualMidiDevice proxy. */
	class MIDIDEVICEWRAPPERSUBSYSTEM_API FVirtualMidiDeviceAsset
	{
	public:
		FVirtualMidiDeviceAsset() = default;
		FVirtualMidiDeviceAsset(const FVirtualMidiDeviceAsset&) = default;
		FVirtualMidiDeviceAsset& operator=(const FVirtualMidiDeviceAsset&) = default;

		FVirtualMidiDeviceAsset(const TSharedPtr<Audio::IProxyData>& InInitData)
		{
			Proxy = StaticCastSharedPtr<FVirtualMidiDeviceProxy>(InInitData);
		}

		bool IsInitialized() const { return Proxy.IsValid(); }
		const FVirtualMidiDeviceProxy* GetProxy() const { return Proxy.Get(); }

	private:
		TSharedPtr<FVirtualMidiDeviceProxy, ESPMode::ThreadSafe> Proxy;
	};

	DECLARE_METASOUND_DATA_REFERENCE_TYPES(FVirtualMidiDeviceAsset, MIDIDEVICEWRAPPERSUBSYSTEM_API, FVirtualMidiDeviceAssetTypeInfo, FVirtualMidiDeviceAssetReadRef, FVirtualMidiDeviceAssetWriteRef)
}
