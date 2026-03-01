#pragma once

#include "state.h"

inline
void
update_audio_input_selection(GlobalState *AppState, int Index)
{
	AppState->CurrentAudioDeviceIndex = Index;
	#ifdef DEBUG
		printf("Selected audio input device index: %d\n", Index);
		if (Index >= 0 && Index < (int)AppState->AudioInputDevices.size())
		{
			AudioInputDeviceInfo Device = AppState->AudioInputDevices.at(Index);
			printf("Selected audio input device: %s\n", Device.Name.c_str());
		}
	#endif
}

inline
void
update_inference_device_selection(GlobalState *AppState, int Index)
{
	AppState->CurrentInferenceDeviceIndex = Index;
	#ifdef DEBUG
		printf("Selected inference device index: %d\n", Index);
	#endif
}

inline
void
update_stt_model_selection(GlobalState *AppState, int Index)
{
	AppState->CurrentSTTModelIndex = Index;
	#ifdef DEBUG
		printf("Selected STT model index: %d\n", Index);
	#endif
}

inline
void
toggle_recording(GlobalState *AppState)
{
	AppState->IsRecording = !AppState->IsRecording;
	
	if (AppState->IsRecording)
	{
		AppState->RecordButton->setStyleSheet(BUTTON_STYLE_RED);
		AppState->RecordButton->setText("Stop (Alt+F1)");
	}
	else
	{
		AppState->RecordButton->setStyleSheet(BUTTON_STYLE_GREEN);
		AppState->RecordButton->setText("Record (Alt+F1)");
	}
	#ifdef DEBUG
		qDebug() << "Recording toggled to:" << AppState->IsRecording;
	#endif

	// TODO(warren): start/stop audio recording
	// for speed, we want to be able to stream it to whisper if possible.
}

inline
void
toggle_stt_model_load(GlobalState *AppState)
{
	#ifdef DEBUG
		printf("[control] toggle_stt_model_load called\n");
	#endif

	if (is_whisper_model_loaded(&AppState->WhisperState))
	{
		#ifdef DEBUG
			printf("[control] Unloading STT model...\n");
		#endif
		unload_whisper_model(&AppState->WhisperState);

		AppState->LoadModelButton->setStyleSheet(BUTTON_STYLE_GREY);
		AppState->LoadModelButton->setText("Load Selected STT Model");

		#ifdef DEBUG
			printf("[control] STT model unloaded, button updated to grey\n");
		#endif
	}
	else
	{
		AppState->LoadModelButton->setEnabled(false);
		AppState->LoadModelButton->setText("Loading...");
		AppState->LoadModelButton->repaint();

		// Process pending events to show the "Loading..." text
		QApplication::processEvents();

		bool Success = load_whisper_model(&AppState->WhisperState, AppState->CurrentSTTModelIndex);
		AppState->LoadModelButton->setEnabled(true);
		if (Success)
		{
			AppState->LoadModelButton->setStyleSheet(BUTTON_STYLE_BLUE);
			AppState->LoadModelButton->setText("Unload STT Model");
			#ifdef DEBUG
				printf("[control] STT model loaded successfully, button updated to blue\n");
			#endif
		}
		else
		{
			AppState->LoadModelButton->setStyleSheet(BUTTON_STYLE_RED);
			AppState->LoadModelButton->setText("Failed to load Model.");
			#ifdef DEBUG
				printf("[control] ERROR: Failed to load STT model\n");
			#endif
		}

	}
}
