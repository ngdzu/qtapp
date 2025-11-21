# Lesson 21 Quiz Answers

1. **What is QPropertyAnimation and what can it animate?**

QPropertyAnimation animates any Qt property (Q_PROPERTY) over time.

It can animate position, size, opacity, color, geometry, rotation - any property exposed through Qt's property system. You specify start/end values and duration, and Qt interpolates between them smoothly.

2. **How do you create a 1-second animation that moves a widget?**

```cpp
QPropertyAnimation *anim = new QPropertyAnimation(widget, "pos");
anim->setDuration(1000);  // 1000ms = 1 second
anim->setEndValue(QPoint(200, 200));
anim->start();
```
If you don't set startValue, it uses the current value.

3. **What does this code do?**

Sets the animation timing curve to bounce at the end.

OutBounce makes the animation overshoot the target and bounce back, creating a playful, elastic effect. Different easing curves create different motion feels (linear, smooth, elastic, bouncy).

4. **What's the difference between QParallelAnimationGroup and QSequentialAnimationGroup?**

**QParallelAnimationGroup** runs all animations simultaneously (at the same time). **QSequentialAnimationGroup** runs them one after another.

Parallel is for combined effects (move + fade together). Sequential is for step-by-step choreography (move right, then down, then left).

5. **How do you make an animation loop infinitely?**

```cpp
animation->setLoopCount(-1);  // -1 = infinite
```
Use positive numbers for finite loops: `setLoopCount(3)` loops 3 times.

6. **What signal is emitted when an animation completes?**

The `finished()` signal:
```cpp
connect(animation, &QPropertyAnimation::finished, []() {
    qDebug() << "Animation done!";
});
```

7. **How do you animate multiple properties simultaneously?**

Create separate animations and add to QParallelAnimationGroup:
```cpp
QParallelAnimationGroup *group = new QParallelAnimationGroup();
group->addAnimation(posAnimation);
group->addAnimation(sizeAnimation);
group->start();
```

8. **What does `setKeyValueAt(0.5, value)` do in an animation?**

Sets an intermediate value at the 50% point of the animation.

The animation will interpolate: start → 50% (key value) → end. Useful for animations that need to pass through specific values (e.g., fade out then back in).

9. **What's a good duration for smooth UI animations?**

300-500ms for most UI animations.

Shorter (200-300ms) feels snappy. Longer (500-800ms) is noticeable but not slow. Over 1 second feels sluggish. Match duration to distance traveled - longer movements can take more time.

10. **How do you prevent memory leaks when creating animations dynamically?**

Use `QAbstractAnimation::DeleteWhenStopped`:
```cpp
animation->start(QAbstractAnimation::DeleteWhenStopped);
```
The animation deletes itself automatically when finished. Otherwise, manually delete or parent it to a widget.
