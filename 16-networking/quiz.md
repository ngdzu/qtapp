# Lesson 16 Quiz: Networking

1. What is the role of QNetworkAccessManager in Qt networking?

2. How do you make a GET request to a URL?

3. What does this code do?
```cpp
QNetworkRequest request(url);
request.setRawHeader("Authorization", "Bearer abc123");
QNetworkReply *reply = manager->get(request);
```

4. Why must you call `reply->deleteLater()` after processing a response?

5. How do you handle network errors in Qt?

6. What signal indicates a network request has completed?

7. How would you send JSON data in a POST request?

8. Can QNetworkAccessManager make multiple simultaneous requests?

9. What's the difference between synchronous and asynchronous network requests?

10. How do you add custom headers to a request?
