# Lesson 16: Networking

## Learning Goals
- Understand Qt Network module architecture
- Make HTTP requests with QNetworkAccessManager
- Handle network responses and errors
- Parse JSON data from web APIs
- Implement asynchronous network operations

## Introduction

Qt Network provides classes for network programming, supporting HTTP, TCP/UDP sockets, and SSL/TLS. The most common use case is making HTTP requests to REST APIs using `QNetworkAccessManager`, which handles requests asynchronously without blocking the UI.

The network module integrates seamlessly with Qt's event loop, using signals and slots to notify your application when data arrives or errors occur.

## Key Concepts

### QNetworkAccessManager

The central class for HTTP networking:

```cpp
QNetworkAccessManager *manager = new QNetworkAccessManager(parent);

// GET request
QNetworkRequest request(QUrl("https://api.example.com/data"));
QNetworkReply *reply = manager->get(request);

// Handle response
connect(reply, &QNetworkReply::finished, [reply]() {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        qDebug() << "Response:" << data;
    }
    reply->deleteLater();
});
```

### Request Headers

Customize requests with headers:

```cpp
QNetworkRequest request(url);
request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
request.setRawHeader("Authorization", "Bearer token123");
```

### POST Requests

Send data to servers:

```cpp
QNetworkRequest request(url);
request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

QJsonObject json;
json["name"] = "John";
json["age"] = 30;

QByteArray data = QJsonDocument(json).toJson();
QNetworkReply *reply = manager->post(request, data);
```

### Error Handling

Always handle network errors:

```cpp
connect(reply, &QNetworkReply::errorOccurred, 
        [reply](QNetworkReply::NetworkError error) {
    qDebug() << "Network error:" << reply->errorString();
});
```

## Example Walkthrough

Our example creates a simple HTTP client that:

1. **Fetch Button** - Makes GET request to httpbin.org
2. **Text Display** - Shows the response
3. **Error Handling** - Displays errors if request fails

The application demonstrates asynchronous networking without blocking the UI.

## Expected Output

A window with:
- "Fetch from httpbin.org" button
- Text area displaying JSON response
- Status updates during request/response

Clicking the button fetches data and displays it in the text area.

## Try It

1. Build and run the application
2. Click "Fetch from httpbin.org"
3. Observe the response appear in the text area
4. Try clicking multiple times to see async behavior
5. Check console for debug output

## Key Takeaways

- `QNetworkAccessManager` handles HTTP requests asynchronously
- Always connect to `finished()` signal to process responses
- Use `reply->deleteLater()` to prevent memory leaks
- Headers customize requests (auth, content-type, etc.)
- Error handling is essential for robust networking
- JSON parsing integrates well with Qt Network
- Network operations never block the UI thread
