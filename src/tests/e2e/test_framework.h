/*
 *    Copyright (C) 2024
 *    welle.io E2E GUI Testing Framework
 *
 *    This file is part of the welle.io E2E testing suite.
 *
 *    welle.io is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <QtTest>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QLoggingCategory>
#include <QtDebug>
#include <QMutex>
#include <QDateTime>
#include <QQmlApplicationEngine>
#include <memory>
#include <vector>

/**
 * @brief Base class for QML component testing
 *
 * Provides utilities for:
 * - Loading QML components in isolation
 * - Capturing console output and detecting errors/warnings
 * - Verifying component properties and signals
 * - Testing visual rendering
 * - Checking interaction handling
 *
 * THREAD SAFETY WARNING:
 * This class uses a static message handler without mutex protection.
 * All tests using this class MUST run in a single thread and MUST NOT
 * run concurrently. The messageHandler function is NOT thread-safe.
 */
class QMLComponentTestBase : public QObject
{
    Q_OBJECT

public:
    virtual ~QMLComponentTestBase() {}

protected:
    std::unique_ptr<QQmlEngine> engine;
    std::unique_ptr<QQmlComponent> component;
    QQuickItem* rootItem = nullptr;
    QStringList consoleMessages;
    QStringList consoleWarnings;
    QStringList consoleErrors;

    // Console message handler
    static QtMessageHandler originalHandler;
    static QMLComponentTestBase* currentTest;

    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        if (currentTest) {
            switch (type) {
            case QtDebugMsg:
                currentTest->consoleMessages << msg;
                break;
            case QtInfoMsg:
                currentTest->consoleMessages << msg;
                break;
            case QtWarningMsg:
                currentTest->consoleWarnings << msg;
                break;
            case QtCriticalMsg:
            case QtFatalMsg:
                currentTest->consoleErrors << msg;
                break;
            }
        }

