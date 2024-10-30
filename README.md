# MidiDeviceMetasoundWrapper
Real time midi device input into metasounds, and a few other QOL improvements

The node in question, just insert the correct name of the device you'd like to use, use the standard MidiDevice functions to query the system for available devices and pass input controller string to the node.
![image](https://github.com/user-attachments/assets/db4d8d52-9fc0-4c4f-a825-94346a5c40be)

The plugin also includes a simple subsystem that keeps track of opened midi device and keeps the midi device usable for as long as the game session is running, this is a work around to a bug in the Epic port midi implementation that makes devices unusable after they've been shut down once, which is problematic for a metasound use case in which you may want to open the same device every time the node is used (or receive the same midi device info in multiple nodes). 

The plugin is very raw and is mostly a proof of concept at this point, I found that latency is kind of detrimental if you want to play anything in time with an actual midi stream, unreal has no notion of recording latency compensation. 
