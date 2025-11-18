/**
 * @file test_framework.cpp
 * @brief Implementation of the end-to-end GUI testing framework for welle.io
 *
 * This file implements the comprehensive testing utilities declared in test_framework.h,
 * providing full console log capture, screenshot functionality, and test reporting.
 */

#include "test_framework.h"
#include <QCoreApplication>
#include <QQmlComponent>
#include <QQuickItem>
#include <QDir>
#include <QStandardPaths>
#include <QBuffer>
#include <QImageWriter>
#include <iostream>

// Static member initialization
QtMessageHandler QMLComponentTestBase::originalHandler = nullptr;
QMLComponentTestBase* QMLComponentTestBase::currentTest = nullptr;

namespace E2E {

// ============================================================================
// ConsoleMessage Implementation
// ============================================================================

QString ConsoleMessage::typeString() const {
    switch (type) {
        case QtDebugMsg:
            return "DEBUG";
        case QtInfoMsg:
            return "INFO";
        case QtWarningMsg:
            return "WARNING";
        case QtCriticalMsg:
            return "CRITICAL";
        case QtFatalMsg:
            return "FATAL";
        default:
            return "UNKNOWN";
    }
}

bool ConsoleMessage::isError() const {
    return type == QtCriticalMsg || type == QtFatalMsg;
}

bool ConsoleMessage::isWarning() const {
    return type == QtWarningMsg;
}

QString ConsoleMessage::format() const {
    QString result;
    result += QString("[%1] ").arg(typeString());
    result += message;

    if (!file.isEmpty()) {
        result += QString(" (%1:%2)").arg(file).arg(line);
    }

    if (!function.isEmpty()) {
        result += QString(" in %1").arg(function);
    }

    return result;
}

// ============================================================================
// ConsoleMessageHandler Implementation
// ============================================================================

ConsoleMessageHandler& ConsoleMessageHandler::instance() {
    static ConsoleMessageHandler handler;
    return handler;
}

void ConsoleMessageHandler::install() {
    if (previousHandler_ == nullptr) {
        previousHandler_ = qInstallMessageHandler(messageHandler);
    }
}

void ConsoleMessageHandler::uninstall() {
    if (previousHandler_ != nullptr) {
        qInstallMessageHandler(previousHandler_);
        previousHandler_ = nullptr;
    }
}

void ConsoleMessageHandler::clear() {
    QMutexLocker locker(&mutex_);
    messages_.clear();
}

std::vector<ConsoleMessage> ConsoleMessageHandler::getMessages() const {
    QMutexLocker locker(&mutex_);
    return messages_;
}

std::vector<ConsoleMessage> ConsoleMessageHandler::getErrors() const {
    QMutexLocker locker(&mutex_);
    std::vector<ConsoleMessage> errors;
    for (const auto& msg : messages_) {
        if (msg.isError()) {
            errors.push_back(msg);
        }
    }
    return errors;
}

std::vector<ConsoleMessage> ConsoleMessageHandler::getWarnings() const {
    QMutexLocker locker(&mutex_);
    std::vector<ConsoleMessage> warnings;
    for (const auto& msg : messages_) {
        if (msg.isWarning()) {
            warnings.push_back(msg);
        }
    }
    return warnings;
}

int ConsoleMessageHandler::errorCount() const {
    QMutexLocker locker(&mutex_);
    int count = 0;
    for (const auto& msg : messages_) {
        if (msg.isError()) {
            count++;
        }
    }
    return count;
}

int ConsoleMessageHandler::warningCount() const {
    QMutexLocker locker(&mutex_);
    int count = 0;
    for (const auto& msg : messages_) {
        if (msg.isWarning()) {
            count++;
        }
    }
    return count;
}

int ConsoleMessageHandler::messageCount() const {
    QMutexLocker locker(&mutex_);
    return messages_.size();
}

bool ConsoleMessageHandler::hasErrors() const {
    return errorCount() > 0;
}

bool ConsoleMessageHandler::hasWarnings() const {
    return warningCount() > 0;
}

void ConsoleMessageHandler::printMessages() const {
    QMutexLocker locker(&mutex_);
    if (messages_.empty()) {
        qInfo() << "No console messages captured.";
        return;
    }

    qInfo() << "=== Captured Console Messages (" << messages_.size() << ") ===";
    for (const auto& msg : messages_) {
        qInfo() << msg.format();
    }
    qInfo() << "===========================================";
}

void ConsoleMessageHandler::printErrors() const {
    auto errors = getErrors();
    if (errors.empty()) {
        qInfo() << "No console errors captured.";
        return;
    }

    qInfo() << "=== Captured Console Errors (" << errors.size() << ") ===";
    for (const auto& msg : errors) {
        qInfo() << msg.format();
    }
    qInfo() << "===========================================";
}

void ConsoleMessageHandler::printWarnings() const {
    auto warnings = getWarnings();
    if (warnings.empty()) {
        qInfo() << "No console warnings captured.";
        return;
    }

    qInfo() << "=== Captured Console Warnings (" << warnings.size() << ") ===";
    for (const auto& msg : warnings) {
        qInfo() << msg.format();
    }
    qInfo() << "===========================================";
}

void ConsoleMessageHandler::messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    ConsoleMessage message;
    message.type = type;
    message.message = msg;
    message.file = context.file ? QString(context.file) : QString();
    message.line = context.line;
    message.function = context.function ? QString(context.function) : QString();
    message.timestamp = QDateTime::currentDateTime();