        // Call original handler if it exists
        if (originalHandler) {
            originalHandler(type, context, msg);
        }
    }

    void setupConsoleCapture()
    {
        currentTest = this;
        originalHandler = qInstallMessageHandler(messageHandler);
        consoleMessages.clear();
        consoleWarnings.clear();
        consoleErrors.clear();
    }

    void teardownConsoleCapture()
    {
        qInstallMessageHandler(originalHandler);
        currentTest = nullptr;
    }

    /**
     * @brief Load a QML component from file
     * @param qmlPath Path to QML file relative to project root
     * @return True if component loaded successfully
     */
    bool loadComponent(const QString& qmlPath)
    {
        // Clean up previous rootItem to prevent memory leak
        if (rootItem) {
            delete rootItem;
            rootItem = nullptr;
        }

        setupConsoleCapture();

        engine = std::make_unique<QQmlEngine>();
        component = std::make_unique<QQmlComponent>(engine.get(), qmlPath);

        if (component->isError()) {
            qWarning() << "Component load errors:";
            for (const auto& error : component->errors()) {
                qWarning() << "  " << error.toString();
                consoleErrors << error.toString();
            }
            return false;
        }

        QObject* obj = component->create();
        if (!obj) {
            consoleErrors << "Failed to create component instance";
            return false;
        }

        rootItem = qobject_cast<QQuickItem*>(obj);
        if (!rootItem) {
            consoleErrors << "Component is not a QQuickItem";
            delete obj;
            return false;
        }

        return true;
    }

    /**
     * @brief Load a QML component from string
     * @param qmlCode QML code as string
     * @param componentName Name for error reporting
     * @return True if component loaded successfully
     */
    bool loadComponentFromString(const QString& qmlCode, const QString& componentName)
    {
        // Clean up previous rootItem to prevent memory leak
        if (rootItem) {
            delete rootItem;
            rootItem = nullptr;
        }

        setupConsoleCapture();

        engine = std::make_unique<QQmlEngine>();
        component = std::make_unique<QQmlComponent>(engine.get());
        component->setData(qmlCode.toUtf8(), QUrl());

        if (component->isError()) {
            qWarning() << componentName << "load errors:";
            for (const auto& error : component->errors()) {
                qWarning() << "  " << error.toString();
                consoleErrors << error.toString();
            }
            return false;
        }

        QObject* obj = component->create();
        if (!obj) {
            consoleErrors << "Failed to create " + componentName + " instance";
            return false;
        }

        rootItem = qobject_cast<QQuickItem*>(obj);
        if (!rootItem) {
            consoleErrors << componentName + " is not a QQuickItem";
            delete obj;
            return false;
        }

        return true;
    }

    /**
     * @brief Verify zero console errors
     * @return True if no errors detected
     */
    bool verifyZeroErrors()
    {
        if (!consoleErrors.isEmpty()) {
            qWarning() << "Console errors detected:";
            for (const auto& error : consoleErrors) {
                qWarning() << "  ERROR:" << error;
            }
            return false;
        }
        return true;
    }

    /**
     * @brief Verify zero console warnings
     * @return True if no warnings detected
     */
    bool verifyZeroWarnings()
    {
        if (!consoleWarnings.isEmpty()) {
            qWarning() << "Console warnings detected:";
            for (const auto& warning : consoleWarnings) {
                qWarning() << "  WARNING:" << warning;
            }
            return false;
        }
        return true;
    }

    /**
     * @brief Verify property exists on component
     */
    bool hasProperty(const QString& propertyName)
    {
        if (!rootItem) return false;
        return rootItem->property(propertyName.toUtf8().constData()).isValid();
    }

    /**
     * @brief Get property value from component
     */
    QVariant getProperty(const QString& propertyName)
    {
        if (!rootItem) return QVariant();
        return rootItem->property(propertyName.toUtf8().constData());
    }

    /**
     * @brief Set property value on component
     */
    bool setProperty(const QString& propertyName, const QVariant& value)
    {
        if (!rootItem) return false;
        return rootItem->setProperty(propertyName.toUtf8().constData(), value);
    }

    /**
     * @brief Verify signal exists on component
     */
    bool hasSignal(const QString& signalName)
    {
        if (!rootItem) return false;

        const QMetaObject* metaObj = rootItem->metaObject();
        QString normalizedSignal = QMetaObject::normalizedSignature(signalName.toUtf8().constData());

        for (int i = metaObj->methodOffset(); i < metaObj->methodCount(); ++i) {
            QMetaMethod method = metaObj->method(i);
            if (method.methodType() == QMetaMethod::Signal) {
                if (method.methodSignature() == normalizedSignal) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * @brief Test component visual rendering
     * @return True if component can be rendered
     */
    bool testVisualRendering()
    {
        if (!rootItem) return false;

        // Create a window to host the component
        QQuickWindow window;
        rootItem->setParentItem(window.contentItem());

        // Set reasonable size
        rootItem->setWidth(200);
        rootItem->setHeight(100);

        // Force polish and update
        rootItem->polish();
        window.contentItem()->polish();

        // Process events to allow rendering
        QTest::qWait(100);

        return true;
    }

    /**
     * @brief Simulate mouse click on component
     */
    bool simulateClick(int x = -1, int y = -1)
    {
        if (!rootItem) return false;

        // Use center if coordinates not specified
        if (x < 0) x = rootItem->width() / 2;
        if (y < 0) y = rootItem->height() / 2;

        QPointF pos(x, y);
        QMouseEvent pressEvent(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

        QCoreApplication::sendEvent(rootItem, &pressEvent);
        QCoreApplication::sendEvent(rootItem, &releaseEvent);

        QTest::qWait(50);
        return true;
    }

    /**
     * @brief Clean up component and engine
     */
    void cleanup()
    {
        if (rootItem) {
            delete rootItem;
            rootItem = nullptr;
        }
        component.reset();
        engine.reset();
        teardownConsoleCapture();
    }

    /**
     * @brief Print test report
     */
    void printReport(const QString& componentName)
    {
        qInfo() << "=== Test Report:" << componentName << "===";
        qInfo() << "Console Messages:" << consoleMessages.size();
        qInfo() << "Console Warnings:" << consoleWarnings.size();
        qInfo() << "Console Errors:" << consoleErrors.size();

        if (!consoleErrors.isEmpty()) {
            qInfo() << "Errors:";
            for (const auto& error : consoleErrors) {
                qInfo() << "  -" << error;
            }
        }

        if (!consoleWarnings.isEmpty()) {
            qInfo() << "Warnings:";
            for (const auto& warning : consoleWarnings) {
                qInfo() << "  -" << warning;
            }
        }
        qInfo() << "===========================================";
    }
};

/**
 * @brief Helper macros for QML component testing
 */
#define VERIFY_ZERO_CONSOLE_ERRORS() \
    QVERIFY2(verifyZeroErrors(), "Console errors detected - see output above")

#define VERIFY_ZERO_CONSOLE_WARNINGS() \
    QVERIFY2(verifyZeroWarnings(), "Console warnings detected - see output above")

#define VERIFY_COMPONENT_LOADED() \
    QVERIFY2(rootItem != nullptr, "Component failed to load")

#define VERIFY_PROPERTY_EXISTS(prop) \
    QVERIFY2(hasProperty(prop), QString("Property '%1' does not exist").arg(prop).toUtf8().constData())

#define VERIFY_SIGNAL_EXISTS(signal) \
    QVERIFY2(hasSignal(signal), QString("Signal '%1' does not exist").arg(signal).toUtf8().constData())

// ============================================================================
// E2E Testing Framework Classes
// ============================================================================

namespace E2E {

/**
 * @brief Structure to hold a single console message with full context
 */
struct ConsoleMessage {
    QtMsgType type;
    QString message;
    QString file;
    int line;
    QString function;
    QDateTime timestamp;

    QString typeString() const;
    bool isError() const;
    bool isWarning() const;
    QString format() const;
};

/**
 * @brief Thread-safe console message handler singleton
 *
 * Captures all Qt console messages (qDebug, qInfo, qWarning, qCritical, qFatal)
 * and provides thread-safe access to them for test verification.
 */
class ConsoleMessageHandler {
public:
    static ConsoleMessageHandler& instance();

    void install();
    void uninstall();
    void clear();

    std::vector<ConsoleMessage> getMessages() const;
    std::vector<ConsoleMessage> getErrors() const;
    std::vector<ConsoleMessage> getWarnings() const;

    int errorCount() const;
    int warningCount() const;
    int messageCount() const;

    bool hasErrors() const;
    bool hasWarnings() const;

    void printMessages() const;
    void printErrors() const;
    void printWarnings() const;

private:
    ConsoleMessageHandler() = default;
    ~ConsoleMessageHandler() = default;
    ConsoleMessageHandler(const ConsoleMessageHandler&) = delete;
    ConsoleMessageHandler& operator=(const ConsoleMessageHandler&) = delete;

    static void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);
    void addMessage(const ConsoleMessage& msg);

    mutable QMutex mutex_;
    std::vector<ConsoleMessage> messages_;
    QtMessageHandler previousHandler_ = nullptr;
};

/**
 * @brief Comprehensive E2E testing framework for QML applications
 *
 * Provides:
 * - QML engine initialization and management
 * - Console message capture and verification
 * - Screenshot capture functionality
 * - Test result reporting
 * - Event processing utilities
 */
class E2ETestFramework {
public:
    E2ETestFramework();
    ~E2ETestFramework();

    // QML Engine Management
    bool initializeQmlEngine(const QString& qmlPath,
                           const QStringList& additionalImportPaths = QStringList());
    bool loadQmlFile(const QString& qmlFile);
    QQmlContext* rootContext() const;

    // Console Message Verification
    bool checkNoConsoleErrors() const;
    bool checkNoConsoleWarnings() const;
    bool checkNoConsoleIssues() const;
    ConsoleMessageHandler& messageHandler() const;
    void clearConsoleMessages();

    // Screenshot Capture
    bool captureScreenshot(const QString& filename, QQuickWindow* window = nullptr);
    bool captureScreenshotOnFailure(const QString& testName);

    // Test Reporting
    void printTestResult(const QString& testName, bool passed) const;
    void printConsoleMessages() const;
    void printConsoleErrors() const;
    void printConsoleWarnings() const;

    // Utilities
    bool waitForEngineReady(int timeoutMs = 5000);
    void processEvents(int durationMs = 0);

private:
    void setupScreenshotDirectory();

    std::unique_ptr<QQmlApplicationEngine> engine_;
    QString qmlPath_;
    QString screenshotDir_;
    bool engineReady_;
};

} // namespace E2E

#endif // TEST_FRAMEWORK_H
