#pragma once

#include "qt.h"
#include "state.h"
#include "control.h"

inline
void
init_main_window(GlobalState *AppState)
{
	QWidget *MainWindow = AppState->QtMainWindow;
	std::vector<AudioInputDeviceInfo> *AudioInputDevices = &(AppState->AudioInputDevices);

	// ---- Left Column ----
	QGridLayout *GridLayout = new QGridLayout(MainWindow);
	
	// Record Button
	QPushButton *RecordButton = new QPushButton(MainWindow);
	RecordButton->setStyleSheet(BUTTON_STYLE_GREEN);
	RecordButton->setText("Record (Alt+F1)");
	RecordButton->setMinimumHeight(60);
	GridLayout->addWidget(RecordButton, 0, 0);

	AppState->RecordButton = RecordButton;

	// Stream Button
	QPushButton *StreamButton = new QPushButton(MainWindow);
	StreamButton->setStyleSheet(BUTTON_STYLE_GREEN);
	StreamButton->setText("Start Streaming (Alt+F2)");
	StreamButton->setMinimumHeight(60);
	GridLayout->addWidget(StreamButton, 1, 0);

	AppState->StreamButton = StreamButton;

	QLabel *AudioSelectLabel = new QLabel("Audio Input", MainWindow);
	GridLayout->addWidget(AudioSelectLabel, 2, 0);

	QComboBox *AudioInputSelect = new QComboBox(MainWindow);
	for (size_t i = 0; i < AudioInputDevices->size(); i++)
	{
		AudioInputDeviceInfo Device = AudioInputDevices->at(i);
		QString Description = QString::fromStdString(Device.Name);
		AudioInputSelect->addItem(Description);
		if (AppState->CurrentAudioDeviceIndex == (int)i)
		{
			AudioInputSelect->setCurrentIndex(i);
		}
		#ifdef DEBUG
			qDebug() << "Initializing device input in select box.";
			qDebug() << "Device.Name: " << Device.Name;
		#endif
	}
	AudioInputSelect->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	GridLayout->addWidget(AudioInputSelect, 3, 0);

	QLabel *ModelSelectLabel = new QLabel("STT Model", MainWindow);
	GridLayout->addWidget(ModelSelectLabel, 4, 0);
	QComboBox *STTModelSelect = new QComboBox(MainWindow);
	for (size_t i = 0; i < AppState->STTModels.size(); i++)
	{
		QString Description = QString::fromStdString(AppState->STTModels.at(i));
		STTModelSelect->addItem(Description);
		if (AppState->CurrentSTTModelIndex == (int)i)
		{
			STTModelSelect->setCurrentIndex(i);
		}
	}
	GridLayout->addWidget(STTModelSelect, 5, 0);

	QLabel *InferenceDeviceSelectLabel = new QLabel("Inference Device", MainWindow);
	GridLayout->addWidget(InferenceDeviceSelectLabel, 6, 0);
	QComboBox *InferenceDeviceSelect = new QComboBox(MainWindow);
	for (size_t i = 0; i < AppState->InferenceDevices.size(); i++)
	{
		QString Description = QString::fromStdString(AppState->InferenceDevices.at(i));
		InferenceDeviceSelect->addItem(Description);
		if (AppState->CurrentInferenceDeviceIndex == (int)i)
		{
			InferenceDeviceSelect->setCurrentIndex(i);
		}
	}

	GridLayout->addWidget(InferenceDeviceSelect, 7, 0);

	// ---- Right Column ----
	// QLabel *InferenceDeviceLabel = new QLabel("Transcription Stats: ", MainWindow);
	// GridLayout->addWidget(InferenceDeviceLabel, 0, 1);

	QPushButton *LoadModelButton = new QPushButton(MainWindow);
	LoadModelButton->setStyleSheet(BUTTON_STYLE_GREY);
	LoadModelButton->setText("Load Selected STT Model");
	LoadModelButton->setMinimumHeight(60);
	GridLayout->addWidget(LoadModelButton, 0, 1);

	AppState->LoadModelButton = LoadModelButton;

	#ifdef DEBUG
		printf("[ui] Load Model button initialized (grey = not loaded)\n");
	#endif

	// --- "Event Listeners" ----
	QObject::connect(AudioInputSelect, &QComboBox::currentIndexChanged, [AppState](int index)
	{
		update_audio_input_selection(AppState, index);
	});

	QObject::connect(InferenceDeviceSelect, &QComboBox::currentIndexChanged, [AppState](int index)
	{
		update_inference_device_selection(AppState, index);
	});

	QObject::connect(STTModelSelect, &QComboBox::currentIndexChanged, [AppState](int index)
	{
		update_stt_model_selection(AppState, index);
	});

	QObject::connect(RecordButton, &QPushButton::clicked, [AppState]()
	{
		toggle_recording(AppState);
	});

	QObject::connect(StreamButton, &QPushButton::clicked, [AppState]()
	{
		toggle_streaming(AppState);
	});

	QObject::connect(LoadModelButton, &QPushButton::clicked, [AppState]()
	{
		toggle_stt_model_load(AppState);
	});
}

inline
void
init_ui(GlobalState *AppState)
{
	QFont Font("Georgia");
	Font.setPointSize(12);
	Font.setWeight(QFont::Bold);
	AppState->QtApp->setFont(Font);

	AppState->QtMainWindow->setWindowTitle("Voice Typer");
	AppState->QtMainWindow->resize(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);

	init_main_window(AppState);
}
