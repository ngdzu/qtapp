// Path: /path/to/project/tests/TestChatOverlay.cpp

#include <QtTest/QtTest>
#include "ChatOverlay.h"

class TestChatOverlay : public QObject
{
    Q_OBJECT

private slots:
    void testUISetup();
    void testMessageSubmission();
    void testApiIntegration();

private:
    ChatOverlay *chatOverlay;
};

void TestChatOverlay::testUISetup()
{
    chatOverlay = new ChatOverlay();
    QVERIFY(chatOverlay->findChild<QLineEdit *>("inputField") != nullptr);
    QVERIFY(chatOverlay->findChild<QVBoxLayout *>("chatLayout") != nullptr);
    delete chatOverlay;
}

void TestChatOverlay::testMessageSubmission()
{
    chatOverlay = new ChatOverlay();
    QLineEdit *inputField = chatOverlay->findChild<QLineEdit *>("inputField");
    QVBoxLayout *chatLayout = chatOverlay->findChild<QVBoxLayout *>("chatLayout");

    QSignalSpy spy(chatOverlay, SIGNAL(onMessageSubmitted()));
    inputField->setText("Hello, ChatGPT!");
    QTest::keyPress(inputField, Qt::Key_Return);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(chatLayout->count(), 2); // One for the input field, one for the user message
    delete chatOverlay;
}

void TestChatOverlay::testApiIntegration()
{
    chatOverlay = new ChatOverlay();
    QNetworkAccessManager *networkManager = chatOverlay->findChild<QNetworkAccessManager *>("networkManager");

    QSignalSpy spy(networkManager, SIGNAL(finished(QNetworkReply *)));
    chatOverlay->sendMessageToChatGPT("Test message");

    QVERIFY(spy.wait(5000)); // Wait for the network reply
    QCOMPARE(spy.count(), 1);
    delete chatOverlay;
}

QTEST_MAIN(TestChatOverlay)
#include "TestChatOverlay.moc"
