#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QComboBox>
#include <QGraphicsOpacityEffect>
#include <QEasingCurve>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Lesson 21: Animation and Transitions");
    window.resize(700, 500);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&window);

    // Title
    QLabel *titleLabel = new QLabel("Qt Animation Framework Demo");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; margin: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Animation target widget
    QPushButton *animatedWidget = new QPushButton("Animated Widget");
    animatedWidget->setStyleSheet(R"(
        QPushButton {
            background-color: #4CAF50;
            color: white;
            font-size: 16px;
            font-weight: bold;
            border-radius: 10px;
            min-width: 120px;
            min-height: 60px;
        }
    )");
    animatedWidget->setGeometry(50, 100, 120, 60);
    
    // Opacity effect for fade animation
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect();
    animatedWidget->setGraphicsEffect(opacityEffect);
    opacityEffect->setOpacity(1.0);

    // Spacer to give space for animation
    QWidget *animationArea = new QWidget();
    animationArea->setMinimumHeight(200);
    animationArea->setStyleSheet("background-color: #f0f0f0; border-radius: 5px;");
    QVBoxLayout *areaLayout = new QVBoxLayout(animationArea);
    areaLayout->addWidget(animatedWidget);
    areaLayout->setAlignment(animatedWidget, Qt::AlignLeft | Qt::AlignTop);
    mainLayout->addWidget(animationArea);

    // Easing curve selector
    QHBoxLayout *easingLayout = new QHBoxLayout();
    QLabel *easingLabel = new QLabel("Easing Curve:");
    QComboBox *easingCombo = new QComboBox();
    easingCombo->addItem("Linear", QEasingCurve::Linear);
    easingCombo->addItem("InOutQuad", QEasingCurve::InOutQuad);
    easingCombo->addItem("OutBounce", QEasingCurve::OutBounce);
    easingCombo->addItem("InOutElastic", QEasingCurve::InOutElastic);
    easingCombo->addItem("OutBack", QEasingCurve::OutBack);
    easingCombo->setCurrentIndex(1);
    easingLayout->addWidget(easingLabel);
    easingLayout->addWidget(easingCombo);
    easingLayout->addStretch();
    mainLayout->addLayout(easingLayout);

    // Control buttons
    QHBoxLayout *buttonLayout1 = new QHBoxLayout();
    QPushButton *posBtn = new QPushButton("Animate Position");
    QPushButton *sizeBtn = new QPushButton("Animate Size");
    QPushButton *fadeBtn = new QPushButton("Fade Animation");
    buttonLayout1->addWidget(posBtn);
    buttonLayout1->addWidget(sizeBtn);
    buttonLayout1->addWidget(fadeBtn);
    mainLayout->addLayout(buttonLayout1);

    QHBoxLayout *buttonLayout2 = new QHBoxLayout();
    QPushButton *parallelBtn = new QPushButton("Parallel Animation");
    QPushButton *sequentialBtn = new QPushButton("Sequential Animation");
    QPushButton *resetBtn = new QPushButton("Reset");
    buttonLayout2->addWidget(parallelBtn);
    buttonLayout2->addWidget(sequentialBtn);
    buttonLayout2->addWidget(resetBtn);
    mainLayout->addLayout(buttonLayout2);

    // Status label
    QLabel *statusLabel = new QLabel("Ready");
    statusLabel->setStyleSheet("color: #666; margin-top: 10px;");
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);

    // Helper function to get current easing curve
    auto getCurrentEasing = [easingCombo]() {
        return static_cast<QEasingCurve::Type>(
            easingCombo->currentData().toInt()
        );
    };

    // Position animation
    QObject::connect(posBtn, &QPushButton::clicked, [=]() {
        QPropertyAnimation *anim = new QPropertyAnimation(animatedWidget, "pos");
        anim->setDuration(1000);
        anim->setStartValue(animatedWidget->pos());
        anim->setEndValue(QPoint(400, 100));
        anim->setEasingCurve(getCurrentEasing());
        
        QObject::connect(anim, &QPropertyAnimation::finished, [statusLabel]() {
            statusLabel->setText("✓ Position animation completed!");
        });
        
        statusLabel->setText("Running position animation...");
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    });

    // Size animation
    QObject::connect(sizeBtn, &QPushButton::clicked, [=]() {
        QPropertyAnimation *anim = new QPropertyAnimation(animatedWidget, "size");
        anim->setDuration(800);
        anim->setStartValue(animatedWidget->size());
        anim->setEndValue(QSize(200, 100));
        anim->setEasingCurve(getCurrentEasing());
        
        QObject::connect(anim, &QPropertyAnimation::finished, [statusLabel]() {
            statusLabel->setText("✓ Size animation completed!");
        });
        
        statusLabel->setText("Running size animation...");
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    });

    // Fade animation
    QObject::connect(fadeBtn, &QPushButton::clicked, [=]() {
        QPropertyAnimation *anim = new QPropertyAnimation(opacityEffect, "opacity");
        anim->setDuration(600);
        anim->setStartValue(1.0);
        anim->setKeyValueAt(0.5, 0.2);
        anim->setEndValue(1.0);
        anim->setEasingCurve(QEasingCurve::InOutQuad);
        
        QObject::connect(anim, &QPropertyAnimation::finished, [statusLabel]() {
            statusLabel->setText("✓ Fade animation completed!");
        });
        
        statusLabel->setText("Running fade animation...");
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    });

    // Parallel animation (move + resize + fade)
    QObject::connect(parallelBtn, &QPushButton::clicked, [=]() {
        QParallelAnimationGroup *group = new QParallelAnimationGroup();
        
        QPropertyAnimation *posAnim = new QPropertyAnimation(animatedWidget, "pos");
        posAnim->setDuration(1000);
        posAnim->setEndValue(QPoint(300, 50));
        posAnim->setEasingCurve(getCurrentEasing());
        
        QPropertyAnimation *sizeAnim = new QPropertyAnimation(animatedWidget, "size");
        sizeAnim->setDuration(1000);
        sizeAnim->setEndValue(QSize(180, 80));
        sizeAnim->setEasingCurve(getCurrentEasing());
        
        QPropertyAnimation *opacityAnim = new QPropertyAnimation(opacityEffect, "opacity");
        opacityAnim->setDuration(1000);
        opacityAnim->setKeyValueAt(0.5, 0.3);
        opacityAnim->setEndValue(1.0);
        
        group->addAnimation(posAnim);
        group->addAnimation(sizeAnim);
        group->addAnimation(opacityAnim);
        
        QObject::connect(group, &QParallelAnimationGroup::finished, [statusLabel]() {
            statusLabel->setText("✓ Parallel animation completed!");
        });
        
        statusLabel->setText("Running parallel animation (3 properties)...");
        group->start(QAbstractAnimation::DeleteWhenStopped);
    });

    // Sequential animation (move right -> move down -> move left)
    QObject::connect(sequentialBtn, &QPushButton::clicked, [=]() {
        QSequentialAnimationGroup *sequence = new QSequentialAnimationGroup();
        
        // Move right
        QPropertyAnimation *moveRight = new QPropertyAnimation(animatedWidget, "pos");
        moveRight->setDuration(500);
        moveRight->setEndValue(QPoint(400, animatedWidget->pos().y()));
        moveRight->setEasingCurve(getCurrentEasing());
        
        // Move down
        QPropertyAnimation *moveDown = new QPropertyAnimation(animatedWidget, "pos");
        moveDown->setDuration(500);
        moveDown->setEndValue(QPoint(400, 100));
        moveDown->setEasingCurve(getCurrentEasing());
        
        // Move left
        QPropertyAnimation *moveLeft = new QPropertyAnimation(animatedWidget, "pos");
        moveLeft->setDuration(500);
        moveLeft->setEndValue(QPoint(50, 100));
        moveLeft->setEasingCurve(getCurrentEasing());
        
        sequence->addAnimation(moveRight);
        sequence->addAnimation(moveDown);
        sequence->addAnimation(moveLeft);
        
        QObject::connect(sequence, &QSequentialAnimationGroup::finished, [statusLabel]() {
            statusLabel->setText("✓ Sequential animation completed!");
        });
        
        statusLabel->setText("Running sequential animation (3 steps)...");
        sequence->start(QAbstractAnimation::DeleteWhenStopped);
    });

    // Reset button
    QObject::connect(resetBtn, &QPushButton::clicked, [=]() {
        animatedWidget->setGeometry(50, 100, 120, 60);
        opacityEffect->setOpacity(1.0);
        statusLabel->setText("Reset to initial state");
    });

    window.show();

    return app.exec();
}
