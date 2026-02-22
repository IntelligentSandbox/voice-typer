#include "state.h"

#include <thread>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#endif

std::atomic<bool> HotkeyThreadRunning = false;

void hotkey_thread_func(GlobalState *AppState)
{
    bool ToggleKeyWasDown = false;

    while (HotkeyThreadRunning)
    {
        // TODO(warren): Might want to allow the user to change the keyboard shortcut, if some apps listen for Alt+F1 already and intercept it or something.
        
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
    AppState->AudioInputDevices = QMediaDevices::audioInputs();

    QString DefaultAudioDeviceDescription = QMediaDevices::defaultAudioInput().description();
    for (int i = 0; i < AppState->AudioInputDevices.size(); i++)
    {
        QString Description = AppState->AudioInputDevices.at(i).description();
        if (DefaultAudioDeviceDescription == Description)
        {
            AppState->CurrentAudioDeviceIndex = i;
            break;
        }
    }

    if (AppState->CurrentAudioDeviceIndex == -1 && AppState->AudioInputDevices.size() > 0)
    {
        AppState->CurrentAudioDeviceIndex = 0;
    }
    // TODO(warren): if CurrentAudioDeviceIndex is still -1, assume no audio input devices. Then what?
}

void query_inference_devices(GlobalState *AppState)
{
    // TODO(warren): When we have the CPU name, use that instead.
    AppState->InferenceDevices = QList<QString>();
    AppState->InferenceDevices.append("CPU");
    AppState->CurrentInferenceDeviceIndex = 0;

    // TODO(warren): Same thing for GPUs...
    // And maybe in the future if auto detect a good gpu, default to that.
    AppState->InferenceDevices.append("GPU");
}

void query_available_stt_models(GlobalState *AppState)
{
    AppState->STTModels = QList<QString>();

    // TODO(warren): Validate model file is present before setting in future.
    AppState->STTModels.append("Whisper.cpp (GGML) - base.en");
    AppState->CurrentSTTModelIndex = 0;
}
