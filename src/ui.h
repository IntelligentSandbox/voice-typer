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

	QGridLayout *GridLayout = new QGridLayout(MainWindow);
	int Row = 0;

	// Record Button
	QPushButton *RecordButton = new QPushButton(MainWindow);
	RecordButton->setStyleSheet(BUTTON_STYLE_GREEN);
	RecordButton->setText("Record (Alt+F1)");
	RecordButton->setMinimumHeight(60);
	GridLayout->addWidget(RecordButton, Row++, 0);
	AppState->RecordButton = RecordButton;

	// Stream Button
	QPushButton *StreamButton = new QPushButton(MainWindow);
	StreamButton->setStyleSheet(BUTTON_STYLE_GREEN);
	StreamButton->setText("Start Streaming (Alt+F2)");
	StreamButton->setMinimumHeight(60);
	GridLayout->addWidget(StreamButton, Row++, 0);
	AppState->StreamButton = StreamButton;

	// Audio Input
	GridLayout->addWidget(new QLabel("Audio Input", MainWindow), Row++, 0);
	QComboBox *AudioInputSelect = new QComboBox(MainWindow);
	for (size_t i = 0; i < AudioInputDevices->size(); i++)
	{
		AudioInputDeviceInfo Device = AudioInputDevices->at(i);
		AudioInputSelect->addItem(QString::fromStdString(Device.Name));
		if (AppState->CurrentAudioDeviceIndex == (int)i)
			AudioInputSelect->setCurrentIndex(i);
		#ifdef DEBUG
			qDebug() << "Initializing device input in select box.";
			qDebug() << "Device.Name: " << Device.Name;
		#endif
	}
	AudioInputSelect->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	GridLayout->addWidget(AudioInputSelect, Row++, 0);

	// STT Model
	GridLayout->addWidget(new QLabel("STT Model", MainWindow), Row++, 0);
	QComboBox *STTModelSelect = new QComboBox(MainWindow);
	for (size_t i = 0; i < AppState->STTModels.size(); i++)
	{
		STTModelSelect->addItem(QString::fromStdString(AppState->STTModels.at(i)));
		if (AppState->CurrentSTTModelIndex == (int)i)
			STTModelSelect->setCurrentIndex(i);
	}
	GridLayout->addWidget(STTModelSelect, Row++, 0);

	// Load Model Button
	QPushButton *LoadModelButton = new QPushButton(MainWindow);
	LoadModelButton->setStyleSheet(BUTTON_STYLE_GREY);
	LoadModelButton->setText("Load Selected STT Model");
	LoadModelButton->setMinimumHeight(60);
	GridLayout->addWidget(LoadModelButton, Row++, 0);
	AppState->LoadModelButton = LoadModelButton;

	#ifdef DEBUG
		printf("[ui] Load Model button initialized (grey = not loaded)\n");
	#endif

	// Inference Device
	GridLayout->addWidget(new QLabel("Inference Device", MainWindow), Row++, 0);
	QComboBox *InferenceDeviceSelect = new QComboBox(MainWindow);
	for (size_t i = 0; i < AppState->InferenceDevices.size(); i++)
	{
		InferenceDeviceSelect->addItem(QString::fromStdString(AppState->InferenceDevices.at(i)));
		if (AppState->CurrentInferenceDeviceIndex == (int)i)
			InferenceDeviceSelect->setCurrentIndex(i);
	}
	GridLayout->addWidget(InferenceDeviceSelect, Row++, 0);

	// CPU Cores
	GridLayout->addWidget(new QLabel("CPU Cores for Inference", MainWindow), Row++, 0);
	QSpinBox *ThreadCountSpinner = new QSpinBox(MainWindow);
	ThreadCountSpinner->setMinimum(1);
	ThreadCountSpinner->setMaximum(query_logical_processor_count());
	ThreadCountSpinner->setValue(AppState->WhisperThreadCount);
	GridLayout->addWidget(ThreadCountSpinner, Row++, 0);

	// --- Connections ---
	QObject::connect(RecordButton, &QPushButton::clicked, [AppState]()
	{
		toggle_recording(AppState);
	});

	QObject::connect(StreamButton, &QPushButton::clicked, [AppState]()
	{
		toggle_streaming(AppState);
	});

	QObject::connect(AudioInputSelect, &QComboBox::currentIndexChanged, [AppState](int index)
	{
		update_audio_input_selection(AppState, index);
	});

	QObject::connect(STTModelSelect, &QComboBox::currentIndexChanged, [AppState](int index)
	{
		update_stt_model_selection(AppState, index);
	});

	QObject::connect(LoadModelButton, &QPushButton::clicked, [AppState]()
	{
		toggle_stt_model_load(AppState);
	});

	QObject::connect(InferenceDeviceSelect, &QComboBox::currentIndexChanged, [AppState](int index)
	{
		update_inference_device_selection(AppState, index);
	});

	QObject::connect(ThreadCountSpinner, &QSpinBox::valueChanged, [AppState](int value)
	{
		update_whisper_thread_count(AppState, value);
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
