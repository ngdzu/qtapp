#ifndef CHATOVERLAY_H
#define CHATOVERLAY_H

#include <QWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class QKeyEvent;

class ChatOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit ChatOverlay(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onMessageSubmitted();
    void onApiResponse(QNetworkReply *reply);

private:
    QLineEdit *inputField;
    QVBoxLayout *chatLayout;
    QNetworkAccessManager *networkManager;
    void setupUI();
    void sendMessageToChatGPT(const QString &message);
};

#endif // CHATOVERLAY_H
