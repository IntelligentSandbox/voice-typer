#pragma once

#include <vector>
#include <string>

#include <windows.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <propkey.h>
#include <functiondiscoverykeys.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "propsys.lib")

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

#define AUDIO_CAPTURE_SAMPLE_RATE    16000
#define AUDIO_CAPTURE_CHANNELS       1
#define AUDIO_CAPTURE_BITS_PER_SAMPLE 16
#define AUDIO_CAPTURE_BUFFER_MS      100
#define AUDIO_CAPTURE_BUFFER_COUNT   8

inline
std::vector<AudioInputDeviceInfo>
query_audio_input_devices_native()
{
	std::vector<AudioInputDeviceInfo> Devices;

	UINT NumDevices = waveInGetNumDevs();

	WAVEINCAPS2W Caps = {};
	MMRESULT Result;

	IMMDeviceEnumerator* pEnumerator = nullptr;
	IMMDeviceCollection* pCollection = nullptr;
	BOOL hasWASAPI = FALSE;

	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	hasWASAPI = (CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator) == S_OK);

	if (hasWASAPI && pEnumerator)
	{
		pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pCollection);
	}

	if (hasWASAPI && pCollection)
	{
		UINT deviceCount = 0;
		pCollection->GetCount(&deviceCount);

		for (UINT i = 0; i < NumDevices && i < deviceCount; i++)
		{
			Result = waveInGetDevCapsW(i, (LPWAVEINCAPSW)&Caps, sizeof(Caps));
			if (Result != MMSYSERR_NOERROR) continue;

            AudioInputDeviceInfo Info;
            Info.Index = i;
            Info.Id = std::to_string(i);

            IMMDevice* pDevice = nullptr;
            IPropertyStore* pProps = nullptr;
            PROPVARIANT varName;

            PropVariantInit(&varName);

            BOOL gotName = FALSE;
            if (pCollection->Item(i, &pDevice) == S_OK)
            {
                if (pDevice->OpenPropertyStore(STGM_READ, &pProps) == S_OK)
                {
                    if (pProps->GetValue(PKEY_Device_FriendlyName, &varName) == S_OK)
                    {
                        if (varName.vt == VT_LPWSTR && varName.pwszVal)
                        {
                            char fullName[MAX_AUDIO_DEVICE_NAME_LENGTH];
                            WideCharToMultiByte(CP_UTF8, 0, varName.pwszVal, -1, fullName, MAX_AUDIO_DEVICE_NAME_LENGTH, NULL, NULL);
                            Info.Name = fullName;
                            gotName = TRUE;
                        }
                        PropVariantClear(&varName);
                    }
                    pProps->Release();
                }
                pDevice->Release();
            }

            if (!gotName)
            {
                char DeviceName[MAX_AUDIO_DEVICE_NAME_LENGTH];
                int converted = WideCharToMultiByte(CP_UTF8, 0, Caps.szPname, -1, DeviceName, MAX_AUDIO_DEVICE_NAME_LENGTH, NULL, NULL);
                if (converted == 0)
                {
                    DeviceName[0] = 0;
                }
                Info.Name = DeviceName;
            }

            Info.IsDefault = false;
            Devices.push_back(Info);
		}

		pCollection->Release();
	}
	else
	{
		for (UINT i = 0; i < NumDevices; i++)
		{
			Result = waveInGetDevCapsW(i, (LPWAVEINCAPSW)&Caps, sizeof(Caps));
			if (Result != MMSYSERR_NOERROR) continue;

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

	if (pEnumerator)
	{
		pEnumerator->Release();
	}

	CoUninitialize();

	return Devices;
}

inline
void
inject_text_to_window(HWND TargetWindow, const char *Utf8Text)
{
	if (!TargetWindow || !Utf8Text || Utf8Text[0] == '\0') return;

	int WideLen = MultiByteToWideChar(CP_UTF8, 0, Utf8Text, -1, nullptr, 0);
	if (WideLen <= 1) return;

	std::wstring Wide(WideLen - 1, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, Utf8Text, -1, &Wide[0], WideLen);

	std::vector<INPUT> Inputs;
	Inputs.reserve(Wide.size() * 2);

	for (wchar_t Wc : Wide)
	{
		INPUT KeyDown = {};
		KeyDown.type           = INPUT_KEYBOARD;
		KeyDown.ki.wVk         = 0;
		KeyDown.ki.wScan       = Wc;
		KeyDown.ki.dwFlags     = KEYEVENTF_UNICODE;
		KeyDown.ki.time        = 0;
		KeyDown.ki.dwExtraInfo = 0;

		INPUT KeyUp = KeyDown;
		KeyUp.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;

		Inputs.push_back(KeyDown);
		Inputs.push_back(KeyUp);
	}

	SetForegroundWindow(TargetWindow);
	SendInput((UINT)Inputs.size(), Inputs.data(), sizeof(INPUT));
}

inline
int
query_logical_processor_count()
{
	SYSTEM_INFO SysInfo = {};
	GetSystemInfo(&SysInfo);
	return (int)SysInfo.dwNumberOfProcessors;
}

inline
const char*
audio_device_get_name(AudioInputDeviceInfo *Info)
{
	if (Info)
	{
		return Info->Name.c_str();
	}
	return "";
}

inline
int
audio_device_get_index(AudioInputDeviceInfo *Info)
{
	if (Info)
	{
		return Info->Index;
	}
	return -1;
}
