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

	init_whisper_state(&AppState.WhisperState);

	#ifdef DEBUG
		printf("[main] Whisper state initialized\n");
	#endif

	// query_system_info(&AppState.SystemInfo);
	query_audio_input_devices(&AppState);
	query_inference_devices(&AppState);
	query_available_stt_models(&AppState);
	// query_cpu_info(&AppState.CpuInfo);

	#ifdef DEBUG
		qDebug() << "Available Audio Input Devices:" << AppState.AudioInputDevices.size();
		for (const AudioInputDeviceInfo &device : AppState.AudioInputDevices)
		{
			qDebug() << "Audio Input Device:" << device.Name.c_str();
		}
	#endif

	start_hotkey_listener(&AppState);

	init_ui(&AppState);

	QtMainWindow.show();
	int exitCode = QtApp.exec();

	stop_hotkey_listener();

	if (is_whisper_model_loaded(&AppState.WhisperState))
	{
		#ifdef DEBUG
			printf("[main] Unloading whisper model on exit\n");
		#endif
		unload_whisper_model(&AppState.WhisperState);
	}
	
	return exitCode;
}
