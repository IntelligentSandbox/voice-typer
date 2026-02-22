#include "state.h"
#include "ui.h"
#include "system.h"

int main(int argc, char *argv[])
{
    // TODO(warren): Need some thinking on the arch, need to initialize a local model first, etc.
    // Although, we should query the machine for specs and try to determine if any model we will have
    // the open weights for will be able to load/run at all. If not, do run a window and show an error
    // message...suggesting need better specs and provide a yolo button to allow trying to run on potatoes.

    QApplication QtApp(argc, argv);
    QWidget QtMainWindow;

    GlobalState AppState;
    AppState.QtApp = &QtApp;
    AppState.QtMainWindow = &QtMainWindow;

    AppState.IsRecording = false;
    // query_system_info(&AppState.SystemInfo);
    query_audio_input_devices(&AppState);
    query_inference_devices(&AppState);
    query_available_stt_models(&AppState);
    // query_cpu_info(&AppState.CpuInfo);

#ifdef DEBUG
    qDebug() << "Available Audio Input Devices:" << AppState.AudioInputDevices.count();
    qDebug() << "Default Audio Input device:" << QMediaDevices::defaultAudioInput().description();
    for (const QAudioDevice &device : AppState.AudioInputDevices)
    {
        qDebug() << "Audio Input Devices:" << device.description();
    }
    // qDebug() << "OS:" << AppState.SystemInfo.ProductName;
    // qDebug() << "CPU arch:" << AppState.CpuInfo.Architecture;
    // qDebug() << "CPU cores:" << AppState.CpuInfo.LogicalCores;
#endif

    start_hotkey_listener(&AppState);

    init_ui(&AppState);
    // register_global_hotkeys(&AppState, &App);

    QtMainWindow.show();
    int exitCode = QtApp.exec();

    stop_hotkey_listener();
    
    // unregister_global_hotkeys(&AppState);
    return exitCode;
}
