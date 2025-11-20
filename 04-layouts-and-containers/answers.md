# Answers: Layouts and Containers

## Question 1
**What is the main advantage of using Qt layouts over manual widget positioning with `setGeometry()`?**

**Answer:** Layouts automatically handle widget positioning and resizing when the window size changes.

**Explanation:** Manual positioning with `setGeometry()` requires you to specify exact pixel coordinates, which don't adapt when users resize windows or use different screen sizes. Layouts automatically reposition and resize widgets to maintain proper spacing and proportions, making your UI responsive and easier to maintain across different environments.

## Question 2
**Which layout would you use to arrange three buttons side-by-side horizontally?**

**Answer:** QHBoxLayout

**Explanation:** QHBoxLayout arranges widgets in a horizontal row from left to right. This is the natural choice for side-by-side button arrangements like "OK/Cancel" dialogs or toolbar buttons. QVBoxLayout would stack them vertically, and QGridLayout would be overkill for a simple horizontal arrangement.

## Question 3
**What does the following code create?**

```cpp
QGridLayout *layout = new QGridLayout();
layout->addWidget(new QLabel("Username:"), 0, 0);
layout->addWidget(new QLineEdit(), 0, 1);
layout->addWidget(new QLabel("Password:"), 1, 0);
layout->addWidget(new QLineEdit(), 1, 1);
```

**Answer:** A 2x2 grid with labels in the left column and input fields in the right column.

**Explanation:** QGridLayout's `addWidget(widget, row, col)` places widgets in grid cells. Row 0 has "Username:" label and text field, row 1 has "Password:" label and text field. This creates a typical form layout with aligned labels and inputs. This is a common pattern for login forms and settings dialogs.

## Question 4
**Spot the bug in this code:**

```cpp
QWidget *window = new QWidget();
QVBoxLayout *layout = new QVBoxLayout();
layout->addWidget(new QPushButton("Button 1"));
layout->addWidget(new QPushButton("Button 2"));
window->show();
```

**Answer:** The layout is never applied to the window.

**Explanation:** Creating a layout and adding widgets to it doesn't automatically associate it with the window. You must call `window->setLayout(layout)` to apply the layout to the widget. Without this, the buttons won't be displayed because they're not parented to the window.

**Corrected code:**
```cpp
QWidget *window = new QWidget();
QVBoxLayout *layout = new QVBoxLayout();
layout->addWidget(new QPushButton("Button 1"));
layout->addWidget(new QPushButton("Button 2"));
window->setLayout(layout);  // Apply layout to window
window->show();
```

## Question 5
**Fill in the missing line to create a button that spans 2 columns in a grid layout:**

```cpp
QGridLayout *layout = new QGridLayout();
QPushButton *wideButton = new QPushButton("Wide Button");
// Missing line here - should place button at row 0, column 0, spanning 1 row and 2 columns
```

**Answer:**
```cpp
layout->addWidget(wideButton, 0, 0, 1, 2);
```

**Explanation:** The full signature is `addWidget(widget, row, column, rowSpan, columnSpan)`. The parameters (0, 0, 1, 2) mean: place at row 0, column 0, span 1 row, span 2 columns. This is useful for buttons like "Cancel" or "Submit" that need to span multiple columns in a form layout.

## Question 6
**What is the purpose of size policies in Qt layouts?**

**Answer:** Size policies control how widgets grow and shrink within layouts when space is available or limited.

**Explanation:** Each widget has horizontal and vertical size policies (e.g., Fixed, Minimum, Expanding, Preferred) that tell the layout manager how the widget should behave during resizing. For example, a text field might have an Expanding policy to use available space, while a button might have a Fixed policy to maintain its size. This gives you fine-grained control over how your UI adapts to different window sizes.

## Question 7
**If you want to add empty space that expands to push widgets apart in a layout, what would you use?**

**Answer:** Use `addStretch()` or `addSpacing()`.

**Explanation:** `addStretch()` adds flexible space that expands to fill available room, pushing widgets to opposite ends of a layout. For example, in a horizontal layout with buttons, `addStretch()` between them creates flexible spacing. `addSpacing(pixels)` adds fixed-size spacing. These are useful for creating margins, separating groups of widgets, or aligning widgets to one side of a layout.

## Question 8
**What happens when you call `layout->setSpacing(20)` on a QHBoxLayout?**

**Answer:** It sets 20 pixels of space between each widget in the layout.

**Explanation:** The spacing value controls the gap between adjacent widgets managed by the layout. Setting it to 20 means there will be 20 pixels between each button, label, or other widget in the horizontal layout. The default spacing is usually 6 pixels. This is different from margins (controlled by `setContentsMargins()`), which add space around the edges of the entire layout.

## Question 9
**Why would you use `QGroupBox` when working with layouts?**

**Answer:** QGroupBox provides a titled frame that visually groups related widgets together.

**Explanation:** QGroupBox creates a visual container with a border and optional title, making it clear which widgets belong together. It's commonly used to organize forms into logical sections (e.g., "Personal Information", "Contact Details") and improves UI readability. As shown in the lesson example, each layout demo is wrapped in a QGroupBox to clearly separate the horizontal, vertical, and grid layout demonstrations.

## Question 10
**How would you use layouts to create a typical login form with username/password fields and a submit button? Describe which layouts you'd use and how you'd organize them.**

**Answer:** Use a QGridLayout for label-field pairs and a QVBoxLayout to add the submit button below.

**Explanation:** The most elegant approach uses a QGridLayout for the form fields with labels in column 0 and input fields in column 1. This automatically aligns labels and creates a clean form appearance. Then either add the submit button to the grid below the fields (spanning both columns), or wrap the grid in a QVBoxLayout and add the button at the bottom. For example:

```cpp
QGridLayout *formLayout = new QGridLayout();
formLayout->addWidget(new QLabel("Username:"), 0, 0);
formLayout->addWidget(new QLineEdit(), 0, 1);
formLayout->addWidget(new QLabel("Password:"), 1, 0);
formLayout->addWidget(new QLineEdit(), 1, 1);

QPushButton *submitBtn = new QPushButton("Login");
formLayout->addWidget(submitBtn, 2, 0, 1, 2);  // Span both columns
```

This pattern is used in countless Qt applications and provides a professional, maintainable form layout.
