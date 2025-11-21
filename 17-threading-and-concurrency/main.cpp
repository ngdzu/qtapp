#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QThread>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QProgressBar>
#include <QTimer>
#include <QRandomGenerator>

// Worker class that performs background work
class Worker : public QObject
{
    Q_OBJECT

public:
    explicit Worker(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    void doWork()
    {
        qDebug() << "Worker started on thread:" << QThread::currentThreadId();

        for (int i = 0; i <= 100; i += 10)
        {
            if (QThread::currentThread()->isInterruptionRequested())
            {
                emit finished(false);
                return;
            }

            QThread::msleep(300);
            emit progressChanged(i);
        }

        emit finished(true);
    }

signals:
    void progressChanged(int value);
    void finished(bool success);
};

// Function for QtConcurrent demonstration
int heavyComputation(int n)
{
    qDebug() << "Computing sum on thread:" << QThread::currentThreadId();
    int sum = 0;
    for (int i = 0; i <= n; ++i)
    {
        sum += i;
        if (i % 10000000 == 0)
        {
            QThread::msleep(100);
        }
    }
    return sum;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "Main thread:" << QThread::currentThreadId();

    QWidget window;
    window.setWindowTitle("Lesson 17: Threading and Concurrency");
    QVBoxLayout *layout = new QVBoxLayout(&window);

    QLabel *titleLabel = new QLabel("Threading and Concurrency Demo");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    // QThread Example Section
    QLabel *threadLabel = new QLabel("QThread Example:");
    threadLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    layout->addWidget(threadLabel);

    QLabel *threadStatusLabel = new QLabel("Status: Ready");
    layout->addWidget(threadStatusLabel);

    QProgressBar *threadProgress = new QProgressBar();
    layout->addWidget(threadProgress);

    QPushButton *threadBtn = new QPushButton("Start QThread Task");
    layout->addWidget(threadBtn);

    // QtConcurrent Example Section
    QLabel *concurrentLabel = new QLabel("QtConcurrent Example:");
    concurrentLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    layout->addWidget(concurrentLabel);

    QLabel *concurrentStatusLabel = new QLabel("Status: Ready");
    layout->addWidget(concurrentStatusLabel);

    QProgressBar *concurrentProgress = new QProgressBar();
    concurrentProgress->setRange(0, 0);
    concurrentProgress->setVisible(false);
    layout->addWidget(concurrentProgress);

    QPushButton *concurrentBtn = new QPushButton("Start QtConcurrent Task");
    layout->addWidget(concurrentBtn);

    // UI Responsiveness Test
    QLabel *counterLabel = new QLabel("UI Counter: 0 (proves UI isn't frozen)");
    counterLabel->setStyleSheet("color: #666; margin-top: 10px; font-size: 11px;");
    layout->addWidget(counterLabel);

    // Info label
    QLabel *infoLabel = new QLabel(
        "Qt Threading demonstrates:\n"
        "• QThread - Move work to background thread\n"
        "• QtConcurrent - High-level parallel execution\n"
        "• Thread-safe signal/slot communication\n"
        "• UI remains responsive during background work\n\n"
        "Watch the counter update while tasks run!");
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #555; font-size: 11px; margin-top: 10px; padding: 10px; background: #e3f2fd; border-radius: 5px;");
    layout->addWidget(infoLabel);

    layout->addStretch();

    // UI counter to prove UI thread isn't blocked
    int *counter = new int(0);
    QTimer *counterTimer = new QTimer(&window);
    QObject::connect(counterTimer, &QTimer::timeout, [counterLabel, counter]()
                     {
        (*counter)++;
        counterLabel->setText(QString("UI Counter: %1 (proves UI isn't frozen)").arg(*counter)); });
    counterTimer->start(100);

    // QThread setup
    QThread *workerThread = new QThread(&window);
    Worker *worker = new Worker();
    worker->moveToThread(workerThread);

    QObject::connect(workerThread, &QThread::started, worker, &Worker::doWork);
    QObject::connect(worker, &Worker::finished, workerThread, &QThread::quit);
    QObject::connect(workerThread, &QThread::finished, [threadStatusLabel, threadBtn]()
                     {
        threadStatusLabel->setText("Status: Thread finished");
        threadStatusLabel->setStyleSheet("color: #4CAF50; font-weight: bold;");
        threadBtn->setEnabled(true); });

    QObject::connect(worker, &Worker::progressChanged, [threadProgress, threadStatusLabel](int value)
                     {
        threadProgress->setValue(value);
        threadStatusLabel->setText(QString("Status: Working... %1%").arg(value));
        threadStatusLabel->setStyleSheet("color: #2196F3;"); });

    QObject::connect(threadBtn, &QPushButton::clicked, [workerThread, threadBtn, threadProgress, threadStatusLabel]()
                     {
        threadBtn->setEnabled(false);
        threadProgress->setValue(0);
        threadStatusLabel->setText("Status: Starting thread...");
        threadStatusLabel->setStyleSheet("color: #FF9800;");
        workerThread->start(); });

    // QtConcurrent setup
    QFutureWatcher<int> *watcher = new QFutureWatcher<int>(&window);

    QObject::connect(watcher, &QFutureWatcher<int>::finished, [watcher, concurrentStatusLabel, concurrentProgress, concurrentBtn]()
                     {
        int result = watcher->result();
        concurrentStatusLabel->setText(QString("Status: Completed! Result = %1").arg(result));
        concurrentStatusLabel->setStyleSheet("color: #4CAF50; font-weight: bold;");
        concurrentProgress->setVisible(false);
        concurrentBtn->setEnabled(true); });

    QObject::connect(concurrentBtn, &QPushButton::clicked, [watcher, concurrentBtn, concurrentProgress, concurrentStatusLabel]()
                     {
        concurrentBtn->setEnabled(false);
        concurrentProgress->setVisible(true);
        concurrentStatusLabel->setText("Status: Computing in parallel...");
        concurrentStatusLabel->setStyleSheet("color: #2196F3;");
        
        QFuture<int> future = QtConcurrent::run(heavyComputation, 100000000);
        watcher->setFuture(future); });

    window.resize(450, 500);
    window.show();

    return app.exec();
}

#include "main.moc"
