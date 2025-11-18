/*
 *    Copyright (C) 2025
 *    welle.io Team
 *
 *    This file is part of the welle.io.
 *
 *    welle.io is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    welle.io is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with welle.io; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <QtQuick>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QDebug>

/**
 * Test Framework for QML E2E GUI Testing
 *
 * This framework provides utilities for testing QML components with:
 * - Console message capture and validation (ZERO error tolerance)
 * - QML component loading and validation
 * - Child object verification
 * - Error/warning detection and reporting
 */

namespace TestFramework {

// Message types for console output
enum MessageType {
    QtDebugMsg,
    QtWarningMsg,
    QtCriticalMsg,
    QtFatalMsg,
    QtInfoMsg
};

// Console message handler structure
struct ConsoleMessage {
    MessageType type;
    QString message;
    QString file;
    int line;
};

// Global message list for capturing console output
static QList<ConsoleMessage> g_consoleMessages;
static QtMessageHandler g_originalHandler = nullptr;

// Custom message handler to capture Qt console output
void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    ConsoleMessage consoleMsg;
    consoleMsg.message = msg;
    consoleMsg.file = context.file ? QString(context.file) : QString();
    consoleMsg.line = context.line;

    switch (type) {
        case QtDebugMsg:
            consoleMsg.type = QtDebugMsg;
            break;
        case QtWarningMsg:
            consoleMsg.type = QtWarningMsg;
            break;
        case QtCriticalMsg:
            consoleMsg.type = QtCriticalMsg;
            break;
        case QtFatalMsg:
            consoleMsg.type = QtFatalMsg;
            break;
        case QtInfoMsg:
            consoleMsg.type = QtInfoMsg;
            break;
    }

    g_consoleMessages.append(consoleMsg);

    // Also call the original handler to maintain normal logging
    if (g_originalHandler) {
        g_originalHandler(type, context, msg);
    }
}

// Install console message handler
void installMessageHandler()
{
    g_consoleMessages.clear();
    g_originalHandler = qInstallMessageHandler(customMessageHandler);
}

// Remove console message handler
void removeMessageHandler()
{
    qInstallMessageHandler(g_originalHandler);
    g_originalHandler = nullptr;
}

// Get all console messages
QList<ConsoleMessage> getConsoleMessages()
{
    return g_consoleMessages;
}

// Get console messages by type
QList<ConsoleMessage> getConsoleMessagesByType(MessageType type)
{
    QList<ConsoleMessage> filtered;
    for (const auto& msg : g_consoleMessages) {
        if (msg.type == type) {
            filtered.append(msg);
        }
    }
    return filtered;
}

// Get error count
int getErrorCount()
{
    int count = 0;
    for (const auto& msg : g_consoleMessages) {
        if (msg.type == QtCriticalMsg || msg.type == QtFatalMsg) {
            count++;
        }
    }
    return count;
}

// Get warning count
int getWarningCount()
{
    int count = 0;
    for (const auto& msg : g_consoleMessages) {
        if (msg.type == QtWarningMsg) {
            count++;
        }
    }
    return count;
}

// Get QML-specific error count (errors containing "QML" in message)
int getQmlErrorCount()
{
    int count = 0;
    for (const auto& msg : g_consoleMessages) {
        if ((msg.type == QtCriticalMsg || msg.type == QtFatalMsg || msg.type == QtWarningMsg)
            && msg.message.contains("QML", Qt::CaseInsensitive)) {
            count++;
        }
    }
    return count;
}

// Clear console messages
void clearConsoleMessages()
{
    g_consoleMessages.clear();
}

// Print all console messages (for debugging)
void printAllConsoleMessages()
{
    qDebug() << "=== Console Messages (" << g_consoleMessages.size() << "total) ===";
    for (const auto& msg : g_consoleMessages) {
        QString typeStr;
        switch (msg.type) {
            case QtDebugMsg: typeStr = "DEBUG"; break;
            case QtWarningMsg: typeStr = "WARNING"; break;
            case QtCriticalMsg: typeStr = "CRITICAL"; break;
            case QtFatalMsg: typeStr = "FATAL"; break;
            case QtInfoMsg: typeStr = "INFO"; break;
        }
        qDebug() << QString("[%1] %2").arg(typeStr).arg(msg.message);
        if (!msg.file.isEmpty()) {
            qDebug() << QString("  at %1:%2").arg(msg.file).arg(msg.line);
        }
    }
    qDebug() << "=== End Console Messages ===";
}

// Print errors and warnings only
void printErrorsAndWarnings()
{
    qDebug() << "=== Errors and Warnings ===";
    int errorCount = 0;
    int warningCount = 0;

    for (const auto& msg : g_consoleMessages) {
        if (msg.type == QtCriticalMsg || msg.type == QtFatalMsg) {
            qDebug() << "[ERROR]" << msg.message;
            if (!msg.file.isEmpty()) {
                qDebug() << QString("  at %1:%2").arg(msg.file).arg(msg.line);
            }
            errorCount++;
        } else if (msg.type == QtWarningMsg) {
            qDebug() << "[WARNING]" << msg.message;
            if (!msg.file.isEmpty()) {
                qDebug() << QString("  at %1:%2").arg(msg.file).arg(msg.line);
            }
            warningCount++;
        }
    }

    qDebug() << QString("Total: %1 errors, %2 warnings").arg(errorCount).arg(warningCount);
    qDebug() << "=== End Errors and Warnings ===";
}

