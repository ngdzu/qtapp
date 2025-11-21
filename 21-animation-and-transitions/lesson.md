# Lesson 21: Animation and Transitions

## Learning Goals
- Use QPropertyAnimation for smooth animations
- Work with animation groups (parallel and sequential)
- Create easing curves for natural motion
- Animate widget properties dynamically
- Build complex animation sequences
- Handle animation state changes

## Introduction

Qt's animation framework provides a powerful system for creating smooth, professional animations. **QPropertyAnimation** animates any Qt property (position, size, opacity, color, etc.), while **QParallelAnimationGroup** and **QSequentialAnimationGroup** let you orchestrate complex animation sequences.

Animations make UIs feel responsive and guide user attention. Qt's framework is declarative - you specify start/end values and duration, and Qt handles the interpolation.

## Key Concepts

### Basic Property Animation

Animate widget position:

```cpp
QPropertyAnimation *animation = new QPropertyAnimation(widget, "pos");
animation->setDuration(1000);  // 1 second
animation->setStartValue(QPoint(0, 0));
animation->setEndValue(QPoint(200, 200));
animation->start();
```

### Easing Curves

Control animation timing:

```cpp
animation->setEasingCurve(QEasingCurve::OutBounce);  // Bouncy
animation->setEasingCurve(QEasingCurve::InOutQuad);  // Smooth acceleration
animation->setEasingCurve(QEasingCurve::Linear);     // Constant speed
```

Common easing curves:
- **Linear** - Constant speed
- **InQuad/OutQuad/InOutQuad** - Quadratic acceleration/deceleration
- **InOutElastic** - Elastic spring effect
- **OutBounce** - Bouncing at the end
- **InOutBack** - Slight overshoot

### Animating Multiple Properties

Animate size and opacity together:

```cpp
QPropertyAnimation *sizeAnim = new QPropertyAnimation(widget, "size");
sizeAnim->setDuration(500);
sizeAnim->setEndValue(QSize(200, 200));

QPropertyAnimation *opacityAnim = new QPropertyAnimation(widget, "windowOpacity");
opacityAnim->setDuration(500);
opacityAnim->setEndValue(0.5);

QParallelAnimationGroup *group = new QParallelAnimationGroup();
group->addAnimation(sizeAnim);
group->addAnimation(opacityAnim);
group->start();
```

### Sequential Animations

Run animations one after another:

```cpp
QSequentialAnimationGroup *sequence = new QSequentialAnimationGroup();

QPropertyAnimation *moveRight = new QPropertyAnimation(widget, "pos");
moveRight->setDuration(500);
moveRight->setEndValue(QPoint(200, 0));

QPropertyAnimation *moveDown = new QPropertyAnimation(widget, "pos");
moveDown->setDuration(500);
moveDown->setEndValue(QPoint(200, 200));

sequence->addAnimation(moveRight);
sequence->addAnimation(moveDown);
sequence->start();
```

### Animation State Signals

React to animation events:

```cpp
connect(animation, &QPropertyAnimation::finished, []() {
    qDebug() << "Animation completed!";
});

connect(animation, &QPropertyAnimation::stateChanged, 
        [](QAbstractAnimation::State newState) {
    if (newState == QAbstractAnimation::Running) {
        qDebug() << "Animation started";
    }
});
```

### Loop Animations

Repeat animations:

```cpp
animation->setLoopCount(-1);  // Infinite loop
animation->setLoopCount(3);   // Loop 3 times
```

## Example Walkthrough

Our example demonstrates:

1. **Position Animation** - Move widget smoothly
2. **Size Animation** - Grow/shrink with easing
3. **Opacity Animation** - Fade in/out effects
4. **Parallel Group** - Multiple properties at once
5. **Sequential Group** - Step-by-step animation sequence
6. **Different Easing Curves** - Visual comparison

The application shows various animation types with interactive controls.

## Expected Output

A window with:
- Animated button that moves, grows, and fades
- Controls to trigger different animation types
- Easing curve selector
- Visual feedback of animation state
- Sequential animation demonstration

## Try It

1. Build and run the application
2. Click "Animate Position" to see smooth movement
3. Click "Animate Size" to see growth/shrinking
4. Click "Fade Animation" to see opacity changes
5. Try "Parallel Animation" for combined effects
6. Test "Sequential Animation" for step-by-step motion
7. Change easing curves and observe the difference

## Key Takeaways

- **QPropertyAnimation** animates any Q_PROPERTY
- Duration is in milliseconds (1000 = 1 second)
- **Easing curves** control acceleration/deceleration
- **QParallelAnimationGroup** runs animations simultaneously
- **QSequentialAnimationGroup** chains animations
- Use `setStartValue()` or let Qt use current value
- Animations emit `finished()` and `stateChanged()` signals
- Set `setLoopCount(-1)` for infinite loops
- Delete animations or set `deleteWhenStopped` property
- Smooth animations are 300-500ms, longer feels slow
