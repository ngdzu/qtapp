# Lesson 26 Quiz: CI, Docker, and Builds

1. What is the primary benefit of multi-stage Docker builds for Qt applications, and how much size reduction can typically be achieved?

2. In the following Dockerfile snippet, what's wrong with this approach?
   ```dockerfile
   FROM ubuntu:22.04
   RUN apt-get update && apt-get install -y qt6-base-dev cmake g++
   COPY . .
   RUN cmake -B build && cmake --build build
   CMD ["./build/myapp"]
   ```

3. How do you make GitHub Actions cache Qt installation between builds to speed up CI runs?

4. What environment variable can you check to detect if your Qt application is running inside a Docker container?

5. When using GitLab CI with artifacts, what's the advantage of separating build and test into different jobs/stages?

6. In a GitHub Actions build matrix testing Qt 6.5 and 6.6 on 3 platforms (Linux/Windows/macOS), how many build jobs run, and why might this fail if you have a free GitHub account?

7. What's the purpose of the `COPY --from=builder` command in multi-stage Docker builds?

8. How can you detect whether your Qt application was built in Debug or Release mode at runtime?

9. What's a common pitfall when running Qt GUI applications in Docker, and how do you fix it?

10. If your CI build passes but the deployed application crashes with "cannot find libQt6Core.so.6", what's the likely cause and solution?
