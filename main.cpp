#include <QApplication>
#include <QLabel>
#include <QFont>
#include <QComboBox>
#include <QGridLayout>
#include <QPlainTextEdit>
#include <QAudioDevice>
#include <QMediaDevices>

#ifdef DEBUG
#include <QDebug>
#endif

void init_gui(QWidget *MainWindow, QList<QAudioDevice> *AudioInputDevices)
{
    // NOTE(warren): If need multiple screens of buttons/settings, although ideally just put everything on the first screen, 
    // use multiple layouts I think and a tab/option bar at the top to switch between them.

    QGridLayout *GridLayout = new QGridLayout(MainWindow);
    QLabel *AudioSelectLabel = new QLabel("Audio Input", MainWindow);
    GridLayout->addWidget(AudioSelectLabel, 0, 0);

    QComboBox *AudioInputSelect = new QComboBox(MainWindow);
    QString DefaultAudioDeviceDescription = QMediaDevices::defaultAudioInput().description();
    QString CurrentAudioDeviceDescription;
    for (int i = 0; i < AudioInputDevices->size(); i++)
    {
        QString Description = AudioInputDevices->at(i).description();
        AudioInputSelect->addItem(Description);
        if (DefaultAudioDeviceDescription == Description)
        {
            AudioInputSelect->setCurrentIndex(i);
        }
    }
    AudioInputSelect->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    // TODO(warren): Need to attach a function that listens to and sets the selected value
    // on the AudioInputSelect, storing that into a state we maintain at a higher level somewhere...global state?
    GridLayout->addWidget(AudioInputSelect, 1, 0);

    QLabel *ModelSelectLabel = new QLabel("STT Model", MainWindow);
    GridLayout->addWidget(ModelSelectLabel, 2, 0);
    QComboBox *ModelSelect = new QComboBox(MainWindow);
    ModelSelect->addItem("TODO");
    GridLayout->addWidget(ModelSelect, 3, 0);

    QLabel *InferenceDeviceSelectLabel = new QLabel("Inference Device", MainWindow);
    GridLayout->addWidget(InferenceDeviceSelectLabel, 4, 0);
    QComboBox *InferenceDeviceSelect = new QComboBox(MainWindow);
    InferenceDeviceSelect->addItem("TODO");
    GridLayout->addWidget(InferenceDeviceSelect, 5, 0);

    QLabel *RawTextOutputLabel = new QLabel("Raw Text Output", MainWindow);
    GridLayout->addWidget(RawTextOutputLabel, 6, 0);
    QPlainTextEdit* RawTextOutputTextBox = new QPlainTextEdit("TODO", MainWindow);
    RawTextOutputTextBox->setReadOnly(true);
    GridLayout->addWidget(RawTextOutputTextBox, 7, 0);
}

int main(int argc, char *argv[])
{
    // TODO(warren): Need some thinking on the arch, need to initialize a local model first, etc.
    // Although, we should query the machine for specs and try to determine if any model we will have
    // the open weights for will be able to load/run at all. If not, do run a window and show an error
    // message...suggesting need better specs and provide a yolo button to allow trying to run on potatoes.
    // ex. query_machine_specs()
    
    QApplication App(argc, argv);

    QFont Font("Georgia");
    Font.setPointSize(12);
    Font.setWeight(QFont::Bold);
    App.setFont(Font);

    QWidget MainWindow;
    MainWindow.setWindowTitle("Voice Typer");
    MainWindow.resize(500, 500);

    // TODO(warren): Maybe just have a global main chunk of
    // mem to store stuff, some kinda global allocator, arena allocator?
    QList<QAudioDevice> AudioInputDevices = QMediaDevices::audioInputs();
#ifdef DEBUG
    qDebug() << "Available Audio Input Devices:" << AudioInputDevices.count();
    qDebug() << "Default Audio Input device:" << QMediaDevices::defaultAudioInput().description();
    for (const QAudioDevice &device : AudioInputDevices)
    {
        qDebug() << "Audio Input Devices:" << device.description();
    }
#endif

    init_gui(&MainWindow, &AudioInputDevices);

    MainWindow.show();
    return App.exec();
}
