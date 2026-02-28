#pragma once

#include <vector>
#include <string>

#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#define MAX_AUDIO_DEVICE_NAME_LENGTH 512

struct AudioInputDeviceInfo
{
	int Index;
	std::string Id;
	std::string Name;
	bool IsDefault;
};

struct AudioCaptureDevice
{
	HWAVEIN Handle;
	int DeviceIndex;
	bool IsCapturing;
};

inline std::vector<AudioInputDeviceInfo> query_audio_input_devices_native()
{
	std::vector<AudioInputDeviceInfo> Devices;

	UINT NumDevices = waveInGetNumDevs();
	
	WAVEINCAPS2W Caps = {};
	MMRESULT Result;
	
	for (UINT i = 0; i < NumDevices; i++)
	{
		Result = waveInGetDevCapsW(i, (LPWAVEINCAPSW)&Caps, sizeof(Caps));
		if (Result == MMSYSERR_NOERROR)
		{
			AudioInputDeviceInfo Info;
			Info.Index = i;
			Info.Id = std::to_string(i);
			char DeviceName[MAX_AUDIO_DEVICE_NAME_LENGTH];
			int converted = WideCharToMultiByte(CP_UTF8, 0, Caps.szPname, -1, DeviceName, MAX_AUDIO_DEVICE_NAME_LENGTH, NULL, NULL);
			if (converted == 0)
			{
				DeviceName[0] = 0;
			}
			Info.Name = DeviceName;
			Info.IsDefault = false;
			Devices.push_back(Info);
		}
	}

	return Devices;
}

inline const char* audio_device_get_name(AudioInputDeviceInfo* Info)
{
	if (Info) return Info->Name.c_str();
	return "";
}

inline int audio_device_get_index(AudioInputDeviceInfo* Info)
{
	if (Info) return Info->Index;
	return -1;
}
