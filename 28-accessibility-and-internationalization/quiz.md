# Lesson 28 Quiz: Accessibility and Internationalization

1. What's the difference between tr() and QCoreApplication::translate(), and when would you use each?

2. What's wrong with this approach to translations?
   ```cpp
   QString msg;
   if (language == "English") {
       msg = "Save file?";
   } else if (language == "Spanish") {
       msg = "¿Guardar archivo?";
   }
   ```

3. How do you handle plural forms correctly in Qt for languages with complex plural rules (like Polish or Arabic)?

4. What Qt tools are used in the translation workflow, and in what order?

5. What accessibility property should you set on a custom icon-only button so screen readers can describe it?

6. How does Qt automatically handle right-to-left (RTL) languages like Arabic or Hebrew?

7. What's the correct way to format a currency value according to the user's locale?

8. Why is keyboard navigation important for accessibility, and how do you ensure proper tab order in Qt?

9. If you change languages at runtime using QTranslator, what must you do to update all UI elements?

10. What's the difference between accessibility and internationalization, and why are both important?
