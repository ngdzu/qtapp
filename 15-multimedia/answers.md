# Lesson 15 Quiz Answers

1. **What is the difference between QMediaPlayer and QAudioOutput?**

`QMediaPlayer` decodes and manages media playback. `QAudioOutput` routes audio to speakers/headphones and controls volume.

In Qt 6, you connect QAudioOutput to QMediaPlayer using `setAudioOutput()`. The player handles the media file, while QAudioOutput handles where the sound goes and how loud it is.

2. **How do you load a media file from a local path?**

Use `setSource()` with a `QUrl`:
```cpp
player->setSource(QUrl::fromLocalFile("/path/to/audio.mp3"));
```
Or for network URLs:
```cpp
player->setSource(QUrl("https://example.com/stream.mp3"));
```

3. **What does this code do?**

It creates a media player with audio output set to 75% volume.

The QAudioOutput is configured and connected to the player. When the player plays audio, it will route through this output at 75% of max volume.

4. **How do you detect when playback has finished?**

Connect to the `playbackStateChanged` signal:
```cpp
connect(player, &QMediaPlayer::playbackStateChanged,
        [](QMediaPlayer::PlaybackState state) {
    if (state == QMediaPlayer::StoppedState) {
        qDebug() << "Playback finished";
    }
});
```

5. **What signal reports the current playback position?**

`positionChanged(qint64 position)`:
```cpp
connect(player, &QMediaPlayer::positionChanged,
        [](qint64 pos) {
    qDebug() << "Position:" << pos << "ms";
});
```
Position is in milliseconds.

6. **How do you implement a seek/scrub bar for video playback?**

Use a QSlider connected bidirectionally:
```cpp
// Update slider when position changes
connect(player, &QMediaPlayer::positionChanged,
        slider, &QSlider::setValue);

// Seek when slider is moved
connect(slider, &QSlider::sliderMoved,
        player, &QMediaPlayer::setPosition);

// Set slider range based on duration
connect(player, &QMediaPlayer::durationChanged,
        slider, &QSlider::setMaximum);
```

7. **What are the three playback states of QMediaPlayer?**

(1) `PlayingState` - Media is playing
(2) `PausedState` - Media is paused
(3) `StoppedState` - Media is stopped or not loaded

8. **How do you handle media loading errors?**

Connect to the `errorOccurred` signal:
```cpp
connect(player, &QMediaPlayer::errorOccurred,
        [player](QMediaPlayer::Error error, const QString &errorString) {
    qDebug() << "Media error:" << errorString;
});
```

9. **Can QMediaPlayer play network streams (HTTP URLs)?**

Yes! Just use a network URL:
```cpp
player->setSource(QUrl("https://example.com/stream.mp3"));
```
The player will buffer and stream the content.

10. **What's the difference between pause() and stop()?**

`pause()` keeps the current position - resume with `play()` continues from where you paused.

`stop()` resets to the beginning - calling `play()` after stop starts from position 0. Stop also releases some resources.
