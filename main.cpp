#include <QApplication>
#include <QLabel>
#include <QFont>

// TODO(warren): discover the available audio input devices
// surely QT has a way to do that talking to the OS
void query_audio_input_dev()
{

}

void init_gui(QWidget *MainWindow)
{
    int MarginOffset = 10;
    int SizeInBetweenLabels = 25;
    int DefaultLabelMinWidth = 150;

    // x,y,width,height

    // TODO(warren): how to get dynamic resizable ui?
    QLabel *AudioSelectLabel = new QLabel("Audio Input", MainWindow);
    AudioSelectLabel->setGeometry(MarginOffset, 10, DefaultLabelMinWidth, 25);

    // TODO(warren): dropdown QComboBox to select audio input devices

    QLabel *ModelSelectLabel = new QLabel("STT Model", MainWindow);
    ModelSelectLabel->setGeometry(MarginOffset + SizeInBetweenLabels + DefaultLabelMinWidth,
                                  10,
                                  DefaultLabelMinWidth,
                                  25);

    QLabel *InferenceDeviceSelectLabel = new QLabel("Inference Device", MainWindow);
    InferenceDeviceSelectLabel->setGeometry(MarginOffset + SizeInBetweenLabels + (2*DefaultLabelMinWidth),
                                            10,
                                            DefaultLabelMinWidth,
                                            25);

    QLabel *RawTextOutputLabel = new QLabel("Raw Text Output", MainWindow);
    RawTextOutputLabel->setGeometry(MarginOffset,
                                    100,
                                    DefaultLabelMinWidth,
                                    25);
}

int main(int argc, char *argv[])
{
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

    // query_audio_input_dev

    init_gui(&MainWindow);

    MainWindow.show();
    return App.exec();
}
