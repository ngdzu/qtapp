#include "ChatOverlay.h"
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray> // Add this line
#include <QNetworkRequest>
#include <QKeyEvent>

ChatOverlay::ChatOverlay(QWidget *parent) : QWidget(parent), networkManager(new QNetworkAccessManager(this))
{
    setupUI();
    connect(inputField, &QLineEdit::returnPressed, this, &ChatOverlay::onMessageSubmitted);
    connect(networkManager, &QNetworkAccessManager::finished, this, &ChatOverlay::onApiResponse);
}

void ChatOverlay::setupUI()
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QWidget *scrollWidget = new QWidget;
    chatLayout = new QVBoxLayout();
    scrollWidget->setLayout(chatLayout);

    scrollWidget->setLayout(chatLayout);
    inputField = new QLineEdit(this);
    chatLayout->addWidget(inputField);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);

    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->addWidget(scrollArea);
    setLayout(mainlayout);
}

void ChatOverlay::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        hide();
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void ChatOverlay::onMessageSubmitted()
{
    QString message = inputField->text();
    if (!message.isEmpty())
    {
        QLabel *userMessage = new QLabel("You: " + message, this);
        chatLayout->addWidget(userMessage);
        sendMessageToChatGPT(message);
        inputField->clear();
    }
}

void ChatOverlay::sendMessageToChatGPT(const QString &message)
{
    QNetworkRequest request(QUrl("https://api.openai.com/v1/engines/davinci-codex/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer YOUR_API_KEY");

    QJsonObject json;
    json["prompt"] = message;
    json["max_tokens"] = 150;

    QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());
}

void ChatOverlay::onApiResponse(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray response = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(response);
        QJsonObject jsonObject = jsonResponse.object();
        QString chatGPTResponse = jsonObject["choices"].toArray().at(0).toObject()["text"].toString();

        QLabel *responseLabel = new QLabel("ChatGPT: " + chatGPTResponse, this);
        chatLayout->addWidget(responseLabel);
    }
    else
    {
        QLabel *errorLabel = new QLabel("Error: " + reply->errorString(), this);
        chatLayout->addWidget(errorLabel);
    }
    reply->deleteLater();
}