    instance().addMessage(message);

    // Also output to stderr for immediate visibility
    QString formatted = message.format();
    fprintf(stderr, "%s\n", formatted.toUtf8().constData());
    fflush(stderr);
}

void ConsoleMessageHandler::addMessage(const ConsoleMessage& msg) {
    QMutexLocker locker(&mutex_);
    messages_.push_back(msg);
}

// ============================================================================
// E2ETestFramework Implementation
// ============================================================================

E2ETestFramework::E2ETestFramework()
    : engineReady_(false)
{
    setupScreenshotDirectory();
    ConsoleMessageHandler::instance().install();
    ConsoleMessageHandler::instance().clear();
}

E2ETestFramework::~E2ETestFramework() {
    engine_.reset();
    ConsoleMessageHandler::instance().uninstall();
}

bool E2ETestFramework::initializeQmlEngine(const QString& qmlPath,
                                           const QStringList& additionalImportPaths) {
    qmlPath_ = qmlPath;

    // Create the QML engine
    engine_ = std::make_unique<QQmlApplicationEngine>();

    if (!engine_) {
        qCritical() << "Failed to create QML engine";
        return false;
    }

    // Add import paths
    engine_->addImportPath(qmlPath);
    for (const QString& path : additionalImportPaths) {
        engine_->addImportPath(path);
    }

    // Enable debugging
    engine_->setOutputWarningsToStandardError(true);

    engineReady_ = true;
    qInfo() << "QML engine initialized successfully";
    qInfo() << "Import paths:" << engine_->importPathList();

    return true;
}

bool E2ETestFramework::loadQmlFile(const QString& qmlFile) {
    if (!engine_) {
        qCritical() << "QML engine not initialized. Call initializeQmlEngine() first.";
        return false;
    }

    // Clear previous messages before loading
    ConsoleMessageHandler::instance().clear();

    QUrl url;
    if (qmlFile.startsWith("qrc:")) {
        url = QUrl(qmlFile);
    } else if (QFile::exists(qmlFile)) {
        url = QUrl::fromLocalFile(qmlFile);
    } else {
        // Try relative to qmlPath
        QString fullPath = qmlPath_ + "/" + qmlFile;
        if (QFile::exists(fullPath)) {
            url = QUrl::fromLocalFile(fullPath);
        } else {
            qCritical() << "QML file not found:" << qmlFile;
            return false;
        }
    }

    qInfo() << "Loading QML file:" << url.toString();
    engine_->load(url);

    // Wait a bit for the engine to process
    processEvents(100);

    // Check for load errors
    if (engine_->rootObjects().isEmpty()) {
        qCritical() << "Failed to load QML file, no root objects created";
        return false;
    }

    qInfo() << "QML file loaded successfully";
    return true;
}

QQmlContext* E2ETestFramework::rootContext() const {
    if (!engine_) {
        return nullptr;
    }
    return engine_->rootContext();
}

bool E2ETestFramework::checkNoConsoleErrors() const {
    return !ConsoleMessageHandler::instance().hasErrors();
}

