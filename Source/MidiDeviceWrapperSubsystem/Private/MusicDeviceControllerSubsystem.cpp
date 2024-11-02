// Fill out your copyright notice in the Description page of Project Settings.


#include "MusicDeviceControllerSubsystem.h"

namespace MusicDeviceRegistry
{

	static TMap<FString, UMIDIDeviceInputController*> MidiDeviceControllers;

}

UMIDIDeviceInputController* UMusicDeviceControllerSubsystem::GetOrCreateMidiInputDeviceController(const FString& MidiDeviceName)
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
		
		UMIDIDeviceInputController* NewController = UMIDIDeviceManager::CreateMIDIDeviceInputController(NewDeviceToCreate->DeviceID, 64);
	
		MusicDeviceRegistry::MidiDeviceControllers.Add(MidiDeviceName, NewController);
		return NewController;
	}
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

