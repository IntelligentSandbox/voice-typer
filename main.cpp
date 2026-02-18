#include <QApplication>
#include <QLabel>
#include <QFont>
#include <QComboBox>
#include <QGridLayout>
#include <QPlainTextEdit>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QObject>

#ifdef DEBUG
#include <QDebug>
#endif

#include "system.h"

struct GlobalState
{
    int CurrentAudioDeviceIndex;
    QList<QAudioDevice> AudioInputDevices;
    QWidget *MainWindow;

    int CurrentInferenceDeviceIndex;
    QList<QString> InferenceDevices;

    int CurrentSTTModelIndex;
    QList<QString> STTModels;

    SystemInfo SystemInfo;
    CPUInfo CpuInfo;
};

void query_audio_input_devices(GlobalState *AppState)
{
    AppState->CurrentAudioDeviceIndex = -1;
    AppState->AudioInputDevices = QMediaDevices::audioInputs();

    QString DefaultAudioDeviceDescription = QMediaDevices::defaultAudioInput().description();
    for (int i = 0; i < AppState->AudioInputDevices.size(); i++)
    {
        QString Description = AppState->AudioInputDevices.at(i).description();
        if (DefaultAudioDeviceDescription == Description)
        {
            AppState->CurrentAudioDeviceIndex = i;
            break;
        }
    }

    if (AppState->CurrentAudioDeviceIndex == -1 && AppState->AudioInputDevices.size() > 0)
    {
        AppState->CurrentAudioDeviceIndex = 0;
    }
    // TODO(warren): if CurrentAudioDeviceIndex is still -1, assume no audio input devices. Then what?
}

void query_inference_devices(GlobalState *AppState)
{
    // TODO(warren): When we have the CPU name, use that instead.
    AppState->InferenceDevices = QList<QString>();
    AppState->InferenceDevices.append("CPU");
    AppState->CurrentInferenceDeviceIndex = 0;

    // TODO(warren): Same thing for GPUs...
    // And maybe in the future if auto detect a good gpu, default to that.
    AppState->InferenceDevices.append("GPU");
}

void query_available_stt_models(GlobalState *AppState)
{
    AppState->STTModels = QList<QString>();

    // TODO(warren): Validate model file is present before setting in future.
    AppState->STTModels.append("Whisper.cpp (GGML) - base.en");
    AppState->CurrentSTTModelIndex = 0;
}

void init_gui(GlobalState *AppState)
{
    // NOTE(warren): If need multiple screens of buttons/settings, although ideally just put everything on the first screen, 
    // use multiple layouts I think and a tab/option bar at the top to switch between them.
    QWidget *MainWindow = AppState->MainWindow;
    QList<QAudioDevice> *AudioInputDevices = &AppState->AudioInputDevices;

    // ---- Left Column ----
    QGridLayout *GridLayout = new QGridLayout(MainWindow);
    QLabel *AudioSelectLabel = new QLabel("Audio Input", MainWindow);
    GridLayout->addWidget(AudioSelectLabel, 0, 0);

    QComboBox *AudioInputSelect = new QComboBox(MainWindow);
    QString CurrentAudioDeviceDescription;
    for (int i = 0; i < AudioInputDevices->size(); i++)
    {
        QAudioDevice Device = AudioInputDevices->at(i);
        QString Description = Device.description();
        AudioInputSelect->addItem(Description);
        if (AppState->CurrentAudioDeviceIndex == i)
        {
            AudioInputSelect->setCurrentIndex(i);
        }
    }
    AudioInputSelect->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    GridLayout->addWidget(AudioInputSelect, 1, 0);

    QLabel *ModelSelectLabel = new QLabel("STT Model", MainWindow);
    GridLayout->addWidget(ModelSelectLabel, 2, 0);
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
    GridLayout->addWidget(ModelSelect, 3, 0);

    QLabel *InferenceDeviceSelectLabel = new QLabel("Inference Device", MainWindow);
    GridLayout->addWidget(InferenceDeviceSelectLabel, 4, 0);
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

    GridLayout->addWidget(InferenceDeviceSelect, 5, 0);

    QLabel *RawTextOutputLabel = new QLabel("Raw Text Output", MainWindow);
    GridLayout->addWidget(RawTextOutputLabel, 6, 0);
    QPlainTextEdit* RawTextOutputTextBox = new QPlainTextEdit("TODO", MainWindow);
    RawTextOutputTextBox->setReadOnly(true);
    GridLayout->addWidget(RawTextOutputTextBox, 7, 0);

    // ---- Right Column ----
    QLabel *InferenceDeviceLabel = new QLabel("Inference Device: ", MainWindow);
    GridLayout->addWidget(InferenceDeviceLabel, 0, 1);

    QLabel *InferenceDeviceDetailsLabel = new QLabel("", MainWindow);
    GridLayout->addWidget(InferenceDeviceDetailsLabel, 0, 2);

    // --- "Event Listeners" ----
    QObject::connect(AudioInputSelect, &QComboBox::currentIndexChanged, [AppState](int index)
    {
        AppState->CurrentAudioDeviceIndex = index;
#ifdef DEBUG
        qDebug() << "Selected audio input device index: " << index;
        if (index < AppState->AudioInputDevices.size())
        {
            QAudioDevice Device = AppState->AudioInputDevices.at(index);
            QString Description = Device.description();
            qDebug() << "Selected audio input device description: " << Description;
        }
#endif
    });

    QObject::connect(InferenceDeviceSelect, &QComboBox::currentIndexChanged, [AppState, InferenceDeviceDetailsLabel](int index)
    {
        AppState->CurrentInferenceDeviceIndex = index;
        if (index < AppState->InferenceDevices.size())
        {
            InferenceDeviceDetailsLabel->setText(AppState->InferenceDevices.at(index));
        }
#ifdef DEBUG
        qDebug() << "Selected inference device index: " << index;
#endif
    });
}

int main(int argc, char *argv[])
{
    // TODO(warren): Need some thinking on the arch, need to initialize a local model first, etc.
    // Although, we should query the machine for specs and try to determine if any model we will have
    // the open weights for will be able to load/run at all. If not, do run a window and show an error
    // message...suggesting need better specs and provide a yolo button to allow trying to run on potatoes.
    
    QApplication App(argc, argv);

    QFont Font("Georgia");
    Font.setPointSize(12);
    Font.setWeight(QFont::Bold);
    App.setFont(Font);

    QWidget MainWindow;
    MainWindow.setWindowTitle("Voice Typer");
    MainWindow.resize(500, 500);

    GlobalState AppState;
    AppState.MainWindow = &MainWindow;

    query_system_info(&AppState.SystemInfo);
    query_audio_input_devices(&AppState);
    query_inference_devices(&AppState);
    query_available_stt_models(&AppState);
    query_cpu_info(&AppState.CpuInfo);

#ifdef DEBUG
    qDebug() << "Available Audio Input Devices:" << AppState.AudioInputDevices.count();
    qDebug() << "Default Audio Input device:" << QMediaDevices::defaultAudioInput().description();
    for (const QAudioDevice &device : AppState.AudioInputDevices)
    {
        qDebug() << "Audio Input Devices:" << device.description();
    }
    qDebug() << "OS:" << AppState.SystemInfo.ProductName;
    qDebug() << "CPU arch:" << AppState.CpuInfo.Architecture;
    qDebug() << "CPU cores:" << AppState.CpuInfo.LogicalCores;
#endif

    init_gui(&AppState);

    MainWindow.show();
    return App.exec();
}