bool E2ETestFramework::checkNoConsoleWarnings() const {
    return !ConsoleMessageHandler::instance().hasWarnings();
}

bool E2ETestFramework::checkNoConsoleIssues() const {
    return checkNoConsoleErrors() && checkNoConsoleWarnings();
}

ConsoleMessageHandler& E2ETestFramework::messageHandler() const {
    return ConsoleMessageHandler::instance();
}

void E2ETestFramework::clearConsoleMessages() {
    ConsoleMessageHandler::instance().clear();
}

bool E2ETestFramework::captureScreenshot(const QString& filename, QQuickWindow* window) {
    QQuickWindow* targetWindow = window;

    // If no window specified, try to get the root window
    if (!targetWindow && engine_) {
        QList<QObject*> roots = engine_->rootObjects();
        if (!roots.isEmpty()) {
            QObject* root = roots.first();
            QQuickWindow* rootWindow = qobject_cast<QQuickWindow*>(root);
            if (rootWindow) {
                targetWindow = rootWindow;
            } else {
                // Try to find a window through the item
                QQuickItem* item = qobject_cast<QQuickItem*>(root);
                if (item) {
                    targetWindow = item->window();
                }
            }
        }
    }

    if (!targetWindow) {
        qWarning() << "No window available for screenshot";
        return false;
    }

    // Capture the window content
    QImage screenshot = targetWindow->grabWindow();

    if (screenshot.isNull()) {
        qWarning() << "Failed to capture screenshot";
        return false;
    }

    // Save the screenshot
    QString fullPath = screenshotDir_ + "/" + filename;
    if (!fullPath.endsWith(".png", Qt::CaseInsensitive)) {
        fullPath += ".png";
    }

    bool saved = screenshot.save(fullPath, "PNG");
    if (saved) {
        qInfo() << "Screenshot saved to:" << fullPath;
    } else {
        qWarning() << "Failed to save screenshot to:" << fullPath;
    }

    return saved;
}

bool E2ETestFramework::captureScreenshotOnFailure(const QString& testName) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString filename = QString("failure_%1_%2").arg(testName).arg(timestamp);
    return captureScreenshot(filename);
}

void E2ETestFramework::printTestResult(const QString& testName, bool passed) const {
    qInfo() << "========================================";
    qInfo() << "Test:" << testName;
    qInfo() << "Result:" << (passed ? "PASSED" : "FAILED");
    qInfo() << "Console Messages:" << ConsoleMessageHandler::instance().messageCount();
    qInfo() << "Console Warnings:" << ConsoleMessageHandler::instance().warningCount();
    qInfo() << "Console Errors:" << ConsoleMessageHandler::instance().errorCount();
    qInfo() << "========================================";
}

void E2ETestFramework::printConsoleMessages() const {
    ConsoleMessageHandler::instance().printMessages();
}

void E2ETestFramework::printConsoleErrors() const {
    ConsoleMessageHandler::instance().printErrors();
}

void E2ETestFramework::printConsoleWarnings() const {
    ConsoleMessageHandler::instance().printWarnings();
}

bool E2ETestFramework::waitForEngineReady(int timeoutMs) {
    if (engineReady_) {
        return true;
    }

    int elapsed = 0;
    int interval = 100;

    while (elapsed < timeoutMs) {
        processEvents(interval);
        if (engineReady_) {
            return true;
        }
        elapsed += interval;
    }

    qWarning() << "Timeout waiting for QML engine to be ready";
    return false;
}

void E2ETestFramework::processEvents(int durationMs) {
    QCoreApplication::processEvents();

    if (durationMs > 0) {
        QTest::qWait(durationMs);
    }
}

void E2ETestFramework::setupScreenshotDirectory() {
    // Create screenshots directory in temp location
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    screenshotDir_ = tempDir + "/welle-io-test-screenshots";

    QDir dir;
    if (!dir.exists(screenshotDir_)) {
        if (dir.mkpath(screenshotDir_)) {
            qInfo() << "Created screenshot directory:" << screenshotDir_;
        } else {
            qWarning() << "Failed to create screenshot directory:" << screenshotDir_;
            screenshotDir_ = tempDir; // Fallback to temp
        }
    }
}

} // namespace E2E
