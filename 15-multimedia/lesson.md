# Lesson 15: Multimedia

## Learning Goals
- Understand Qt Multimedia architecture
- Play audio and video files with QMediaPlayer
- Control playback, volume, and position
- Handle media status and errors
- Work with QAudioOutput for audio routing

## Introduction

Qt Multimedia provides classes for audio, video, radio, and camera functionality. The module uses a backend-based architecture, supporting different media engines on various platforms. For desktop applications, you'll primarily work with `QMediaPlayer` for playback and `QAudioOutput` (Qt 6) for audio routing.

The multimedia framework separates media sources (`QMediaPlayer`), output routing (`QAudioOutput`, `QVideoSink`), and UI components, giving you flexibility in building media applications.

## Key Concepts

### QMediaPlayer

`QMediaPlayer` is the core class for media playback:

```cpp
QMediaPlayer *player = new QMediaPlayer();
QAudioOutput *audioOutput = new QAudioOutput();
player->setAudioOutput(audioOutput);

// Load media
player->setSource(QUrl::fromLocalFile("/path/to/audio.mp3"));

// Control playback
player->play();
player->pause();
player->stop();
```

### Audio Output Routing

Qt 6 uses `QAudioOutput` to route audio to speakers:

```cpp
QAudioOutput *audioOutput = new QAudioOutput();
audioOutput->setVolume(0.5);  // 50% volume
player->setAudioOutput(audioOutput);
```

### Playback State

Monitor playback with signals:

```cpp
connect(player, &QMediaPlayer::playbackStateChanged, [](QMediaPlayer::PlaybackState state) {
    if (state == QMediaPlayer::PlayingState)
        qDebug() << "Playing";
    else if (state == QMediaPlayer::PausedState)
        qDebug() << "Paused";
});
```

### Position and Duration

Track playback progress:

```cpp
connect(player, &QMediaPlayer::positionChanged, [](qint64 position) {
    qDebug() << "Position:" << position << "ms";
});

connect(player, &QMediaPlayer::durationChanged, [](qint64 duration) {
    qDebug() << "Duration:" << duration << "ms";
});
```

## Example Walkthrough

Our example creates a simple audio player with:

1. **Play Button** - Starts/pauses playback
2. **Volume Slider** - Controls audio level (0-100%)
3. **Status Label** - Shows playback state

The player demonstrates basic multimedia operations without requiring an actual media file (for Docker compatibility).

## Expected Output

A window displaying:
- "Qt Multimedia Demo" title
- Play button (triggers playback simulation)
- Volume slider (0-100)
- Status updates when interacting with controls

Since we're running in Docker without actual media files, the demo simulates playback behavior.

## Try It

1. Build and run the application
2. Click the Play button to see state changes
3. Adjust the volume slider and observe the audio output changes
4. Check the console for playback messages

## Key Takeaways

- `QMediaPlayer` handles all media types (audio, video)
- `QAudioOutput` routes audio and controls volume in Qt 6
- Signals provide real-time playback state and position updates
- Media can be loaded from local files or network URLs
- Backends vary by platform (FFmpeg, GStreamer, AVFoundation, etc.)
- Always check media status to handle errors gracefully
