#pragma once

#include "state.h"

void update_audio_input_selection(GlobalState *AppState, int index)
{
	AppState->CurrentAudioDeviceIndex = index;
#ifdef DEBUG
	qDebug() << "Selected audio input device index:" << index;
	if (index >= 0 && index < (int)AppState->AudioInputDevices.size())
	{
		AudioInputDeviceInfo Device = AppState->AudioInputDevices.at(index);
		qDebug() << "Selected audio input device:" << Device.Name.c_str();
	}
#endif
}

void update_inference_device_selection(GlobalState *AppState, int index)
{
    AppState->CurrentInferenceDeviceIndex = index;
#ifdef DEBUG
    qDebug() << "Selected inference device index: " << index;
#endif
}

void toggle_recording(GlobalState *AppState)
{
    AppState->IsRecording = !AppState->IsRecording;
    
    if (AppState->IsRecording)
    {
        AppState->RecordButton->setStyleSheet(AppState->BUTTON_STYLE_RED);
        AppState->RecordButton->setText("Stop (Alt+F1)");
    }
    else
    {
        AppState->RecordButton->setStyleSheet(AppState->BUTTON_STYLE_GREEN);
        AppState->RecordButton->setText("Record (Alt+F1)");
    }
#ifdef DEBUG
    qDebug() << "Recording toggled to:" << AppState->IsRecording;
#endif

    // TODO(warren): start/stop audio recording
    // for speed, we want to be able to stream it to whisper if possible.
}
