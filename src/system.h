#include "state.h"

#include <thread>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#include "platform_win32.h"
#endif

std::atomic<bool> HotkeyThreadRunning = false;

void hotkey_thread_func(GlobalState *AppState)
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

void start_hotkey_listener(GlobalState *AppState)
{
    HotkeyThreadRunning = true;
    std::thread(hotkey_thread_func, AppState).detach();
}

void stop_hotkey_listener()
{
    HotkeyThreadRunning = false;
}

void query_audio_input_devices(GlobalState *AppState)
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

void query_inference_devices(GlobalState *AppState)
{
    AppState->InferenceDevices = QList<QString>();
    AppState->InferenceDevices.append("CPU");
    AppState->CurrentInferenceDeviceIndex = 0;

    AppState->InferenceDevices.append("GPU");
}

void query_available_stt_models(GlobalState *AppState)
{
    AppState->STTModels = QList<QString>();

    AppState->STTModels.append("Whisper.cpp (GGML) - base.en");
    AppState->CurrentSTTModelIndex = 0;
}
