// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#ifdef WITH_MIDI_DEVICE
#include "MidiDeviceManager.h"
#include "MIDIDeviceController.h"
#endif
#include "MusicDeviceControllerSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_SixParams(FOnDAWRawMidiEvent, FName /* MusicDeviceController */, int32 /* Timestamp */, int32 /* Type */, int32 /* Channel */, int32 /* MessageData1 */, int32 /* MessageData2 */);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SevenParams(FOnMIDIInputEvent, class UMusicDeviceInput*, MUsicDeviceController, int32, Timestamp, EMIDIEventType, EventType, int32, Channel, int32, ControlID, int32, Velocity, int32, RawEventType);


//cross platform safe aggregator for midi devices and other inputs
UCLASS()
class MIDIDEVICEWRAPPERSUBSYSTEM_API UMusicDeviceInput : public UMIDIDeviceInputController
{
	GENERATED_BODY()

	~UMusicDeviceInput()
	{
		//unbind the delegate and call super destructor
		OnMIDIEvent.Clear();

	}

public:

	
	/** Register with this to find out about incoming MIDI events from this device */
	UPROPERTY(BlueprintAssignable, Category = "MIDI Device Controller")
	FOnMIDIInputEvent OnMIDIEvent;
	

#ifdef WITH_MIDI_DEVICE
	UMIDIDeviceInputController* UnderlyingMidiDeviceController;
#endif

};

/**
 * This subsystem solves some annoyances with the unreal portmidi implementation, namely it lets us keep a mididevice controller alive
 * and not have to worry about it remaining in a 'used' state when we try to invoke it again
 * but this subsystem is also a good place to put other music device controllers, like OSC, MIDI, etc.
 * perhaps we will allow allow injecting midi and custom events on these endpoints
 */
UCLASS()
class MIDIDEVICEWRAPPERSUBSYSTEM_API UMusicDeviceControllerSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	
	//as shuttindg down midi devices doesn't really work in unreal, we will keep a registry of all the midi devices we have created
	//the user probably doesn't need to destroy them at all, they can remain in memory, otherwise they're just not gonna be accessible
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MusicDeviceControllerSubsystem")
	static UMusicDeviceInput* GetOrCreateMidiInputDeviceController(const FString& MidiDeviceName);

	//UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MusicDeviceControllerSubsystem")
	//static UMusicDeviceInput* GetOrCreateMusicDeviceInput(const FString& MusicDeviceName) {};

	UFUNCTION(BlueprintCallable, Category = "MusicDeviceControllerSubsystem")
	static void TransmitNoteOnForDevice(const FName& DeviceName, int32 note, int32 velocity);

	UFUNCTION(BlueprintCallable, Category = "MusicDeviceControllerSubsystem")
	static void TransmitNoteOffForDevice(const FName& DeviceName, int32 note, int32 velocity);

	static UMusicDeviceInput* SetupMidiDeviceInput(const FString& MidiDeviceName);

};
