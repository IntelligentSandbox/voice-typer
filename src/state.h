#pragma once

#include "qt.h"

struct GlobalState
{
    // Constants
    const char *BUTTON_STYLE_GREEN = "font-family: Georgia; font-size: 12pt; font-weight: bold; color: black; background-color: green;";
    const char *BUTTON_STYLE_RED = "font-family: Georgia; font-size: 12pt; font-weight: bold; color: black; background-color: #bF1121;";
    const int WINDOW_DEFAULT_WIDTH = 500;
    const int WINDOW_DEFAULT_HEIGHT = 200;

    // UI
    QApplication *QtApp;
    QWidget *QtMainWindow;
    QPushButton *RecordButton;

    // Logic
    bool IsRecording;

    // Audio
    int CurrentAudioDeviceIndex;
    QList<QAudioDevice> AudioInputDevices;

    // Inference Device 
    int CurrentInferenceDeviceIndex;
    QList<QString> InferenceDevices;

    // Whisper Wrapper
    int CurrentSTTModelIndex;
    QList<QString> STTModels;

    // SystemInfo SystemInfo;
    // CPUInfo CpuInfo;
};
