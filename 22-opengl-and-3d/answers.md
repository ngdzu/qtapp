# Lesson 22 Quiz Answers

1. **What is QOpenGLWidget and why use it instead of raw OpenGL?**

QOpenGLWidget integrates OpenGL rendering with Qt's widget system.

It handles OpenGL context creation, event handling, layout integration, and cleanup automatically. You get OpenGL performance with Qt's signals/slots, layouts, and cross-platform compatibility. Raw OpenGL requires manual context management.

2. **What are the three essential methods to override in QOpenGLWidget?**

```cpp
initializeGL()   // One-time OpenGL setup when context is ready
paintGL()        // Render frame (called on update())
resizeGL(w, h)   // Handle window resize, update viewport
```

3. **What does this code do?**

Initializes OpenGL functions and enables depth testing.

`initializeOpenGLFunctions()` loads OpenGL function pointers for the current context. `glEnable(GL_DEPTH_TEST)` enables z-buffer testing so closer objects hide farther ones - essential for proper 3D rendering.

4. **When is paintGL() called?**

When the widget needs repainting - triggered by `update()`, resize, or window exposure.

Call `update()` to schedule a repaint. For animation, use a QTimer that calls `update()` regularly (e.g., every 16ms for 60 FPS).

5. **Why do you need to call initializeOpenGLFunctions()?**

To load OpenGL function pointers for the current platform.

OpenGL functions are loaded dynamically at runtime. This call binds them to the current OpenGL context. Without it, OpenGL calls will crash. Call it once in `initializeGL()`.

6. **How do you create continuous animation at 60 FPS?**

Use a QTimer with 16ms interval:
```cpp
QTimer *timer = new QTimer(this);
connect(timer, &QTimer::timeout, this, [this]() {
    angle += 2.0f;
    update();  // Triggers paintGL()
});
timer->start(16);  // ~60 FPS (1000ms / 60 ≈ 16ms)
```

7. **What's the difference between GL_COLOR_BUFFER_BIT and GL_DEPTH_BUFFER_BIT?**

**GL_COLOR_BUFFER_BIT** clears the color buffer (pixel colors to background). **GL_DEPTH_BUFFER_BIT** clears the depth buffer (z-buffer for 3D occlusion).

For 3D, clear both: `glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)`. Otherwise, previous frame's depth values cause rendering artifacts.

8. **How do you rotate a 3D object in OpenGL?**

Use `glRotatef()`:
```cpp
glLoadIdentity();
glTranslatef(0.0f, 0.0f, -5.0f);  // Move back
glRotatef(angle, 1.0f, 1.0f, 1.0f);  // Rotate around axis
// Draw object
```
Angle is in degrees, axis is (x, y, z) direction.

9. **What happens if you don't enable GL_DEPTH_TEST in 3D rendering?**

Objects render in draw order, not depth order - you see through objects incorrectly.

Without depth testing, later-drawn faces appear on top regardless of actual 3D position. Back faces show through front faces. The scene looks wrong and "inside-out".

10. **How does resizeGL() help maintain proper aspect ratio?**

It updates the viewport and projection matrix when the window size changes:
```cpp
void resizeGL(int w, int h) override {
    glViewport(0, 0, w, h);
    // Update projection with new aspect ratio
    float aspect = float(w) / float(h);
}
```
Without it, the scene would stretch/squash when resized.
