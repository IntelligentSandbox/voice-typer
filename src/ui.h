#pragma once

#include "qt.h"
#include "state.h"
#include "control.h"

#include <QLineEdit>
#include <QKeyEvent>
#include <QFocusEvent>

// ---- Hotkey capture ----
// Plain data: what the user has pressed so far in the settings box.
struct HotkeyCaptureState
{
	HotkeyConfig Captured;
	bool         HasCapture;
};

// Minimal QObject subtype used purely as a Qt event filter — no Q_OBJECT, no
// signals, no slots, no constructors with logic.  The only reason this inherits
// QObject at all is that installEventFilter() requires a QObject*.
struct HotkeyEventFilter : QObject
{
	HotkeyCaptureState *State;
	QLineEdit          *Edit;

	bool eventFilter(QObject * /*Watched*/, QEvent *Event) override
	{
		if (Event->type() == QEvent::KeyPress)
		{
			QKeyEvent *KeyEv = static_cast<QKeyEvent *>(Event);
			Qt::KeyboardModifiers Mods = KeyEv->modifiers();
			Qt::Key               Key  = Qt::Key(KeyEv->key());

			bool IsModifierOnly = (Key == Qt::Key_Control ||
			                       Key == Qt::Key_Alt     ||
			                       Key == Qt::Key_Shift   ||
			                       Key == Qt::Key_Meta);

			if (Key == Qt::Key_Escape)
			{
				State->HasCapture = false;
				State->Captured   = HotkeyConfig{};
				Edit->clear();
				return true;
			}

			if (Mods == Qt::NoModifier && !IsModifierOnly) return false;

			HotkeyConfig Config;
			Config.Modifiers  = Mods;
			Config.Key        = IsModifierOnly ? Qt::Key(0) : Key;
			State->Captured   = Config;
			State->HasCapture = true;
			Edit->setText(Config.to_label());
			return true;
		}

		if (Event->type() == QEvent::FocusIn)
		{
			Edit->setStyleSheet("border: 2px solid #2196F3; background: #e8f4fd;");
			return false;
		}

		if (Event->type() == QEvent::FocusOut)
		{
			Edit->setStyleSheet("");
			return false;
		}

		return false;
	}
};

// Allocates and wires a hotkey-capture QLineEdit.
// The filter's lifetime is tied to Edit (Edit is its QObject parent).
inline
void
make_hotkey_capture_edit(
	QWidget            *Parent,
	HotkeyCaptureState *State,
	QLineEdit         **OutEdit,
	const HotkeyConfig &Initial)
{
	QLineEdit *Edit = new QLineEdit(Parent);
	Edit->setReadOnly(true);
	Edit->setPlaceholderText("Click here, then press your hotkey...");
	Edit->setAlignment(Qt::AlignCenter);

	State->Captured   = Initial;
	State->HasCapture = Initial.is_valid();
	if (State->HasCapture) Edit->setText(Initial.to_label());

	HotkeyEventFilter *Filter = new HotkeyEventFilter();
	Filter->State = State;
	Filter->Edit  = Edit;
	Edit->installEventFilter(Filter);
	// Give Edit ownership so the filter is destroyed with the widget
	Filter->setParent(Edit);

	*OutEdit = Edit;
}

// ---- Settings Window ----
// Layout:
//   Row 0 : "Keyboard Shortcuts" heading
//   Row 1 : [Record btn] [Stream btn] [Load/Unload Model btn]  — action selector
//   Row 2 : "Current: <label>"
//   Row 3 : hotkey capture box
//   Row 4 : hint text
//   Row 5 : [Set Hotkey btn] [Close btn]

// Shared state threaded through the lambda callbacks.
struct SettingsWindowState
{
	// Which action is selected (0=Record, 1=Stream, 2=LoadModel)
	int             SelectedAction;

	// Capture box state
	HotkeyCaptureState Capture;

	// Pointers back into the dialog so callbacks can refresh UI
	QLabel     *CurrentLabel;
	QLineEdit  *CaptureEdit;
	QPushButton *ActionButtons[3];
};

inline
void
settings_select_action(GlobalState *AppState, SettingsWindowState *S, int Action)
{
	S->SelectedAction = Action;

	// Highlight the selected button, un-highlight the others
	for (int I = 0; I < 3; I++)
	{
		S->ActionButtons[I]->setStyleSheet(
			I == Action ? BUTTON_STYLE_BLUE : "");
	}

	// Show the current hotkey for the selected action in the capture box
	HotkeyConfig *Configs[3] = {
		&AppState->RecordHotkey,
		&AppState->StreamHotkey,
		&AppState->LoadModelHotkey
	};

	HotkeyConfig Current = *Configs[Action];
	S->CurrentLabel->setText(QString("Current: %1").arg(Current.to_label()));

	// Reset capture box to the existing hotkey for this action
	S->Capture.Captured   = Current;
	S->Capture.HasCapture = Current.is_valid();
	S->CaptureEdit->setText(Current.to_label());
}

