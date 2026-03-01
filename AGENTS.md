## High Level Context
This is a voice typing application that will run as a native application on the host operating system
that will run audio transcription in real time to a locally hosted, in memory Speech To Text model
running via Whisper.cpp external project that is a core dependency. This application's main job is to
facilitate user control of:
piping their audio input -> whisper.cpp model -> insert text to focused text input field if available

## Reminders
use tab chars for indentation
use crlf line endings
follow the existing coding guidelines
do not remove code comments unless explicitly directed to
build the project using `build.bat debug`
use early returns in logical flow where possible, helps to reduce code indentation
try not to have long lines (more than 120 chars), unroll onto newlines if needed
do not insert comments unless explicitly directed to
