# Lesson 4: Layouts and Containers

## Learning Goals

- Understand Qt's layout management system
- Learn to use QHBoxLayout, QVBoxLayout, and QGridLayout
- Master widget resizing behavior and size policies
- Create flexible, responsive user interfaces

## Introduction

In Lesson 3, we positioned widgets manually using `setGeometry()`. While this works for simple cases, it becomes unwieldy for complex interfaces and doesn't handle window resizing well. Qt's layout system solves these problems by automatically managing widget positions and sizes.

Layouts are invisible container objects that control how widgets are arranged within a parent widget. They respond to window resizing, respect widget size hints, and maintain proper spacing. This makes your UI adaptable to different screen sizes and user preferences.

## The Three Essential Layouts

### QHBoxLayout - Horizontal Layout

`QHBoxLayout` arranges widgets in a single horizontal row from left to right:

```cpp
QHBoxLayout *layout = new QHBoxLayout();
layout->addWidget(new QPushButton("Left"));
layout->addWidget(new QPushButton("Center"));
layout->addWidget(new QPushButton("Right"));
```

Widgets automatically share the available horizontal space. You can control spacing with `setSpacing()` and margins with `setContentsMargins()`.

### QVBoxLayout - Vertical Layout

`QVBoxLayout` arranges widgets in a single vertical column from top to bottom:

```cpp
QVBoxLayout *layout = new QVBoxLayout();
layout->addWidget(new QLabel("Top"));
layout->addWidget(new QLineEdit());
layout->addWidget(new QPushButton("Submit"));
```

This is perfect for forms, lists, or any vertical arrangement of UI elements.

### QGridLayout - Grid Layout

`QGridLayout` arranges widgets in a two-dimensional grid with rows and columns:

```cpp
QGridLayout *layout = new QGridLayout();
layout->addWidget(new QLabel("Name:"), 0, 0);
layout->addWidget(new QLineEdit(), 0, 1);
layout->addWidget(new QLabel("Email:"), 1, 0);
layout->addWidget(new QLineEdit(), 1, 1);
```

Parameters are (widget, row, column). You can also specify row/column spans for widgets that occupy multiple cells.

## Size Policies and Stretch

Widgets have size policies that tell layouts how they should grow or shrink. Common policies include:

- `Fixed`: Widget has a fixed size and won't resize
- `Minimum`: Widget can grow but has a minimum size
- `Maximum`: Widget can shrink but has a maximum size
- `Preferred`: Widget prefers a certain size but can adjust
- `Expanding`: Widget wants as much space as possible

You can also add stretch factors to layouts to control how extra space is distributed among widgets.

## Example Walkthrough

Our example creates a window demonstrating all three layout types. The main window uses a vertical layout containing three sections, each showing a different layout style:

1. A horizontal layout with three buttons that resize proportionally
2. A vertical layout with labels and text fields
3. A grid layout simulating a simple calculator keypad

When you resize the window, you'll see how layouts automatically reposition and resize widgets while maintaining their relative arrangements.

## Expected Output

The application displays a window with three distinct sections:
- Top: Three buttons arranged horizontally that expand to fill width
- Middle: A vertical stack of labels and input fields
- Bottom: A 3x3 grid of number buttons like a calculator keypad

Resizing the window demonstrates how layouts automatically adjust widget positions and sizes while maintaining proper spacing and proportions.

## Try It

1. Build and run the example application
2. Resize the window and observe how widgets adjust automatically
3. Compare this to manual positioning with `setGeometry()` from Lesson 3
4. Modify the code to add stretch factors: `layout->addStretch()`
5. Experiment with different size policies on individual widgets
6. Try nesting layouts: add a horizontal layout inside a vertical layout

## Key Takeaways

- Layouts automatically manage widget positions and handle resizing
- QHBoxLayout arranges widgets horizontally, QVBoxLayout vertically
- QGridLayout creates two-dimensional grids for complex arrangements
- Size policies control how widgets grow and shrink within layouts
- Layouts can be nested to create sophisticated UI structures
- Using layouts makes UIs responsive and easier to maintain than manual positioning
