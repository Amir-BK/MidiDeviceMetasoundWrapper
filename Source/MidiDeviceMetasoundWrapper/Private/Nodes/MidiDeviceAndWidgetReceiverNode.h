// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MetasoundNodeInterface.h"

#include "HarmonixMetasound/Common.h"

namespace MidiDeviceMetasoundwrapper::MidiDeviceAndWidgetReceiverNode
{
	MIDIDEVICEMETASOUNDWRAPPER_API const Metasound::FNodeClassName& GetClassName();
	MIDIDEVICEMETASOUNDWRAPPER_API int32 GetCurrentMajorVersion();

	//namespace Inputs
	//{
	//	DECLARE_METASOUND_PARAM_ALIAS(Enable);
	//	DECLARE_METASOUND_PARAM_ALIAS(MidiStream);
	//	DECLARE_METASOUND_PARAM_EXTERN(MinTrackIndex);
	//	DECLARE_METASOUND_PARAM_EXTERN(MaxTrackIndex);
	//	DECLARE_METASOUND_PARAM_EXTERN(IncludeConductorTrack);
	//}

	//namespace Outputs
	//{
	//	DECLARE_METASOUND_PARAM_ALIAS(MidiStream);
	//}
}
