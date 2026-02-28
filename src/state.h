#pragma once

#include "qt.h"
#include "platform_win32.h"

#define WINDOW_DEFAULT_WIDTH 500
#define WINDOW_DEFAULT_HEIGHT 200

struct GlobalState
{
	// Constants
	const char *BUTTON_STYLE_GREEN = "font-family: Georgia; font-size: 12pt; font-weight: bold; color: black; background-color: green;";
	const char *BUTTON_STYLE_RED = "font-family: Georgia; font-size: 12pt; font-weight: bold; color: black; background-color: #bF1121;";

	// UI
	QApplication *QtApp;
	QWidget *QtMainWindow;
	QPushButton *RecordButton;

	// Logic
	bool IsRecording;

	// Audio - platform-agnostic
	int CurrentAudioDeviceIndex;
	std::vector<AudioInputDeviceInfo> AudioInputDevices;

	// Inference Device 
	int CurrentInferenceDeviceIndex;
	QList<QString> InferenceDevices;

	// Whisper Wrapper
	int CurrentSTTModelIndex;
	QList<QString> STTModels;

	// SystemInfo SystemInfo;
	// CPUInfo CpuInfo;
};
