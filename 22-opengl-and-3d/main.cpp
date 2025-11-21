#include <QApplication>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QTimer>
#include <QVBoxLayout>
#include <QLabel>
#include <QWidget>
#include <cmath>

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    GLWidget(QWidget *parent = nullptr) : QOpenGLWidget(parent), angle(0.0f) {
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this]() {
            angle += 2.0f;
            if (angle > 360.0f) angle -= 360.0f;
            update();
        });
        timer->start(16);  // ~60 FPS
    }

protected:
    void initializeGL() override {
        initializeOpenGLFunctions();
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    }

    void paintGL() override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -5.0f);
        glRotatef(angle, 1.0f, 1.0f, 1.0f);
        
        drawCube();
    }

    void resizeGL(int w, int h) override {
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        
        float aspect = float(w) / float(h ? h : 1);
        float fov = 45.0f;
        float nearPlane = 0.1f;
        float farPlane = 100.0f;
        
        float top = nearPlane * std::tan(fov * 3.14159f / 360.0f);
        float right = top * aspect;
        glFrustum(-right, right, -top, top, nearPlane, farPlane);
        
        glMatrixMode(GL_MODELVIEW);
    }

private:
    void drawCube() {
        glBegin(GL_QUADS);
        
        // Front face (red)
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f,  1.0f);
        glVertex3f( 1.0f, -1.0f,  1.0f);
        glVertex3f( 1.0f,  1.0f,  1.0f);
        glVertex3f(-1.0f,  1.0f,  1.0f);
        
        // Back face (green)
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);
        glVertex3f(-1.0f,  1.0f, -1.0f);
        glVertex3f( 1.0f,  1.0f, -1.0f);
        glVertex3f( 1.0f, -1.0f, -1.0f);
        
        // Top face (blue)
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-1.0f,  1.0f, -1.0f);
        glVertex3f(-1.0f,  1.0f,  1.0f);
        glVertex3f( 1.0f,  1.0f,  1.0f);
        glVertex3f( 1.0f,  1.0f, -1.0f);
        
        // Bottom face (yellow)
        glColor3f(1.0f, 1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);
        glVertex3f( 1.0f, -1.0f, -1.0f);
        glVertex3f( 1.0f, -1.0f,  1.0f);
        glVertex3f(-1.0f, -1.0f,  1.0f);
        
        // Right face (cyan)
        glColor3f(0.0f, 1.0f, 1.0f);
        glVertex3f( 1.0f, -1.0f, -1.0f);
        glVertex3f( 1.0f,  1.0f, -1.0f);
        glVertex3f( 1.0f,  1.0f,  1.0f);
        glVertex3f( 1.0f, -1.0f,  1.0f);
        
        // Left face (magenta)
        glColor3f(1.0f, 0.0f, 1.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);
        glVertex3f(-1.0f, -1.0f,  1.0f);
        glVertex3f(-1.0f,  1.0f,  1.0f);
        glVertex3f(-1.0f,  1.0f, -1.0f);
        
        glEnd();
    }

    float angle;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Lesson 22: OpenGL and 3D");
    window.resize(600, 500);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&window);

    // Title
    QLabel *titleLabel = new QLabel("Qt OpenGL 3D Cube Demo");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px; background: white; padding: 5px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // OpenGL widget
    GLWidget *glWidget = new GLWidget();
    glWidget->setMinimumSize(400, 400);
    mainLayout->addWidget(glWidget);

    // Info label
    QLabel *infoLabel = new QLabel("Rotating 3D cube with OpenGL • 6 colored faces • Depth testing enabled");
    infoLabel->setStyleSheet("color: #666; margin-top: 5px; background: white; padding: 5px;");
    infoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(infoLabel);

    window.show();

    return app.exec();
}