inline
void
open_settings_window(GlobalState *AppState)
{
	QDialog *Dialog = new QDialog(AppState->QtMainWindow);
	Dialog->setWindowTitle("Settings");
	Dialog->setModal(true);
	Dialog->resize(500, 280);

	QGridLayout *Layout = new QGridLayout(Dialog);
	int Row = 0;

	// Heading
	QLabel *Heading = new QLabel("Keyboard Shortcuts", Dialog);
	QFont HeadingFont = Heading->font();
	HeadingFont.setPointSize(11);
	HeadingFont.setWeight(QFont::Bold);
	Heading->setFont(HeadingFont);
	Layout->addWidget(Heading, Row++, 0, 1, 3);

	// Action selector row
	QPushButton *RecordBtn    = new QPushButton("Record",           Dialog);
	QPushButton *StreamBtn    = new QPushButton("Stream",           Dialog);
	QPushButton *LoadModelBtn = new QPushButton("Load / Unload Model", Dialog);
	RecordBtn->setMinimumHeight(40);
	StreamBtn->setMinimumHeight(40);
	LoadModelBtn->setMinimumHeight(40);
	Layout->addWidget(RecordBtn,    Row, 0);
	Layout->addWidget(StreamBtn,    Row, 1);
	Layout->addWidget(LoadModelBtn, Row, 2);
	Row++;

	// "Current: ..." label
	QLabel *CurrentLabel = new QLabel("", Dialog);
	Layout->addWidget(CurrentLabel, Row++, 0, 1, 3);

	// Shared state declared here so the capture edit writes directly into S.Capture
	SettingsWindowState S = {};
	S.SelectedAction   = 0;
	S.CurrentLabel     = CurrentLabel;
	S.ActionButtons[0] = RecordBtn;
	S.ActionButtons[1] = StreamBtn;
	S.ActionButtons[2] = LoadModelBtn;

	// Hotkey capture box — filter writes into S.Capture
	QLineEdit *CaptureEdit = nullptr;
	make_hotkey_capture_edit(Dialog, &S.Capture, &CaptureEdit, AppState->RecordHotkey);
	CaptureEdit->setMinimumHeight(40);
	Layout->addWidget(CaptureEdit, Row++, 0, 1, 3);
	S.CaptureEdit = CaptureEdit;

	// Hint
	QLabel *HintLabel = new QLabel(
		"Select an action above, then click the box and press your desired combination.\n"
		"Modifier-only combos (e.g. Ctrl+Alt) are supported. Escape clears the box.",
		Dialog);
	HintLabel->setWordWrap(true);
	QFont HintFont = HintLabel->font();
	HintFont.setPointSize(9);
	HintFont.setWeight(QFont::Normal);
	HintLabel->setFont(HintFont);
	Layout->addWidget(HintLabel, Row++, 0, 1, 3);

	// Set Hotkey / Close buttons
	QPushButton *SetButton   = new QPushButton("Set Hotkey", Dialog);
	QPushButton *CloseButton = new QPushButton("Close",      Dialog);
	SetButton->setMinimumHeight(40);
	CloseButton->setMinimumHeight(40);
	Layout->addWidget(SetButton,   Row, 0, 1, 2);
	Layout->addWidget(CloseButton, Row, 2);

	// Select Record by default to prime the UI
	settings_select_action(AppState, &S, 0);

	// Action selector connections — each button selects its action
	QObject::connect(RecordBtn, &QPushButton::clicked, [AppState, &S]()
	{
		settings_select_action(AppState, &S, 0);
	});
	QObject::connect(StreamBtn, &QPushButton::clicked, [AppState, &S]()
	{
		settings_select_action(AppState, &S, 1);
	});
	QObject::connect(LoadModelBtn, &QPushButton::clicked, [AppState, &S]()
	{
		settings_select_action(AppState, &S, 2);
	});

	// Set Hotkey — saves for the currently selected action
	QObject::connect(SetButton, &QPushButton::clicked, [AppState, Dialog, &S]()
	{
		if (!S.Capture.HasCapture)
		{
			QMessageBox::warning(Dialog, "No Hotkey Set",
				"Please press a key combination in the box before saving.");
			return;
		}

		const char *JsonKeys[3]  = { "record_hotkey", "stream_hotkey", "load_model_hotkey" };
		HotkeyConfig *Targets[3] = {
			&AppState->RecordHotkey,
			&AppState->StreamHotkey,
			&AppState->LoadModelHotkey
		};

		apply_hotkey(AppState,
		             JsonKeys[S.SelectedAction],
		             Targets[S.SelectedAction],
		             S.Capture.Captured);

		// Refresh the "Current:" label to the newly saved value
		S.CurrentLabel->setText(
			QString("Current: %1").arg(S.Capture.Captured.to_label()));
	});

	QObject::connect(CloseButton, &QPushButton::clicked, [Dialog]()
	{
		Dialog->accept();
	});

	Dialog->exec();
	Dialog->deleteLater();
}

inline
void
init_main_window(GlobalState *AppState)
{
	QWidget *MainWindow = AppState->QtMainWindow;
	std::vector<AudioInputDeviceInfo> *AudioInputDevices = &(AppState->AudioInputDevices);

	QGridLayout *GridLayout = new QGridLayout(MainWindow);
	int Row = 0;

	// Record Button — label reflects current hotkey
	QPushButton *RecordButton = new QPushButton(MainWindow);
	RecordButton->setStyleSheet(BUTTON_STYLE_GREEN);
	RecordButton->setText(QString("Record (%1)").arg(AppState->RecordHotkey.to_label()));
	RecordButton->setMinimumHeight(60);
	GridLayout->addWidget(RecordButton, Row++, 0);
	AppState->RecordButton = RecordButton;

	// Stream Button
	QPushButton *StreamButton = new QPushButton(MainWindow);
	StreamButton->setStyleSheet(BUTTON_STYLE_GREEN);
	StreamButton->setText(stream_button_idle_label(AppState));
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
	LoadModelButton->setText(load_model_button_idle_label(AppState));
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

	// Settings Button
	QPushButton *SettingsButton = new QPushButton(MainWindow);
	SettingsButton->setText("Settings");
	SettingsButton->setMinimumHeight(40);
	GridLayout->addWidget(SettingsButton, Row++, 0);
	AppState->SettingsButton = SettingsButton;

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

	QObject::connect(SettingsButton, &QPushButton::clicked, [AppState]()
	{
		open_settings_window(AppState);
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
