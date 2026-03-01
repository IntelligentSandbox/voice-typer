#pragma once

#include "qt.h"
#include "platform_win32.h"
#include "whisper_wrapper.h"

#include <atomic>
#include <mutex>
#include <vector>
#include <thread>

#define WINDOW_DEFAULT_WIDTH 500
#define WINDOW_DEFAULT_HEIGHT 500

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
	QPushButton *StreamButton;
	QPushButton *LoadModelButton;

	// Logic
	bool IsRecording;
	bool IsStreaming;

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

	// Audio capture pipeline
	std::atomic<bool> CaptureRunning;
	std::thread CaptureThread;
	std::mutex AudioBufferMutex;
	std::vector<float> AudioAccumBuffer;

	// Inference threading
	int WhisperThreadCount;

	// Text injection target: captured when a pipeline starts, null if our own window was focused
	HWND FocusedWindow;

	// SystemInfo SystemInfo;
	// CPUInfo CpuInfo;
};
