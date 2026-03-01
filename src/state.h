#pragma once

#include "qt.h"
#include "platform_win32.h"
#include "whisper_wrapper.h"

#define WINDOW_DEFAULT_WIDTH 750
#define WINDOW_DEFAULT_HEIGHT 300

#define BUTTON_STYLE_GREEN "font-family: Georgia; font-size: 12pt; font-weight: bold; color: black; background-color: green;"
#define BUTTON_STYLE_RED "font-family: Georgia; font-size: 12pt; font-weight: bold; color: black; background-color: #bF1121;"
#define BUTTON_STYLE_GREY "font-family: Georgia; font-size: 12pt; font-weight: bold; color: black; background-color: #808080;"
#define BUTTON_STYLE_BLUE "font-family: Georgia; font-size: 12pt; font-weight: bold; color: white; background-color: #2196F3;"

struct GlobalState
{
	// UI
	QApplication *QtApp;
	QWidget *QtMainWindow;
	QPushButton *RecordButton;
	QPushButton *LoadModelButton;

	// Logic
	bool IsRecording;

	// Audio - platform-agnostic
	int CurrentAudioDeviceIndex;
	std::vector<AudioInputDeviceInfo> AudioInputDevices;

	// Inference Device 
	int CurrentInferenceDeviceIndex;
	std::vector<std::string> InferenceDevices;

	// Whisper Wrapper
	int CurrentSTTModelIndex;
	std::vector<std::string> STTModels;
	WhisperModelState WhisperState;

	// SystemInfo SystemInfo;
	// CPUInfo CpuInfo;
};
