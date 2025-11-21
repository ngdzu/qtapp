# Lesson 16 Quiz Answers

1. **What is the role of QNetworkAccessManager in Qt networking?**

`QNetworkAccessManager` manages network operations and maintains connection pooling.

It handles all HTTP requests (GET, POST, PUT, DELETE), manages cookies, caching, and proxy settings. It's designed to be reused for multiple requests throughout your application's lifetime.

2. **How do you make a GET request to a URL?**

```cpp
QNetworkAccessManager *manager = new QNetworkAccessManager();
QNetworkRequest request(QUrl("https://api.example.com/data"));
QNetworkReply *reply = manager->get(request);
```

3. **What does this code do?**

It makes an authenticated GET request with a Bearer token.

The `Authorization` header is set with a bearer token, commonly used for API authentication. The `get()` method initiates the request asynchronously.

4. **Why must you call `reply->deleteLater()` after processing a response?**

To prevent memory leaks.

`QNetworkReply` objects are dynamically allocated and won't be automatically deleted. `deleteLater()` schedules deletion after the event loop processes pending events, which is safe even if called from within a slot.

5. **How do you handle network errors in Qt?**

Connect to the `errorOccurred` signal:
```cpp
connect(reply, &QNetworkReply::errorOccurred,
        [reply](QNetworkReply::NetworkError error) {
    qDebug() << "Error:" << reply->errorString();
});
```
Or check `reply->error()` in the `finished` slot.

6. **What signal indicates a network request has completed?**

`QNetworkReply::finished()`

This signal is emitted whether the request succeeded or failed. Always check `reply->error()` to determine the outcome.

7. **How would you send JSON data in a POST request?**

```cpp
QNetworkRequest request(url);
request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

QJsonObject json;
json["key"] = "value";
QByteArray data = QJsonDocument(json).toJson();

QNetworkReply *reply = manager->post(request, data);
```

8. **Can QNetworkAccessManager make multiple simultaneous requests?**

Yes! It manages multiple concurrent requests efficiently.

Each request returns its own `QNetworkReply` object. The manager handles connection pooling and can reuse TCP connections for better performance.

9. **What's the difference between synchronous and asynchronous network requests?**

Asynchronous requests (Qt's default) don't block the UI - you get results via signals.

Synchronous requests block execution until complete, freezing the UI. Qt strongly discourages synchronous requests in GUI applications. Use async with signal/slot instead.

10. **How do you add custom headers to a request?**

Use `setHeader()` for standard headers or `setRawHeader()` for custom ones:
```cpp
request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
request.setRawHeader("X-Custom-Header", "value");
```
