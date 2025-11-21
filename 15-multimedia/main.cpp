#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QFileInfo>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Lesson 15: Multimedia");

    QVBoxLayout *layout = new QVBoxLayout(&window);

    // Title
    QLabel *titleLabel = new QLabel("Qt Multimedia Demo");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    // Status label
    QLabel *statusLabel = new QLabel("Loading media...");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("color: #666; margin: 10px;");
    layout->addWidget(statusLabel);

    // Create media player
    QMediaPlayer *player = new QMediaPlayer();
    QAudioOutput *audioOutput = new QAudioOutput();
    player->setAudioOutput(audioOutput);

    // Load media file from the lesson directory (copied into container during build)
    QString mediaPath = "/opt/lesson15/SoundHelix-Song-1.mp3";
    QFileInfo fileInfo(mediaPath);

    if (fileInfo.exists())
    {
        player->setSource(QUrl::fromLocalFile(mediaPath));
        statusLabel->setText("Ready to play: " + fileInfo.fileName());
    }
    else
    {
        statusLabel->setText("No media file found");
        statusLabel->setStyleSheet("color: #f44336; margin: 10px;");
    }

    // Connect to player state changes
    QObject::connect(player, &QMediaPlayer::mediaStatusChanged, [statusLabel, fileInfo](QMediaPlayer::MediaStatus status)
                     {
        switch(status) {
            case QMediaPlayer::LoadingMedia:
                statusLabel->setText("Loading...");
                break;
            case QMediaPlayer::LoadedMedia:
                statusLabel->setText("Ready: " + fileInfo.fileName());
                statusLabel->setStyleSheet("color: #4CAF50; margin: 10px;");
                break;
            case QMediaPlayer::BufferingMedia:
                statusLabel->setText("Buffering...");
                break;
            case QMediaPlayer::EndOfMedia:
                statusLabel->setText("Finished playing");
                break;
            case QMediaPlayer::InvalidMedia:
                statusLabel->setText("Error: Cannot play media file");
                statusLabel->setStyleSheet("color: #f44336; margin: 10px;");
                break;
            default:
                break;
        } });

    // Log playback state changes
    QObject::connect(player, &QMediaPlayer::playbackStateChanged, [statusLabel](QMediaPlayer::PlaybackState state)
                     {
        qDebug() << "Playback state changed to:" << state;
        switch(state) {
            case QMediaPlayer::StoppedState:
                qDebug() << "  -> STOPPED";
                break;
            case QMediaPlayer::PlayingState:
                qDebug() << "  -> PLAYING";
                break;
            case QMediaPlayer::PausedState:
                qDebug() << "  -> PAUSED";
                break;
        } });

    // Log errors
    QObject::connect(player, &QMediaPlayer::errorOccurred, [](QMediaPlayer::Error error, const QString &errorString)
                     { qDebug() << "ERROR occurred:" << error << errorString; });

    // Log position changes to see if playback is actually progressing
    QObject::connect(player, &QMediaPlayer::positionChanged, [](qint64 position)
                     {
        static qint64 lastLogged = -1000;
        if (position - lastLogged >= 1000) { // Log every second
            qDebug() << "Position:" << position << "ms (" << position/1000 << "s)";
            lastLogged = position;
        } });

    // Play button
    QPushButton *playBtn = new QPushButton("Play");
    playBtn->setMinimumHeight(40);
    QObject::connect(playBtn, &QPushButton::clicked, [player, playBtn, statusLabel]()
                     {
        if (player->playbackState() == QMediaPlayer::PlayingState) {
            player->pause();
            playBtn->setText("Play");
            statusLabel->setText("⏸ Paused");
            statusLabel->setStyleSheet("color: #FF9800; margin: 10px;");
        } else {
            player->play();
            playBtn->setText("Pause");
            statusLabel->setText("▶ Playing");
            statusLabel->setStyleSheet("color: #4CAF50; font-weight: bold; margin: 10px;");
        } });
    layout->addWidget(playBtn);

    // Position/Seek control
    QLabel *positionLabel = new QLabel("Position: 0:00 / 0:00");
    layout->addWidget(positionLabel);

    QSlider *seekSlider = new QSlider(Qt::Horizontal);
    seekSlider->setRange(0, 0);
    seekSlider->setEnabled(false);

    // Flag to prevent position updates immediately after seeking
    bool *isSeeking = new bool(false);

    // Update slider when player position changes (but not when user is dragging or just seeked)
    QObject::connect(player, &QMediaPlayer::positionChanged, [seekSlider, positionLabel, player, isSeeking](qint64 position)
                     {
        if (!seekSlider->isSliderDown() && !(*isSeeking)) {
            seekSlider->setValue(position);
        }
        qint64 duration = player->duration();
        int posMin = position / 60000;
        int posSec = (position % 60000) / 1000;
        int durMin = duration / 60000;
        int durSec = (duration % 60000) / 1000;
        positionLabel->setText(QString("Position: %1:%2 / %3:%4")
            .arg(posMin).arg(posSec, 2, 10, QChar('0'))
            .arg(durMin).arg(durSec, 2, 10, QChar('0'))); });

    // Update slider range when duration changes
    QObject::connect(player, &QMediaPlayer::durationChanged, [seekSlider](qint64 duration)
                     {
        seekSlider->setRange(0, duration);
        seekSlider->setEnabled(duration > 0); });

    // Seek when user releases slider (works for both click and drag)
    QObject::connect(seekSlider, &QSlider::sliderReleased, [seekSlider, player, audioOutput, isSeeking]()
                     {
        qDebug() << "\n=== SEEK REQUESTED ===";
        qDebug() << "Slider value:" << seekSlider->value();
        qDebug() << "Player position BEFORE seek:" << player->position();
        qDebug() << "Playback state BEFORE seek:" << player->playbackState();
        qDebug() << "Media status BEFORE seek:" << player->mediaStatus();
        qDebug() << "Audio volume:" << audioOutput->volume();
        
        *isSeeking = true;
        
        bool wasPlaying = (player->playbackState() == QMediaPlayer::PlayingState);
        qDebug() << "Was playing before seek:" << wasPlaying;
        
        // Set the position
        player->setPosition(seekSlider->value());
        
        qDebug() << "Player position AFTER setPosition:" << player->position();
        qDebug() << "Playback state AFTER setPosition:" << player->playbackState();
        qDebug() << "Media status AFTER setPosition:" << player->mediaStatus();
        
        // If it was playing, ensure it continues playing after seek
        if (wasPlaying && player->playbackState() != QMediaPlayer::PlayingState) {
            qDebug() << "Playback stopped after seek, restarting...";
            player->play();
        }
        
        // Reset seeking flag after a short delay
        QTimer::singleShot(100, [isSeeking]() {
            *isSeeking = false;
            qDebug() << "Seeking flag cleared\n";
        }); });

    layout->addWidget(seekSlider);

    // Volume control
    QLabel *volumeLabel = new QLabel("Volume: 50%");
    layout->addWidget(volumeLabel);

    QSlider *volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(50);
    QObject::connect(volumeSlider, &QSlider::valueChanged, [audioOutput, volumeLabel](int value)
                     {
        audioOutput->setVolume(value / 100.0);
        volumeLabel->setText(QString("Volume: %1%").arg(value)); });
    layout->addWidget(volumeSlider);

    // Info label
    QLabel *infoLabel = new QLabel(
        "Playing: SoundHelix-Song-1.mp3 (included in container)\n\n"
        "Qt Multimedia demonstrates:\n"
        "• QMediaPlayer - Media playback control\n"
        "• QAudioOutput - Audio output management\n"
        "• Position seeking with real-time display\n"
        "• Volume control and playback state tracking\n\n"
        "Click Play to start the music!");
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #555; font-size: 11px; margin-top: 20px; padding: 10px; background: #e3f2fd; border-radius: 5px;");
    layout->addWidget(infoLabel);

    layout->addStretch();

    window.resize(400, 300);
    window.show();

    return app.exec();
}
