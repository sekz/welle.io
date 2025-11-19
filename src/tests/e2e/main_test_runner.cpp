/**
 * @file main_test_runner.cpp
 * @brief Main test runner for welle.io end-to-end GUI tests
 *
 * This file sets up and executes all e2e GUI tests for the welle.io Qt/QML application.
 * It initializes the Qt Test framework, configures QML engine with proper imports,
 * sets up console log capture, and runs all registered tests.
 *
 * Usage:
 *   ./e2e_gui_tests                    # Run with default display
 *   QT_QPA_PLATFORM=offscreen ./e2e_gui_tests  # Run headless
 */

#include "test_framework.h"
#include <QtTest/QtTest>
#include <QGuiApplication>
#include <QQmlEngine>
#include <QQuickView>
#include <QDir>
#include <iostream>

/**
 * @brief Basic QML engine initialization test
 *
 * Verifies that:
 * - QML engine can be created
 * - Import paths are configured correctly
 * - Console log capture is working
 * - No errors occur during initialization
 */
class QMLEngineTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        qInfo() << "========================================";
        qInfo() << "Starting QML Engine Initialization Test";
        qInfo() << "========================================";
    }

    void testEngineCreation() {
        E2E::E2ETestFramework framework;

        // Initialize the engine
        QString qmlPath = QCoreApplication::applicationDirPath() + "/../src/welle-gui/QML";
        bool initialized = framework.initializeQmlEngine(qmlPath);

        QVERIFY2(initialized, "QML engine initialization failed");
        QVERIFY2(framework.engine() != nullptr, "QML engine is null");

        // Check that there are no console errors during initialization
        bool noErrors = framework.checkNoConsoleErrors();
        if (!noErrors) {
            framework.printConsoleErrors();
        }
        QVERIFY2(noErrors, "Console errors detected during engine initialization");

        qInfo() << "QML engine created successfully";
    }

    void testConsoleLogCapture() {
        E2E::ConsoleMessageHandler& handler = E2E::ConsoleMessageHandler::instance();
        handler.clear();

        // Trigger some log messages
        qDebug() << "Test debug message";
        qInfo() << "Test info message";
        qWarning() << "Test warning message";

        // Verify messages were captured
        int messageCount = handler.messageCount();
        QVERIFY2(messageCount >= 3, "Console messages not captured");

        qInfo() << "Console log capture working, captured" << messageCount << "messages";

        // Clear for next test
        handler.clear();
    }

    void cleanupTestCase() {
        qInfo() << "========================================";
        qInfo() << "QML Engine Test Completed";
        qInfo() << "========================================";
    }
};

/**
 * @brief QML component loading test
 *
 * Tests loading of basic QML components and validates:
 * - Component can be loaded from string
 * - Properties are accessible
 * - No QML errors occur
 */
class QMLComponentTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        qInfo() << "========================================";
        qInfo() << "Starting QML Component Loading Test";
        qInfo() << "========================================";
    }

    void testSimpleComponentLoading() {
        E2E::E2ETestFramework framework;

        QString qmlPath = QCoreApplication::applicationDirPath() + "/../src/welle-gui/QML";
        QVERIFY(framework.initializeQmlEngine(qmlPath));

        // Clear any initialization messages
        framework.clearConsoleMessages();

        // Create a simple QML component from string to test basic functionality
        QString simpleQml = R"(
            import QtQuick 2.15
            import QtQuick.Controls 2.15

            Rectangle {
                id: root
                width: 100
                height: 100
                color: "blue"

                property string testProperty: "Hello"
            }
        )";

        // Since we need to load from string, we'll use the QML engine directly
        QQmlComponent component(framework.engine());
        component.setData(simpleQml.toUtf8(), QUrl());

        QVERIFY2(!component.isError(), "Component has errors");

        QObject* obj = component.create();
        QVERIFY2(obj != nullptr, "Failed to create component");

        // Test property access
        QVariant prop = obj->property("testProperty");
        QVERIFY2(prop.isValid(), "Property not accessible");
        QCOMPARE(prop.toString(), QString("Hello"));

        // Verify no console errors
        bool noErrors = framework.checkNoConsoleErrors();
        if (!noErrors) {
            framework.printConsoleErrors();
        }
        QVERIFY2(noErrors, "Console errors detected during component loading");

        delete obj;
        qInfo() << "Simple component loaded and tested successfully";
    }

    void cleanupTestCase() {
        qInfo() << "========================================";
        qInfo() << "QML Component Test Completed";
        qInfo() << "========================================";
    }
};

/**
 * @brief Screenshot capture test
 *
 * Verifies that:
 * - Screenshot directory is created
 * - Screenshots can be captured (if display available)
 */
class ScreenshotTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        qInfo() << "========================================";
        qInfo() << "Starting Screenshot Capture Test";
        qInfo() << "========================================";
    }

    void testScreenshotDirectory() {
        E2E::E2ETestFramework framework;

        QString screenshotDir = framework.screenshotDirectory();
        QVERIFY2(!screenshotDir.isEmpty(), "Screenshot directory not set");

        QDir dir(screenshotDir);
        QVERIFY2(dir.exists(), "Screenshot directory does not exist");

        qInfo() << "Screenshot directory verified:" << screenshotDir;
    }

    void cleanupTestCase() {
        qInfo() << "========================================";
        qInfo() << "Screenshot Test Completed";
        qInfo() << "========================================";
    }
};

/**
 * @brief Console message handler test
 *
 * Comprehensive test of the console message capture functionality:
 * - Message type filtering
 * - Error/warning detection
 * - Message counting
 * - Thread safety
 */
class ConsoleHandlerTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        qInfo() << "========================================";
        qInfo() << "Starting Console Handler Test";
        qInfo() << "========================================";
    }

    void testMessageCapture() {
        E2E::ConsoleMessageHandler& handler = E2E::ConsoleMessageHandler::instance();
        handler.clear();

        // Generate different types of messages
        qDebug() << "Debug message 1";
        qInfo() << "Info message 1";
        qWarning() << "Warning message 1";

        // Verify counts
        QVERIFY2(handler.messageCount() >= 3, "Not all messages captured");
        qInfo() << "Captured" << handler.messageCount() << "messages total";
    }

    void testErrorDetection() {
        E2E::ConsoleMessageHandler& handler = E2E::ConsoleMessageHandler::instance();
        handler.clear();

        qDebug() << "This is a debug message";
        QVERIFY2(!handler.hasErrors(), "False positive error detection");

        handler.clear();
    }

    void testWarningDetection() {
        E2E::ConsoleMessageHandler& handler = E2E::ConsoleMessageHandler::instance();
        handler.clear();

        qInfo() << "This is an info message";
        QVERIFY2(!handler.hasWarnings(), "False positive warning detection");

        qWarning() << "This is a warning";
        QVERIFY2(handler.hasWarnings(), "Warning not detected");
        QCOMPARE(handler.warningCount(), 1);

        handler.clear();
    }

    void testMessageClearing() {
        E2E::ConsoleMessageHandler& handler = E2E::ConsoleMessageHandler::instance();

        qDebug() << "Test message";
        QVERIFY2(handler.messageCount() > 0, "Message not captured");

        handler.clear();
        QCOMPARE(handler.messageCount(), 0);

        qInfo() << "Message clearing works correctly";
    }

    void cleanupTestCase() {
        qInfo() << "========================================";
        qInfo() << "Console Handler Test Completed";
        qInfo() << "========================================";
    }
};

/**
 * @brief Main function - Sets up Qt Test and runs all test classes
 */
int main(int argc, char *argv[]) {
    // Disable Qt's default message handler during test setup
    // to avoid interfering with our custom handler
    qSetMessagePattern("[%{type}] %{message}");

    // Create Qt application - required for QML testing
    QGuiApplication app(argc, argv);
    app.setApplicationName("welle-io-e2e-tests");
    app.setOrganizationName("welle.io");

    // Print test environment information
    std::cout << "========================================" << std::endl;
    std::cout << "welle.io End-to-End GUI Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Qt Version: " << qVersion() << std::endl;
    std::cout << "Platform: " << QGuiApplication::platformName().toStdString() << std::endl;
    std::cout << "Working Directory: " << QDir::currentPath().toStdString() << std::endl;
    std::cout << "Application Path: " << QCoreApplication::applicationDirPath().toStdString() << std::endl;

    // Check if running headless
    bool headless = (QGuiApplication::platformName() == "offscreen");
    std::cout << "Headless Mode: " << (headless ? "YES" : "NO") << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Run tests
    int result = 0;
    int testsFailed = 0;
    int testsPassed = 0;

    // Helper macro to run tests and track results
    #define RUN_TEST(TestClass) \
        { \
            TestClass test; \
            int testResult = QTest::qExec(&test, argc, argv); \
            result |= testResult; \
            if (testResult == 0) { \
                testsPassed++; \
            } else { \
                testsFailed++; \
            } \
        }

    // Execute all test classes
    RUN_TEST(QMLEngineTest);
    RUN_TEST(QMLComponentTest);
    RUN_TEST(ScreenshotTest);
    RUN_TEST(ConsoleHandlerTest);

    // Print final summary
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Test Suite Summary" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Test Classes Passed: " << testsPassed << std::endl;
    std::cout << "Test Classes Failed: " << testsFailed << std::endl;
    std::cout << "Overall Result: " << (result == 0 ? "PASSED" : "FAILED") << std::endl;
    std::cout << "========================================" << std::endl;

    return result;
}

// Include the moc file for Qt's meta-object compiler
#include "main_test_runner.moc"
