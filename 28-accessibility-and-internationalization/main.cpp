#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTranslator>
#include <QLocale>
#include <QTextEdit>
#include <QTabWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QDateTime>
#include <QLineEdit>

class I18nAccessibilityWidget : public QWidget
{
    Q_OBJECT
public:
    I18nAccessibilityWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setupUI();
        updateContent();
    }

protected:
    void changeEvent(QEvent *event) override
    {
        if (event->type() == QEvent::LanguageChange)
        {
            retranslateUi();
        }
        QWidget::changeEvent(event);
    }

private:
    void setupUI()
    {
        setWindowTitle(tr("Lesson 28: Accessibility and Internationalization"));
        resize(900, 700);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Title
        titleLabel = new QLabel();
        titleLabel->setAlignment(Qt::AlignCenter);
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(16);
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);
        titleLabel->setAccessibleName(tr("Main Title"));
        mainLayout->addWidget(titleLabel);

        // Language selection
        QGroupBox *langGroup = new QGroupBox();
        QHBoxLayout *langLayout = new QHBoxLayout(langGroup);

        langLabel = new QLabel();
        langLayout->addWidget(langLabel);

        englishBtn = new QPushButton("English");
        englishBtn->setAccessibleName(tr("Switch to English"));
        englishBtn->setAccessibleDescription(tr("Changes the application language to English"));
        connect(englishBtn, &QPushButton::clicked, this, &I18nAccessibilityWidget::switchToEnglish);
        langLayout->addWidget(englishBtn);

        spanishBtn = new QPushButton("Español");
        spanishBtn->setAccessibleName(tr("Switch to Spanish"));
        spanishBtn->setAccessibleDescription(tr("Changes the application language to Spanish"));
        connect(spanishBtn, &QPushButton::clicked, this, &I18nAccessibilityWidget::switchToSpanish);
        langLayout->addWidget(spanishBtn);

        frenchBtn = new QPushButton("Français");
        frenchBtn->setAccessibleName(tr("Switch to French"));
        frenchBtn->setAccessibleDescription(tr("Changes the application language to French"));
        connect(frenchBtn, &QPushButton::clicked, this, &I18nAccessibilityWidget::switchToFrench);
        langLayout->addWidget(frenchBtn);

        langLayout->addStretch();
        mainLayout->addWidget(langGroup);

        // Tab widget
        QTabWidget *tabs = new QTabWidget();
        tabs->setAccessibleName(tr("Main Content Tabs"));

        // Translation Demo Tab
        QWidget *translationTab = new QWidget();
        QVBoxLayout *translationLayout = new QVBoxLayout(translationTab);
        translationInfo = new QTextEdit();
        translationInfo->setReadOnly(true);
        translationInfo->setAccessibleName(tr("Translation Information"));
        translationLayout->addWidget(translationInfo);
        tabs->addTab(translationTab, tr("Translation Demo"));

        // Locale Formatting Tab
        QWidget *localeTab = new QWidget();
        QVBoxLayout *localeLayout = new QVBoxLayout(localeTab);
        localeInfo = new QTextEdit();
        localeInfo->setReadOnly(true);
        localeInfo->setAccessibleName(tr("Locale Formatting Information"));
        localeLayout->addWidget(localeInfo);
        tabs->addTab(localeTab, tr("Locale Formatting"));

        // Accessibility Tab
        QWidget *accessTab = new QWidget();
        QVBoxLayout *accessLayout = new QVBoxLayout(accessTab);
        accessibilityInfo = new QTextEdit();
        accessibilityInfo->setReadOnly(true);
        accessibilityInfo->setAccessibleName(tr("Accessibility Features Information"));
        accessLayout->addWidget(accessibilityInfo);
        tabs->addTab(accessTab, tr("Accessibility Features"));

        // RTL Demo Tab
        QWidget *rtlTab = new QWidget();
        QVBoxLayout *rtlLayout = new QVBoxLayout(rtlTab);
        rtlInfo = new QTextEdit();
        rtlInfo->setReadOnly(true);
        rtlInfo->setAccessibleName(tr("Right-to-Left Layout Information"));
        rtlLayout->addWidget(rtlInfo);

        rtlToggleBtn = new QPushButton();
        rtlToggleBtn->setAccessibleName(tr("Toggle RTL Layout"));
        rtlToggleBtn->setAccessibleDescription(tr("Switches between left-to-right and right-to-left layout"));
        connect(rtlToggleBtn, &QPushButton::clicked, this, &I18nAccessibilityWidget::toggleRTL);
        rtlLayout->addWidget(rtlToggleBtn);

        tabs->addTab(rtlTab, tr("RTL Support"));

        mainLayout->addWidget(tabs);

        // Set tab order for keyboard navigation
        setTabOrder(englishBtn, spanishBtn);
        setTabOrder(spanishBtn, frenchBtn);
        setTabOrder(frenchBtn, tabs);

        currentLanguage = "en";
        retranslateUi();
    }

    void retranslateUi()
    {
        setWindowTitle(tr("Lesson 28: Accessibility and Internationalization"));
        titleLabel->setText(tr("<h2>Accessibility and Internationalization Demo</h2>"));
        langLabel->setText(tr("Select Language:"));
        rtlToggleBtn->setText(tr("Toggle RTL Layout"));

        updateContent();
    }

    void updateContent()
    {
        updateTranslationInfo();
        updateLocaleInfo();
        updateAccessibilityInfo();
        updateRTLInfo();
    }

    void updateTranslationInfo()
    {
        translationInfo->clear();
        translationInfo->append(tr("<b>Translation System Demonstration</b><br>"));

        translationInfo->append(tr("Current Language: %1").arg(getCurrentLanguageName()));
        translationInfo->append("");

        translationInfo->append(tr("<b>Sample Translated Strings:</b>"));
        translationInfo->append(tr("Welcome to the application!"));
        translationInfo->append(tr("File saved successfully."));
        translationInfo->append(tr("Are you sure you want to quit?"));
        translationInfo->append(tr("Settings have been updated."));

        translationInfo->append("");
        translationInfo->append(tr("<b>Plural Forms:</b>"));
        for (int count : {0, 1, 2, 5, 21})
        {
            translationInfo->append(tr("You have %n item(s)", "", count));
        }

        translationInfo->append("");
        translationInfo->append(tr("<b>How tr() Works:</b>"));
        translationInfo->append(tr("1. Developer wraps strings in tr()"));
        translationInfo->append(tr("2. lupdate extracts to .ts files"));
        translationInfo->append(tr("3. Translator translates in Qt Linguist"));
        translationInfo->append(tr("4. lrelease compiles to .qm files"));
        translationInfo->append(tr("5. QTranslator loads at runtime"));
    }

    void updateLocaleInfo()
    {
        localeInfo->clear();

        QLocale locale = getLocaleForLanguage(currentLanguage);

        localeInfo->append(tr("<b>Locale-Specific Formatting</b><br>"));
        localeInfo->append(tr("Locale: %1").arg(locale.name()));
        localeInfo->append("");

        // Date formatting
        QDate today = QDate::currentDate();
        localeInfo->append(tr("<b>Date Formatting:</b>"));
        localeInfo->append(tr("Short format: %1").arg(locale.toString(today, QLocale::ShortFormat)));
        localeInfo->append(tr("Long format: %1").arg(locale.toString(today, QLocale::LongFormat)));

        // Time formatting
        QTime now = QTime::currentTime();
        localeInfo->append("");
        localeInfo->append(tr("<b>Time Formatting:</b>"));
        localeInfo->append(tr("Short format: %1").arg(locale.toString(now, QLocale::ShortFormat)));
        localeInfo->append(tr("Long format: %1").arg(locale.toString(now, QLocale::LongFormat)));

        // Number formatting
        localeInfo->append("");
        localeInfo->append(tr("<b>Number Formatting:</b>"));
        double number = 1234567.89;
        localeInfo->append(tr("Number: %1").arg(locale.toString(number, 'f', 2)));

        // Currency formatting
        localeInfo->append("");
        localeInfo->append(tr("<b>Currency Formatting:</b>"));
        double price = 1234.56;
        localeInfo->append(tr("Price: %1").arg(locale.toCurrencyString(price)));

        // Measurement systems
        localeInfo->append("");
        localeInfo->append(tr("<b>Measurement System:</b>"));
        QString measurement = (locale.measurementSystem() == QLocale::MetricSystem)
                                  ? tr("Metric")
                                  : tr("Imperial");
        localeInfo->append(tr("System: %1").arg(measurement));
    }

    void updateAccessibilityInfo()
    {
        accessibilityInfo->clear();
        accessibilityInfo->append(tr("<b>Accessibility Features</b><br>"));

        accessibilityInfo->append(tr("<b>Accessible Names:</b>"));
        accessibilityInfo->append(tr("• Title: \"%1\"").arg(titleLabel->accessibleName()));
        accessibilityInfo->append(tr("• English button: \"%1\"").arg(englishBtn->accessibleName()));
        accessibilityInfo->append(tr("• Spanish button: \"%1\"").arg(spanishBtn->accessibleName()));

        accessibilityInfo->append("");
        accessibilityInfo->append(tr("<b>Keyboard Navigation:</b>"));
        accessibilityInfo->append(tr("• Tab - Navigate between controls"));
        accessibilityInfo->append(tr("• Shift+Tab - Navigate backwards"));
        accessibilityInfo->append(tr("• Space/Enter - Activate buttons"));
        accessibilityInfo->append(tr("• Arrow keys - Navigate within widgets"));

        accessibilityInfo->append("");
        accessibilityInfo->append(tr("<b>Screen Reader Support:</b>"));
        accessibilityInfo->append(tr("All buttons have accessible names and descriptions"));
        accessibilityInfo->append(tr("Screen readers (NVDA, JAWS, VoiceOver) can announce:"));
        accessibilityInfo->append(tr("• Widget type (button, text edit, etc.)"));
        accessibilityInfo->append(tr("• Widget name and description"));
        accessibilityInfo->append(tr("• Current state and value"));

        accessibilityInfo->append("");
        accessibilityInfo->append(tr("<b>Focus Policy:</b>"));
        accessibilityInfo->append(tr("Buttons: StrongFocus (Tab + Click)"));
        accessibilityInfo->append(tr("Labels: NoFocus (not interactive)"));
        accessibilityInfo->append(tr("Text edits: StrongFocus (Tab + Click)"));

        accessibilityInfo->append("");
        accessibilityInfo->append(tr("<b>Best Practices:</b>"));
        accessibilityInfo->append(tr("✓ All interactive elements keyboard accessible"));
        accessibilityInfo->append(tr("✓ Logical tab order set with setTabOrder()"));
        accessibilityInfo->append(tr("✓ Icon-only buttons have accessible names"));
        accessibilityInfo->append(tr("✓ Tooltips for visual users"));
        accessibilityInfo->append(tr("✓ High contrast support (automatic)"));
    }

    void updateRTLInfo()
    {
        rtlInfo->clear();
        rtlInfo->append(tr("<b>Right-to-Left (RTL) Language Support</b><br>"));

        bool isRTL = layoutDirection() == Qt::RightToLeft;
        rtlInfo->append(tr("Current Layout Direction: %1")
                            .arg(isRTL ? tr("Right-to-Left") : tr("Left-to-Right")));

        rtlInfo->append("");
        rtlInfo->append(tr("<b>RTL Languages:</b>"));
        rtlInfo->append(tr("• Arabic (العربية)"));
        rtlInfo->append(tr("• Hebrew (עברית)"));
        rtlInfo->append(tr("• Persian (فارسی)"));
        rtlInfo->append(tr("• Urdu (اردو)"));

        rtlInfo->append("");
        rtlInfo->append(tr("<b>What Gets Mirrored in RTL:</b>"));
        rtlInfo->append(tr("✓ Layout direction (right to left)"));
        rtlInfo->append(tr("✓ Text alignment (right-aligned by default)"));
        rtlInfo->append(tr("✓ Widget ordering in layouts"));
        rtlInfo->append(tr("✓ Scrollbars (appear on left)"));
        rtlInfo->append(tr("✓ Tab order (reversed)"));
        rtlInfo->append(tr("✓ Some icons (arrows, etc.)"));

        rtlInfo->append("");
        rtlInfo->append(tr("<b>How Qt Handles RTL:</b>"));
        rtlInfo->append(tr("Qt automatically detects RTL from translation files"));
        rtlInfo->append(tr("Or you can set manually: app.setLayoutDirection()"));
        rtlInfo->append(tr("Layouts automatically reverse widget order"));
        rtlInfo->append(tr("No code changes needed for basic RTL support"));

        rtlInfo->append("");
        rtlInfo->append(tr("<b>Testing RTL:</b>"));
        rtlInfo->append(tr("Click the button below to toggle RTL mode"));
        rtlInfo->append(tr("Notice how the entire UI mirrors"));
    }

    void switchToEnglish()
    {
        if (currentLanguage != "en")
        {
            qApp->removeTranslator(&translator);
            currentLanguage = "en";
            setLayoutDirection(Qt::LeftToRight);
            retranslateUi();
        }
    }

    void switchToSpanish()
    {
        switchLanguage("es");
    }

    void switchToFrench()
    {
        switchLanguage("fr");
    }

    void switchLanguage(const QString &lang)
    {
        if (currentLanguage != lang)
        {
            qApp->removeTranslator(&translator);

            // In a real app, load actual .qm translation files
            // translator.load("myapp_" + lang + ".qm", ":/translations");
            // qApp->installTranslator(&translator);

            currentLanguage = lang;
            retranslateUi();
        }
    }

    void toggleRTL()
    {
        Qt::LayoutDirection newDir = (layoutDirection() == Qt::LeftToRight)
                                         ? Qt::RightToLeft
                                         : Qt::LeftToRight;
        setLayoutDirection(newDir);
        updateRTLInfo();
    }

    QString getCurrentLanguageName()
    {
        if (currentLanguage == "en")
            return tr("English");
        if (currentLanguage == "es")
            return tr("Spanish");
        if (currentLanguage == "fr")
            return tr("French");
        return tr("Unknown");
    }

    QLocale getLocaleForLanguage(const QString &lang)
    {
        if (lang == "es")
            return QLocale(QLocale::Spanish, QLocale::Spain);
        if (lang == "fr")
            return QLocale(QLocale::French, QLocale::France);
        return QLocale(QLocale::English, QLocale::UnitedStates);
    }

    QLabel *titleLabel;
    QLabel *langLabel;
    QPushButton *englishBtn;
    QPushButton *spanishBtn;
    QPushButton *frenchBtn;
    QPushButton *rtlToggleBtn;
    QTextEdit *translationInfo;
    QTextEdit *localeInfo;
    QTextEdit *accessibilityInfo;
    QTextEdit *rtlInfo;

    QTranslator translator;
    QString currentLanguage;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    I18nAccessibilityWidget window;
    window.show();

    return app.exec();
}

#include "main.moc"