/**
 * QML Component Test Helper
 *
 * Provides utilities for loading and testing QML components
 */
class QmlComponentTestHelper
{
public:
    QmlComponentTestHelper(QQmlEngine* engine)
        : m_engine(engine)
        , m_component(nullptr)
        , m_object(nullptr)
    {
    }

    ~QmlComponentTestHelper()
    {
        cleanup();
    }

    // Load a QML component from URL
    bool loadComponent(const QString& qmlPath)
    {
        clearConsoleMessages();

        if (!m_engine) {
            qCritical() << "QML Engine is null!";
            return false;
        }

        cleanup();

        QUrl url(qmlPath);
        m_component = new QQmlComponent(m_engine, url);

        if (m_component->isError()) {
            qCritical() << "Component loading errors:";
            for (const auto& error : m_component->errors()) {
                qCritical() << "  " << error.toString();
            }
            return false;
        }

        return true;
    }

    // Create the component instance
    bool createObject(QQmlContext* context = nullptr)
    {
        if (!m_component) {
            qCritical() << "Component is null! Load component first.";
            return false;
        }

        if (m_component->isError()) {
            qCritical() << "Cannot create object from component with errors";
            return false;
        }

        if (context) {
            m_object = qobject_cast<QQuickItem*>(m_component->create(context));
        } else {
            m_object = qobject_cast<QQuickItem*>(m_component->create());
        }

        if (!m_object) {
            qCritical() << "Failed to create QML object!";
            if (m_component->isError()) {
                for (const auto& error : m_component->errors()) {
                    qCritical() << "  " << error.toString();
                }
            }
            return false;
        }

        return true;
    }

    // Get the component
    QQmlComponent* component() const { return m_component; }

    // Get the created object
    QQuickItem* object() const { return m_object; }

    // Find child object by objectName
    QObject* findChild(const QString& objectName) const
    {
        if (!m_object) return nullptr;
        return m_object->findChild<QObject*>(objectName);
    }

    // Find all children of a specific type
    template<typename T>
    QList<T*> findChildren() const
    {
        if (!m_object) return QList<T*>();
        return m_object->findChildren<T*>();
    }

    // Get all child count
    int childCount() const
    {
        if (!m_object) return 0;
        return m_object->childItems().count();
    }

    // Verify component status
    bool isReady() const
    {
        return m_component && m_component->status() == QQmlComponent::Ready;
    }

    // Verify component has no errors
    bool hasNoErrors() const
    {
        return m_component && !m_component->isError();
    }

    // Get component errors
    QList<QQmlError> errors() const
    {
        if (!m_component) return QList<QQmlError>();
        return m_component->errors();
    }

    // Cleanup
    void cleanup()
    {
        if (m_object) {
            delete m_object;
            m_object = nullptr;
        }
        if (m_component) {
            delete m_component;
            m_component = nullptr;
        }
    }

private:
    QQmlEngine* m_engine;
    QQmlComponent* m_component;
    QQuickItem* m_object;
};

/**
 * Test Verification Macros
 */

// Verify zero console errors
#define VERIFY_ZERO_ERRORS() \
    do { \
        int errorCount = TestFramework::getErrorCount(); \
        if (errorCount > 0) { \
            TestFramework::printErrorsAndWarnings(); \
        } \
        QCOMPARE(errorCount, 0); \
    } while (0)

// Verify zero console warnings
#define VERIFY_ZERO_WARNINGS() \
    do { \
        int warningCount = TestFramework::getWarningCount(); \
        if (warningCount > 0) { \
            TestFramework::printErrorsAndWarnings(); \
        } \
        QCOMPARE(warningCount, 0); \
    } while (0)

// Verify zero QML errors
#define VERIFY_ZERO_QML_ERRORS() \
    do { \
        int qmlErrorCount = TestFramework::getQmlErrorCount(); \
        if (qmlErrorCount > 0) { \
            TestFramework::printErrorsAndWarnings(); \
        } \
        QCOMPARE(qmlErrorCount, 0); \
    } while (0)

// Verify component loaded successfully
#define VERIFY_COMPONENT_LOADED(helper) \
    do { \
        QVERIFY2(helper.isReady(), "Component should be ready"); \
        QVERIFY2(helper.hasNoErrors(), "Component should have no errors"); \
        if (!helper.hasNoErrors()) { \
            for (const auto& error : helper.errors()) { \
                qCritical() << error.toString(); \
            } \
        } \
    } while (0)

// Verify object created successfully
#define VERIFY_OBJECT_CREATED(helper) \
    do { \
        QVERIFY2(helper.object() != nullptr, "QML object should be created"); \
    } while (0)

// Verify child exists
#define VERIFY_CHILD_EXISTS(parent, childName) \
    do { \
        QObject* child = parent->findChild<QObject*>(childName); \
        QVERIFY2(child != nullptr, QString("Child '%1' should exist").arg(childName).toUtf8().constData()); \
    } while (0)

} // namespace TestFramework

#endif // TEST_FRAMEWORK_H
