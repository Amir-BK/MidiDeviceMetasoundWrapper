# MidiDeviceMetasoundWrapper
Real time midi device input into metasounds, and a few other QOL improvements


The node in question, just insert the correct name of the device you'd like to use, use the standard MidiDevice functions to query the system for available devices and pass input controller string to the node.

![image](https://github.com/user-attachments/assets/db4d8d52-9fc0-4c4f-a825-94346a5c40be)

The plugin also includes a simple subsystem that keeps track of opened midi device and keeps the midi device usable for as long as the game session is running, this is a work around to a bug in the Epic port midi implementation that makes devices unusable after they've been shut down once, which is problematic for a metasound use case in which you may want to open the same device every time the node is used (or receive the same midi device info in multiple nodes). 

# Metasound Nodes 
![image](https://github.com/user-attachments/assets/c0646ddc-997e-4259-86d8-68fd3b2a7cec)


# Midi Keyboard Metasound Widget (Unreal 5.5+ only) 
UE5.5 adds the ability to register custom widgets for metasound nodes, so it was pretty straight forward to connect the midi input node to a widget, the integration is a bit hacky right now and it only works when the node is actually connected to a valid midi input controller, but still, a pretty fun result (that also demonstrates the usage of the Music Device Controller subsystem) 

https://github.com/user-attachments/assets/13547b9c-4ca7-431e-a2db-be645f8d7c8e



## Usage:
1. The node needs to get the name of the midi input controller to work, you can get this name by using the standard unreal Midi Device Manager methods (Find All Midi Devices, etc), pass this name as a string input to the metasound and connect that to the node if needed, but do not use the 'Create Midi Device' method exposed by the unreal midi device manager, the creation of the midi device is handled by the subsystem and it will not work if a device has already been created. 
2. Unreal won't let two clients register to the same midi input device, the Music Device Controller subsystem in this plugin is a workaround for that, if you need to access the midi input with another object alongside the metasound input node use the provided function to get the Midi Input Controller instead of the default unreal midi device ones, this keeps a pointer to the midi device accessible both the metasound as well as to any other potential client who needs it - ![image](https://github.com/user-attachments/assets/cf3995a0-29da-4980-9e00-f65c625758bf),
   through the returned 'Music Input Device Controller' you can access all midi event delegates exposed by unreal as per usual.
