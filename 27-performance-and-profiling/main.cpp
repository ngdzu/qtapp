#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QElapsedTimer>
#include <QTabWidget>
#include <QProgressBar>
#include <QVector>
#include <QHash>

class PerformanceWidget : public QWidget
{
    Q_OBJECT
public:
    PerformanceWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setupUI();
    }

private:
    void setupUI()
    {
        setWindowTitle("Lesson 27: Performance and Profiling");
        resize(800, 600);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        QLabel *title = new QLabel("<h2>Qt Performance Benchmarks</h2>");
        title->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(title);

        QTabWidget *tabs = new QTabWidget();

        // String Performance Tab
        QWidget *stringTab = new QWidget();
        QVBoxLayout *stringLayout = new QVBoxLayout(stringTab);
        stringResults = new QTextEdit();
        stringResults->setReadOnly(true);
        stringLayout->addWidget(stringResults);
        QPushButton *runStringBtn = new QPushButton("Run String Benchmarks");
        connect(runStringBtn, &QPushButton::clicked, this, &PerformanceWidget::runStringBenchmarks);
        stringLayout->addWidget(runStringBtn);
        tabs->addTab(stringTab, "String Performance");

        // Container Performance Tab
        QWidget *containerTab = new QWidget();
        QVBoxLayout *containerLayout = new QVBoxLayout(containerTab);
        containerResults = new QTextEdit();
        containerResults->setReadOnly(true);
        containerLayout->addWidget(containerResults);
        QPushButton *runContainerBtn = new QPushButton("Run Container Benchmarks");
        connect(runContainerBtn, &QPushButton::clicked, this, &PerformanceWidget::runContainerBenchmarks);
        containerLayout->addWidget(runContainerBtn);
        tabs->addTab(containerTab, "Container Performance");

        // Rendering Performance Tab
        QWidget *renderTab = new QWidget();
        QVBoxLayout *renderLayout = new QVBoxLayout(renderTab);
        renderResults = new QTextEdit();
        renderResults->setReadOnly(true);
        renderLayout->addWidget(renderResults);
        QPushButton *runRenderBtn = new QPushButton("Run Rendering Benchmarks");
        connect(runRenderBtn, &QPushButton::clicked, this, &PerformanceWidget::runRenderBenchmarks);
        renderLayout->addWidget(runRenderBtn);
        tabs->addTab(renderTab, "Rendering Performance");

        // Memory Tab
        QWidget *memoryTab = new QWidget();
        QVBoxLayout *memoryLayout = new QVBoxLayout(memoryTab);
        memoryResults = new QTextEdit();
        memoryResults->setReadOnly(true);
        memoryLayout->addWidget(memoryResults);
        QPushButton *runMemoryBtn = new QPushButton("Run Memory Tests");
        connect(runMemoryBtn, &QPushButton::clicked, this, &PerformanceWidget::runMemoryTests);
        memoryLayout->addWidget(runMemoryBtn);
        tabs->addTab(memoryTab, "Memory Management");

        mainLayout->addWidget(tabs);

        QLabel *note = new QLabel("<i>Note: Results vary by CPU. Run in Release mode for realistic numbers.</i>");
        note->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(note);
    }

    void runStringBenchmarks()
    {
        stringResults->clear();
        stringResults->append("<b>String Performance Benchmarks</b><br>");

        const int iterations = 10000;
        QElapsedTimer timer;

        // Benchmark 1: String construction
        timer.start();
        for (int i = 0; i < iterations; ++i)
        {
            QString s = QString::number(i);
            Q_UNUSED(s);
        }
        qint64 constructionTime = timer.elapsed();

        // Benchmark 2: String concatenation without reserve
        timer.start();
        QString result1;
        for (int i = 0; i < iterations; ++i)
        {
            result1 += QString::number(i);
        }
        qint64 concatNoReserve = timer.elapsed();

        // Benchmark 3: String concatenation with reserve
        timer.start();
        QString result2;
        result2.reserve(iterations * 6); // Estimate
        for (int i = 0; i < iterations; ++i)
        {
            result2 += QString::number(i);
        }
        qint64 concatWithReserve = timer.elapsed();

        // Benchmark 4: QStringList join
        timer.start();
        QStringList parts;
        parts.reserve(iterations);
        for (int i = 0; i < iterations; ++i)
        {
            parts << QString::number(i);
        }
        QString result3 = parts.join("");
        qint64 joinTime = timer.elapsed();

        // Display results
        stringResults->append(QString("<b>1. QString::number() × %1:</b>").arg(iterations));
        stringResults->append(QString("   Time: %1 ms").arg(constructionTime));
        stringResults->append(QString("   Per operation: %1 μs").arg(constructionTime * 1000.0 / iterations, 0, 'f', 2));

        stringResults->append("<br><b>2. Concatenation without reserve():</b>");
        stringResults->append(QString("   Time: %1 ms").arg(concatNoReserve));
        stringResults->append(QString("   Per operation: %1 μs").arg(concatNoReserve * 1000.0 / iterations, 0, 'f', 2));

        stringResults->append("<br><b>3. Concatenation with reserve():</b>");
        stringResults->append(QString("   Time: %1 ms").arg(concatWithReserve));
        stringResults->append(QString("   Per operation: %1 μs").arg(concatWithReserve * 1000.0 / iterations, 0, 'f', 2));
        stringResults->append(QString("   <span style='color: green;'>Speedup: %1x faster</span>")
                                  .arg(double(concatNoReserve) / concatWithReserve, 0, 'f', 2));

        stringResults->append("<br><b>4. QStringList join():</b>");
        stringResults->append(QString("   Time: %1 ms").arg(joinTime));
        stringResults->append(QString("   Per operation: %1 μs").arg(joinTime * 1000.0 / iterations, 0, 'f', 2));

        stringResults->append("<br><b>Summary:</b>");
        stringResults->append("• reserve() dramatically improves concatenation performance");
        stringResults->append("• QStringList::join() is often the fastest for building large strings");
        stringResults->append("• Avoid repeated concatenation in hot loops");
    }

    void runContainerBenchmarks()
    {
        containerResults->clear();
        containerResults->append("<b>Container Performance Benchmarks</b><br>");

        const int size = 100000;
        QElapsedTimer timer;

        // Benchmark 1: QVector append without reserve
        timer.start();
        QVector<int> vec1;
        for (int i = 0; i < size; ++i)
        {
            vec1.append(i);
        }
        qint64 appendNoReserve = timer.elapsed();

        // Benchmark 2: QVector append with reserve
        timer.start();
        QVector<int> vec2;
        vec2.reserve(size);
        for (int i = 0; i < size; ++i)
        {
            vec2.append(i);
        }
        qint64 appendWithReserve = timer.elapsed();

        // Benchmark 3: QVector prepend
        timer.start();
        QVector<int> vec3;
        vec3.reserve(1000);
        for (int i = 0; i < 1000; ++i)
        {
            vec3.prepend(i);
        }
        qint64 prependTime = timer.elapsed();

        // Benchmark 4: QHash insert and lookup
        timer.start();
        QHash<int, int> hash;
        for (int i = 0; i < size; ++i)
        {
            hash.insert(i, i * 2);
        }
        qint64 hashInsert = timer.elapsed();

        timer.start();
        int sum = 0;
        for (int i = 0; i < size; ++i)
        {
            sum += hash.value(i);
        }
        qint64 hashLookup = timer.elapsed();

        // Benchmark 5: Implicit sharing (copy-on-write)
        QVector<int> original;
        original.reserve(size);
        for (int i = 0; i < size; ++i)
        {
            original.append(i);
        }

        timer.start();
        QVector<int> copy = original; // Cheap copy
        qint64 copyTime = timer.nsecsElapsed();

        timer.start();
        copy.append(999); // Triggers detachment
        qint64 detachTime = timer.elapsed();

        // Display results
        containerResults->append(QString("<b>1. QVector::append() without reserve (%1 items):</b>").arg(size));
        containerResults->append(QString("   Time: %1 ms").arg(appendNoReserve));

        containerResults->append("<br><b>2. QVector::append() with reserve():</b>");
        containerResults->append(QString("   Time: %1 ms").arg(appendWithReserve));
        containerResults->append(QString("   <span style='color: green;'>Speedup: %1x faster</span>")
                                     .arg(double(appendNoReserve) / appendWithReserve, 0, 'f', 2));

        containerResults->append(QString("<br><b>3. QVector::prepend() (1000 items):</b>"));
        containerResults->append(QString("   Time: %1 ms (slow due to shifting)").arg(prependTime));

        containerResults->append(QString("<br><b>4. QHash insert (%1 items):</b>").arg(size));
        containerResults->append(QString("   Time: %1 ms").arg(hashInsert));

        containerResults->append("<br><b>5. QHash lookup (100k lookups):</b>");
        containerResults->append(QString("   Time: %1 ms").arg(hashLookup));
        containerResults->append(QString("   Per lookup: %1 ns").arg(hashLookup * 1000000.0 / size, 0, 'f', 0));

        containerResults->append(QString("<br><b>6. Copy-on-write (implicit sharing):</b>"));
        containerResults->append(QString("   Copy time: %1 ns (just ref count)").arg(copyTime));
        containerResults->append(QString("   Detach time: %1 ms (deep copy)").arg(detachTime));
        containerResults->append(QString("   <span style='color: green;'>Copy is %1x faster than detach</span>")
                                     .arg(double(detachTime * 1000000) / copyTime, 0, 'f', 0));

        containerResults->append("<br><b>Summary:</b>");
        containerResults->append("• Always reserve() when you know the size");
        containerResults->append("• Prepend is O(n), append is O(1) - prefer append");
        containerResults->append("• QHash lookup is O(1) average, very fast");
        containerResults->append("• Implicit sharing makes copies cheap until modification");
    }

    void runRenderBenchmarks()
    {
        renderResults->clear();
        renderResults->append("<b>Rendering Performance Benchmarks</b><br>");

        const int updates = 100;
        QElapsedTimer timer;

        // Create test label
        QLabel *testLabel = new QLabel(this);
        testLabel->hide(); // Don't actually show it

        // Benchmark 1: Multiple updates with repainting
        timer.start();
        for (int i = 0; i < updates; ++i)
        {
            testLabel->setText(QString::number(i));
            testLabel->update(); // Each triggers repaint event
        }
        qApp->processEvents(); // Process all pending repaints
        qint64 multipleUpdates = timer.elapsed();

        // Benchmark 2: Batch updates with setUpdatesEnabled
        timer.start();
        testLabel->setUpdatesEnabled(false);
        for (int i = 0; i < updates; ++i)
        {
            testLabel->setText(QString::number(i));
        }
        testLabel->setUpdatesEnabled(true);
        qApp->processEvents();
        qint64 batchUpdates = timer.elapsed();

        delete testLabel;

        // Display results
        renderResults->append(QString("<b>1. Multiple update() calls (%1 updates):</b>").arg(updates));
        renderResults->append(QString("   Time: %1 ms").arg(multipleUpdates));
        renderResults->append("   Note: Qt coalesces these automatically");

        renderResults->append("<br><b>2. Batch with setUpdatesEnabled(false):</b>");
        renderResults->append(QString("   Time: %1 ms").arg(batchUpdates));
        if (batchUpdates < multipleUpdates)
        {
            renderResults->append(QString("   <span style='color: green;'>%1x faster</span>")
                                      .arg(double(multipleUpdates) / batchUpdates, 0, 'f', 2));
        }

        renderResults->append("<br><b>Best Practices:</b>");
        renderResults->append("• Qt automatically coalesces update() calls");
        renderResults->append("• Use setUpdatesEnabled(false) for complex multi-widget updates");
        renderResults->append("• Avoid calling update() in tight loops");
        renderResults->append("• One update() per event loop iteration is sufficient");

        renderResults->append("<br><b>Signal/Slot Overhead:</b>");
        renderResults->append("Approximate overhead per call:");
        renderResults->append("• Direct function call: ~5 ns");
        renderResults->append("• Signal/slot (DirectConnection): ~30 ns");
        renderResults->append("• Signal/slot (QueuedConnection): ~500 ns");
        renderResults->append("• Use direct calls only in hot loops (>1M calls/sec)");
    }

    void runMemoryTests()
    {
        memoryResults->clear();
        memoryResults->append("<b>Memory Management Tests</b><br>");

        // Test 1: Parent-child ownership
        memoryResults->append("<b>1. QObject Parent-Child Ownership:</b>");
        memoryResults->append("```cpp");
        memoryResults->append("QWidget *parent = new QWidget();");
        memoryResults->append("QPushButton *btn = new QPushButton(parent);");
        memoryResults->append("delete parent; // btn deleted automatically");
        memoryResults->append("```");
        memoryResults->append("✓ No memory leak - child deleted with parent");

        // Test 2: Memory leak example
        memoryResults->append("<br><b>2. Common Memory Leak:</b>");
        memoryResults->append("```cpp");
        memoryResults->append("QPushButton *btn = new QPushButton(); // No parent!");
        memoryResults->append("// btn never deleted = memory leak");
        memoryResults->append("```");
        memoryResults->append("✗ Memory leak - no parent to delete it");

        // Test 3: Layout ownership
        memoryResults->append("<br><b>3. Layout Ownership:</b>");
        memoryResults->append("```cpp");
        memoryResults->append("QWidget *widget = new QWidget();");
        memoryResults->append("QVBoxLayout *layout = new QVBoxLayout(widget);");
        memoryResults->append("QPushButton *btn = new QPushButton();");
        memoryResults->append("layout->addWidget(btn);");
        memoryResults->append("delete widget; // Deletes layout and btn");
        memoryResults->append("```");
        memoryResults->append("✓ Layout takes ownership when added to widget");

        // Demonstrate actual allocation
        memoryResults->append("<br><b>4. Memory Allocation Test:</b>");

        QElapsedTimer timer;
        timer.start();

        // Allocate many QObjects with proper parenting
        QObject *root = new QObject();
        for (int i = 0; i < 10000; ++i)
        {
            new QObject(root); // All have root as parent
        }

        qint64 allocTime = timer.elapsed();

        timer.start();
        delete root; // Deletes all 10,000 children
        qint64 deleteTime = timer.elapsed();

        memoryResults->append(QString("Allocated 10,000 QObjects: %1 ms").arg(allocTime));
        memoryResults->append(QString("Deleted all via parent: %1 ms").arg(deleteTime));
        memoryResults->append("✓ All memory freed automatically");

        memoryResults->append("<br><b>Profiling Tools:</b>");
        memoryResults->append("• <b>Valgrind</b> (Linux): valgrind --leak-check=full ./app");
        memoryResults->append("• <b>Instruments</b> (macOS): Leaks and Allocations");
        memoryResults->append("• <b>Dr. Memory</b> (Windows/Linux): Similar to Valgrind");
        memoryResults->append("• <b>AddressSanitizer</b>: Compile with -fsanitize=address");

        memoryResults->append("<br><b>Best Practices:</b>");
        memoryResults->append("✓ Always specify parent for QObjects");
        memoryResults->append("✓ Use smart pointers for non-QObjects");
        memoryResults->append("✓ Let Qt manage widget lifetimes");
        memoryResults->append("✓ Profile with Valgrind to verify no leaks");
        memoryResults->append("✗ Never use raw new/delete without parent");
    }

    QTextEdit *stringResults;
    QTextEdit *containerResults;
    QTextEdit *renderResults;
    QTextEdit *memoryResults;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    PerformanceWidget window;
    window.show();

    return app.exec();
}

#include "main.moc"
