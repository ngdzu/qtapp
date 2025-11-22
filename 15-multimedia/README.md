# Lesson 15: Multimedia

This lesson demonstrates Qt Multimedia for audio playback with modern Qt 6 APIs.

## Building and Running

### One-Time Setup

These steps only need to be done once per machine.

#### 1. Install X11 Server

**For macOS users:**
- Install XQuartz: `brew install --cask xquartz`
- Start XQuartz and enable "Allow connections from network clients" in Preferences > Security

**For Linux users:**
- X11 should be available by default

#### 2. Build the shared Qt base images

From the **root directory** of the repository:

```bash
docker build --target qt-dev-env -t qtapp-qt-dev-env:latest .
docker build --target qt-runtime -t qtapp-qt-runtime:latest .
```

> **Note:** This lesson uses the standard `qt-runtime` base (not nano) because it requires Qt Multimedia libraries and GStreamer plugins. The image will be larger (~350MB) to support audio/video playback.

#### 3. Grant X11 access to Docker containers

From the **root directory** of the repository:

```bash
./scripts/xhost-allow-for-compose.sh allow
```

> **Note:** This disables X11 access control to allow Docker containers to display GUI applications. Run this once per session (after reboot, you'll need to run it again). To revoke access later, run `./scripts/xhost-allow-for-compose.sh revoke`.

#### 4. Audio Setup (macOS only)

For audio to work on macOS, you need PulseAudio:

```bash
brew install pulseaudio
pulseaudio --load=module-native-protocol-tcp --exit-idle-time=-1 --daemon
```

Or add to `~/.config/pulse/default.pa`:
```
load-module module-native-protocol-tcp auth-ip-acl=127.0.0.1
```

Then restart PulseAudio:
```bash
pulseaudio --kill
pulseaudio --start
```

### Build and Run This Lesson

#### Step 1: Build this lesson's image

From the **lesson directory** (`15-multimedia`):

```bash
docker build -t qtapp-lesson15:latest .
```

#### Step 2: Run the application

**On macOS:**

```bash
docker run --rm -e DISPLAY=host.docker.internal:0 -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" -e PULSE_SERVER=tcp:host.docker.internal:4713 qtapp-lesson15:latest
```

**On Linux:**

```bash
docker run --rm \
    -e DISPLAY=$DISPLAY \
    -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    qtapp-lesson15:latest
```

### Alternative: Build locally (requires Qt 6 installed)

```bash
mkdir build
cd build
cmake ..
cmake --build .
./lesson15
```

## What You'll See

A multimedia player interface with:
- **Play/Pause button** - Controls playback of the bundled MP3 file
- **Volume slider** - Adjusts playback volume (0-100%)
- **Status display** - Shows current playback state (Loading, Ready, Playing, Paused, Finished)
- **Real audio playback** - Plays through your system's audio via PulseAudio

The SoundHelix sample music file is embedded in the container at build time, so no external files are needed.

> **Note:** You may see harmless GL warnings in the console (like "failed to load driver: swrast"). These can be safely ignored - the application runs perfectly without hardware acceleration.

## Requirements

- **Qt Modules:** Qt6::Widgets, Qt6::Multimedia, Qt6::MultimediaWidgets
- **CMake:** 3.16 or higher
- **C++ Standard:** C++17
- **Docker:** For containerized build (recommended)
- **X11:** For GUI display on Linux/macOS
- **PulseAudio:** For audio playback (macOS)

## Learning Objectives

- Using `QMediaPlayer` for media playback
- Configuring `QAudioOutput` for audio routing
- Handling media status changes (Loading, Loaded, Playing, etc.)
- Volume control with `setVolume()`
- Playback state management (Playing, Paused, Stopped)
- Loading media from local files with `QUrl::fromLocalFile()`

## Notes

- The Dockerfile uses a multi-stage build with the standard `qt-runtime` base which includes SQL support
- This lesson adds Qt Multimedia libraries and GStreamer plugins for media playback
- The image size is ~350MB due to multimedia dependencies
- For headless testing or CI environments, you can use `Xvfb` (virtual framebuffer) instead of a real X11 server
- On Windows with Docker Desktop, use an X server like VcXsrv and set `DISPLAY=host.docker.internal:0`

## Troubleshooting

**No audio on macOS?**
- Ensure PulseAudio is running: `pulseaudio --check` (should return nothing if running)
- Check it's listening on port 4713: `netstat -an | grep 4713`
- Restart PulseAudio: `pulseaudio --kill && pulseaudio --start`

**Audio works but no sound?**
- Adjust the volume slider in the app
- Check your system volume settings
- Check your system volume isn't muted
- Try playing audio outside Docker to verify PulseAudio works
