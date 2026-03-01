#include "state.h"

#include <thread>
#include <atomic>

	#ifdef _WIN32
		#include <windows.h>
		#include "platform_win32.h"
	#endif

std::atomic<bool> HotkeyThreadRunning = false;

inline
void
hotkey_thread_func(GlobalState *AppState)
{
	bool ToggleKeyWasDown = false;

	while (HotkeyThreadRunning)
	{
		#ifdef _WIN32
			bool ToggleKeyIsDown = GetAsyncKeyState(VK_F1) & 0x8000 && GetAsyncKeyState(VK_MENU) & 0x8000;

			if (ToggleKeyIsDown && !ToggleKeyWasDown)
			{
				toggle_recording(AppState);
			}

			ToggleKeyWasDown = ToggleKeyIsDown;
			Sleep(1);
		#endif
	}
}

inline
void
start_hotkey_listener(GlobalState *AppState)
{
	HotkeyThreadRunning = true;
	std::thread(hotkey_thread_func, AppState).detach();
}

inline
void
stop_hotkey_listener()
{
	HotkeyThreadRunning = false;
}

inline
void
query_audio_input_devices(GlobalState *AppState)
{
	AppState->CurrentAudioDeviceIndex = -1;
	
	std::vector<AudioInputDeviceInfo> NativeDevices = query_audio_input_devices_native();
	
	AppState->AudioInputDevices = NativeDevices;
	
	if (AppState->AudioInputDevices.size() > 0)
	{
		AppState->CurrentAudioDeviceIndex = 0;
	}
	
	#ifdef DEBUG
		qDebug() << "Audio input devices found:" << AppState->AudioInputDevices.size();
		for (const auto &Info : AppState->AudioInputDevices)
		{
			qDebug() << "  Device:" << Info.Name.c_str() << "(index" << Info.Index << ")";
		}
	#endif
}

inline
void
query_inference_devices(GlobalState *AppState)
{
	AppState->InferenceDevices.clear();
	AppState->InferenceDevices.push_back("CPU");
	AppState->CurrentInferenceDeviceIndex = 0;

	// TODO(warren): In the future, enable GPU support for dedicated cards like NVIDIA
	AppState->InferenceDevices.push_back("GPU");
}

// TODO(warren): Assuming all the model files are present. 
// Maybe want to actually look to see if they are present on disk, but if they
// aren't, do we want to just download them automatically from huggingface?
inline
void
query_available_stt_models(GlobalState *AppState)
{
	AppState->STTModels.clear();

	AppState->STTModels.push_back("Whisper tiny.en (75 MB)");
	AppState->STTModels.push_back("Whisper base.en (142 MB)");
	AppState->STTModels.push_back("Whisper small.en (466 MB)");
	AppState->STTModels.push_back("Whisper medium.en (1.5 GB)");
	AppState->STTModels.push_back("Whisper large-v3-turbo (1.5 GB)");

	AppState->CurrentSTTModelIndex = WHISPER_MODEL_BASE_EN;
}
