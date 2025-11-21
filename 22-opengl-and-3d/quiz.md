# Lesson 22 Quiz: OpenGL and 3D

1. What is QOpenGLWidget and why use it instead of raw OpenGL?

2. What are the three essential methods to override in QOpenGLWidget?

3. What does this code do?
```cpp
void initializeGL() override {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
}
```

4. When is paintGL() called?

5. Why do you need to call initializeOpenGLFunctions()?

6. How do you create continuous animation at 60 FPS?

7. What's the difference between GL_COLOR_BUFFER_BIT and GL_DEPTH_BUFFER_BIT?

8. How do you rotate a 3D object in OpenGL?

9. What happens if you don't enable GL_DEPTH_TEST in 3D rendering?

10. How does resizeGL() help maintain proper aspect ratio?
