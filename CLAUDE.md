# CLAUDE.md

## Environment

You are running on **Windows**. Always use **CMD prompt** commands (not bash, PowerShell, or Linux shell commands).
Examples: use `dir` not `ls`, use `copy` not `cp`, use backslashes for paths, use `set` not `export`.

## Project Overview

This is a voice typing application that runs as a native application on the host OS.
It performs real-time audio transcription using a locally hosted, in-memory Speech-to-Text model
via **Whisper.cpp** (an external project and core dependency).

The application's primary responsibility is facilitating user control of:

	audio input -> Whisper.cpp model -> insert text into focused text input field (if available)

## Build Commands

	build.bat          - release build
	build.bat debug    - debug build

Always ensure changes pass **both** release and debug builds before considering work complete.

## Coding Guidelines

### Indentation & Formatting
- Use **tab characters** for indentation (not spaces)
- Use **LF line endings**
- Keep lines under **120 characters**; unroll onto multiple lines if needed

### Comments
- Do **not** insert comments unless explicitly directed to
- Do **not** remove existing code comments unless explicitly directed to

### Control Flow
- Use **early returns** where possible to reduce nesting/indentation
- Single-line `if` statements are allowed without curly brackets, but must fit on one line within 120 chars;
  otherwise use curly brackets on separate lines
- Any `if` and `else` statements must use curly brackets for **both** block scopes
- Single-statement `for` loops **must** use curly brackets

### General
- Follow existing coding style and conventions already present in the codebase
