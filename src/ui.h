#pragma once

#include "qt.h"
#include "state.h"
#include "control.h"

void init_main_window(GlobalState *AppState)
{
	QWidget *MainWindow = AppState->QtMainWindow;
	std::vector<AudioInputDeviceInfo> *AudioInputDevices = &(AppState->AudioInputDevices);

	// ---- Left Column ----
	QGridLayout *GridLayout = new QGridLayout(MainWindow);
	
	// Record Button
	QPushButton *RecordButton = new QPushButton(MainWindow);
	RecordButton->setStyleSheet(AppState->BUTTON_STYLE_GREEN);
	RecordButton->setText("Record (Alt+F1)");
	RecordButton->setMinimumHeight(60);
	GridLayout->addWidget(RecordButton, 0, 0);
	
	AppState->RecordButton = RecordButton;
	
	QLabel *AudioSelectLabel = new QLabel("Audio Input", MainWindow);
	GridLayout->addWidget(AudioSelectLabel, 1, 0);

	QComboBox *AudioInputSelect = new QComboBox(MainWindow);
	for (size_t i = 0; i < AudioInputDevices->size(); i++)
	{
		AudioInputDeviceInfo Device = AudioInputDevices->at(i);
		QString Description = QString::fromUtf8(Device.Name.c_str());
		AudioInputSelect->addItem(Description);
		if (AppState->CurrentAudioDeviceIndex == (int)i)
		{
			AudioInputSelect->setCurrentIndex((int)i);
		}
	}
	AudioInputSelect->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	GridLayout->addWidget(AudioInputSelect, 2, 0);

    QLabel *ModelSelectLabel = new QLabel("STT Model", MainWindow);
    GridLayout->addWidget(ModelSelectLabel, 3, 0);
    QComboBox *ModelSelect = new QComboBox(MainWindow);
    for (int i = 0; i < AppState->STTModels.size(); i++)
    {
        QString Description = AppState->STTModels.at(i);
        ModelSelect->addItem(Description);
        if (AppState->CurrentSTTModelIndex == i)
        {
            ModelSelect->setCurrentIndex(i);
        }
    }
    GridLayout->addWidget(ModelSelect, 4, 0);

    QLabel *InferenceDeviceSelectLabel = new QLabel("Inference Device", MainWindow);
    GridLayout->addWidget(InferenceDeviceSelectLabel, 5, 0);
    QComboBox *InferenceDeviceSelect = new QComboBox(MainWindow);
    for (int i = 0; i < AppState->InferenceDevices.size(); i++)
    {
        QString Description = AppState->InferenceDevices.at(i);
        InferenceDeviceSelect->addItem(Description);
        if (AppState->CurrentInferenceDeviceIndex == i)
        {
            InferenceDeviceSelect->setCurrentIndex(i);
        }
    }

    GridLayout->addWidget(InferenceDeviceSelect, 6, 0);

    // ---- Right Column ----
    QLabel *InferenceDeviceLabel = new QLabel("Transcription Stats: ", MainWindow);
    GridLayout->addWidget(InferenceDeviceLabel, 0, 1);

    // --- "Event Listeners" ----
    QObject::connect(AudioInputSelect, &QComboBox::currentIndexChanged, [AppState](int index)
    {
        update_audio_input_selection(AppState, index);
    });

    QObject::connect(InferenceDeviceSelect, &QComboBox::currentIndexChanged, [AppState](int index)
    {
        update_inference_device_selection(AppState, index);
    });

    QObject::connect(RecordButton, &QPushButton::clicked, [AppState]()
    {
        toggle_recording(AppState);
    });
}

void init_ui(GlobalState *AppState)
{
    QFont Font("Georgia");
    Font.setPointSize(12);
    Font.setWeight(QFont::Bold);
    AppState->QtApp->setFont(Font);

    AppState->QtMainWindow->setWindowTitle("Voice Typer");
    AppState->QtMainWindow->resize(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);

    init_main_window(AppState);
}
