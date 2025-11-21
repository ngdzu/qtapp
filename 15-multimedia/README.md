# Lesson 15: Multimedia

This lesson demonstrates Qt Multimedia for audio playback with modern Qt 6 APIs.

## Prerequisites

### macOS X11 Setup

```bash
brew install --cask xquartz
open -a XQuartz
xhost + 127.0.0.1
```

### Linux X11 Setup

```bash
xhost +local:docker
```

## Building

The lesson includes a sample MP3 file (SoundHelix-Song-1.mp3) that gets bundled into the container.

```bash
cd 15-multimedia
docker build -t qt-lesson-15 .
```

## Running

### macOS

```bash
docker run --rm \
  -e DISPLAY=host.docker.internal:0 \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  -e PULSE_SERVER=tcp:host.docker.internal:4713 \
  qt-lesson-15
```

### Linux

```bash
docker run --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e QT_LOGGING_RULES="*.debug=false;qt.qpa.*=false" \
  qt-lesson-15
```

## Audio Output Setup (macOS)

For audio to work on macOS, you need PulseAudio:

```bash
# Install PulseAudio
brew install pulseaudio

# Start PulseAudio with network access
pulseaudio --load=module-native-protocol-tcp --exit-idle-time=-1 --daemon

# Or add to ~/.config/pulse/default.pa:
# load-module module-native-protocol-tcp auth-ip-acl=127.0.0.1
```

Then restart PulseAudio:
```bash
pulseaudio --kill
pulseaudio --start
```

## Expected Behavior

The application displays a multimedia player interface with:
- **Play/Pause button** - Controls playback of the bundled MP3 file
- **Volume slider** - Adjusts playback volume (0-100%)
- **Status display** - Shows current playback state (Loading, Ready, Playing, Paused, Finished)
- **Real audio playback** - Plays through your system's audio via PulseAudio

The SoundHelix sample music file is embedded in the container at build time, so no external files are needed.

## What You'll Learn

- Using `QMediaPlayer` for media playback
- Configuring `QAudioOutput` for audio routing
- Handling media status changes (Loading, Loaded, Playing, etc.)
- Volume control with `setVolume()`
- Playback state management (Playing, Paused, Stopped)
- Loading media from local files with `QUrl::fromLocalFile()`

## Troubleshooting

**No audio on macOS?**
- Ensure PulseAudio is running: `pulseaudio --check` (should return nothing if running)
- Check it's listening on port 4713: `netstat -an | grep 4713`
- Restart PulseAudio: `pulseaudio --kill && pulseaudio --start`

**Audio works but no sound?**
- Adjust the volume slider in the app
- Check your system volume isn't muted
- Try playing audio outside Docker to verify PulseAudio works
