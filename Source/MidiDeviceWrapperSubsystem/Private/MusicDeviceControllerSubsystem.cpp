// Fill out your copyright notice in the Description page of Project Settings.


#include "MusicDeviceControllerSubsystem.h"

namespace MusicDeviceRegistry
{

	static TMap<FString, UMusicDeviceInput*> MidiDeviceControllers;

}

UMusicDeviceInput* UMusicDeviceControllerSubsystem::GetOrCreateMidiInputDeviceController(const FString& MidiDeviceName)
{
	//if the registry already contains device name, return it from registry, otherwise create a new one and add it to the registry
	if (MusicDeviceRegistry::MidiDeviceControllers.Contains(MidiDeviceName))
	{
		auto FoundDevice = MusicDeviceRegistry::MidiDeviceControllers[MidiDeviceName];
		if (FoundDevice != nullptr)
		{
			return FoundDevice;
			
		}
		UE_LOG(LogTemp, Error, TEXT("Found MIDI device in registry, but it was null!"));
	
	}
	
	//if the device is not in the registry, or the registry contained a nullptr for the device, create a new one
	
	{
		TArray<FMIDIDeviceInfo> InputDevices, OutputDevices;
		UMIDIDeviceManager::FindAllMIDIDeviceInfo(InputDevices, OutputDevices);

		//Find input device with the device name
		FMIDIDeviceInfo* NewDeviceToCreate = InputDevices.FindByPredicate([MidiDeviceName](const FMIDIDeviceInfo& DeviceInfo)
			{
				return DeviceInfo.DeviceName.Contains(MidiDeviceName);
			});

		if (NewDeviceToCreate == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Could not find MIDI device with name %s"), *MidiDeviceName);
			return nullptr;
		}
		
		UMusicDeviceInput* NewController = UMusicDeviceControllerSubsystem::SetupMidiDeviceInput(MidiDeviceName);
	
		
		return NewController;
	}
}

UMusicDeviceInput* UMusicDeviceControllerSubsystem::SetupMidiDeviceInput(const FString& MidiDeviceName)
{
	TArray<FMIDIDeviceInfo> InputDevices, OutputDevices;
	UMIDIDeviceManager::FindAllMIDIDeviceInfo(InputDevices, OutputDevices);

	//Find input device with the device name
	FMIDIDeviceInfo* NewDeviceToCreate = InputDevices.FindByPredicate([MidiDeviceName](const FMIDIDeviceInfo& DeviceInfo)
		{
			return DeviceInfo.DeviceName.Contains(MidiDeviceName);
		});

	if (NewDeviceToCreate == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not find MIDI device with name %s"), *MidiDeviceName);
		return nullptr;
	}

	UMusicDeviceInput* NewController = NewObject<UMusicDeviceInput>();

	bool bStartedSuccessfully = false;
	NewController->StartupDevice(NewDeviceToCreate->DeviceID, 64, /* Out */ bStartedSuccessfully);

	if (!bStartedSuccessfully)
	{
		// Kill it
		NewController->MarkAsGarbage();
		NewController = nullptr;

		//UE_LOG(LogMIDIDevice, Warning, TEXT("Create MIDI Device Controller wasn't able to create the controller successfully. Returning a null reference."));
	}
	else {
		NewController->OnMIDIRawEvent.AddLambda([NewController](UMIDIDeviceInputController* Controller, int32 Timestamp, int32 Type, int32 Channel, int32 MessageData1, int32 MessageData2)
			{
				EMIDIEventType EventType = static_cast<EMIDIEventType>(Type);
				NewController->OnMIDIEvent.Broadcast(NewController, Timestamp, EventType, Channel, MessageData1, MessageData2, Type);
			});

		MusicDeviceRegistry::MidiDeviceControllers.Add(MidiDeviceName, NewController);
	}


	return NewController;
}

void UMusicDeviceControllerSubsystem::TransmitNoteOnForDevice(const FName& DeviceName, int32 note, int32 velocity)
{
	if (MusicDeviceRegistry::MidiDeviceControllers.Contains(DeviceName.ToString()))
	{
		auto FoundDevice = MusicDeviceRegistry::MidiDeviceControllers[DeviceName.ToString()];
		if (FoundDevice != nullptr)
		{
			FoundDevice->OnMIDIRawEvent.Broadcast(FoundDevice, 0, 9, 0, note, velocity);
		}
	}
}

void UMusicDeviceControllerSubsystem::TransmitNoteOffForDevice(const FName& DeviceName, int32 note, int32 velocity)
{
	if (MusicDeviceRegistry::MidiDeviceControllers.Contains(DeviceName.ToString()))
	{
		auto FoundDevice = MusicDeviceRegistry::MidiDeviceControllers[DeviceName.ToString()];
		if (FoundDevice != nullptr)
		{
			FoundDevice->OnMIDIRawEvent.Broadcast(FoundDevice, 0, 8, 0, note, velocity);
		}
	}
}

