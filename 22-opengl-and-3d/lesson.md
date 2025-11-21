# Lesson 22: OpenGL and 3D

## Learning Goals
- Integrate OpenGL with Qt using QOpenGLWidget
- Understand the OpenGL rendering pipeline in Qt
- Create basic 3D scenes with OpenGL
- Handle OpenGL initialization and cleanup
- Implement custom rendering in paintGL()
- Work with OpenGL transformation matrices

## Introduction

Qt provides **QOpenGLWidget** for integrating OpenGL rendering into Qt applications. This allows you to create high-performance 3D graphics, games, and visualizations while maintaining Qt's event handling, layouts, and UI integration.

QOpenGLWidget handles the OpenGL context lifecycle and provides three key methods: `initializeGL()` for setup, `paintGL()` for rendering, and `resizeGL()` for viewport changes.

## Key Concepts

### QOpenGLWidget Basics

Create an OpenGL widget:

```cpp
class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    MyGLWidget(QWidget *parent = nullptr) : QOpenGLWidget(parent) {}

protected:
    void initializeGL() override {
        initializeOpenGLFunctions();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }

    void paintGL() override {
        glClear(GL_COLOR_BUFFER_BIT);
        // Drawing code here
    }

    void resizeGL(int w, int h) override {
        glViewport(0, 0, w, h);
    }
};
```

### The Three Essential Methods

**initializeGL()** - Called once when OpenGL context is ready:
- Initialize OpenGL functions
- Set up buffers, textures, shaders
- Load resources
- Set OpenGL state (clear color, depth testing, etc.)

**paintGL()** - Called whenever the widget needs repainting:
- Clear buffers
- Set up transformations
- Draw geometry
- Swap buffers (automatic)

**resizeGL(int w, int h)** - Called when widget is resized:
- Update viewport
- Adjust projection matrix
- Recalculate aspect ratio

### Basic Triangle Drawing

Draw a simple triangle:

```cpp
void paintGL() override {
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f);  // Red
    glVertex2f(-0.5f, -0.5f);
    
    glColor3f(0.0f, 1.0f, 0.0f);  // Green
    glVertex2f(0.5f, -0.5f);
    
    glColor3f(0.0f, 0.0f, 1.0f);  // Blue
    glVertex2f(0.0f, 0.5f);
    glEnd();
}
```

### 3D Transformations

Rotate a cube:

```cpp
void paintGL() override {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f);
    glRotatef(angle, 1.0f, 1.0f, 0.0f);
    
    // Draw cube...
}
```

### Animation with Timers

Continuous rendering:

```cpp
QTimer *timer = new QTimer(this);
connect(timer, &QTimer::timeout, this, [this]() {
    angle += 1.0f;
    update();  // Triggers paintGL()
});
timer->start(16);  // ~60 FPS
```

### Enable Depth Testing

For proper 3D rendering:

```cpp
void initializeGL() override {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

void paintGL() override {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Now faces render in correct order
}
```

## Example Walkthrough

Our example demonstrates:

1. **QOpenGLWidget Setup** - Proper initialization
2. **Rotating Triangle** - Basic 2D animation
3. **3D Cube** - Full 3D rendering with depth
4. **Color Gradients** - Vertex coloring
5. **Continuous Animation** - Timer-based updates
6. **Viewport Management** - Handling window resize

The application shows a rotating 3D cube with colored faces.

## Expected Output

A window with:
- Rotating 3D cube with colored faces
- Smooth animation at 60 FPS
- Proper depth sorting (no face-through-face rendering)
- Responsive to window resizing
- Dark background

## Try It

1. Build and run the application
2. Watch the cube rotate automatically
3. Resize the window - the cube scales appropriately
4. Observe the colored faces (red, green, blue, etc.)
5. Notice proper depth handling (back faces hidden)

## Key Takeaways

- **QOpenGLWidget** integrates OpenGL with Qt's widget system
- Inherit from QOpenGLWidget and QOpenGLFunctions
- **initializeGL()** - One-time setup when context is ready
- **paintGL()** - Called for every frame render
- **resizeGL()** - Update viewport when window size changes
- Call `initializeOpenGLFunctions()` before using OpenGL
- Use `update()` to trigger a repaint (calls paintGL)
- Enable GL_DEPTH_TEST for 3D rendering
- QTimer with ~16ms interval gives ~60 FPS
- Legacy OpenGL (glBegin/glEnd) works but modern shaders are preferred
- Always clear both color and depth buffers in 3D
