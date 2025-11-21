#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Lesson 16: Networking");
    
    QVBoxLayout *layout = new QVBoxLayout(&window);

    // Title
    QLabel *titleLabel = new QLabel("Qt Network Demo - HTTP GET");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    // Status label
    QLabel *statusLabel = new QLabel("Ready");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("color: #666; margin: 5px;");
    layout->addWidget(statusLabel);

    // Text display
    QTextEdit *textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    textEdit->setPlaceholderText("Response will appear here...");
    layout->addWidget(textEdit);

    // Network manager
    QNetworkAccessManager *manager = new QNetworkAccessManager(&window);

    // Fetch button
    QPushButton *fetchBtn = new QPushButton("Fetch from httpbin.org/get");
    fetchBtn->setMinimumHeight(40);
    
    QObject::connect(fetchBtn, &QPushButton::clicked, [manager, textEdit, statusLabel]() {
        statusLabel->setText("Fetching...");
        textEdit->clear();
        
        QNetworkRequest request(QUrl("https://httpbin.org/get"));
        request.setHeader(QNetworkRequest::UserAgentHeader, "Qt6-NetworkDemo/1.0");
        
        QNetworkReply *reply = manager->get(request);
        
        // Handle successful response
        QObject::connect(reply, &QNetworkReply::finished, [reply, textEdit, statusLabel]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray responseData = reply->readAll();
                
                // Try to pretty-print JSON
                QJsonDocument doc = QJsonDocument::fromJson(responseData);
                if (!doc.isNull()) {
                    textEdit->setText(doc.toJson(QJsonDocument::Indented));
                    statusLabel->setText("Success! Response received.");
                    statusLabel->setStyleSheet("color: green; margin: 5px;");
                } else {
                    textEdit->setText(responseData);
                    statusLabel->setText("Response received (not JSON)");
                }
            } else {
                textEdit->setText("Error: " + reply->errorString());
                statusLabel->setText("Request failed");
                statusLabel->setStyleSheet("color: red; margin: 5px;");
            }
            reply->deleteLater();
        });
        
        // Handle errors
        QObject::connect(reply, &QNetworkReply::errorOccurred,
                        [reply, statusLabel](QNetworkReply::NetworkError) {
            statusLabel->setText("Error: " + reply->errorString());
            statusLabel->setStyleSheet("color: red; margin: 5px;");
        });
    });
    
    layout->addWidget(fetchBtn);

    // Info label
    QLabel *infoLabel = new QLabel(
        "This demo fetches data from httpbin.org, a free HTTP testing service. "
        "The response shows your request headers and origin IP."
    );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #999; font-size: 12px; margin-top: 10px;");
    layout->addWidget(infoLabel);

    window.resize(600, 500);
    window.show();

    return app.exec();
}
