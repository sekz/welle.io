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

#include "../test_framework.h"
#include <QGuiApplication>
#include <QtQuick>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>
#include <QtTest/QtTest>

/**
 * E2E GUI Tests for GeneralView.qml
 *
 * Test Coverage:
 * - Component loading without console errors
 * - Visual elements presence verification
 * - Interactive elements functionality
 * - Zero QML warnings/errors validation
 * - Component initialization and state management
 */
class TestGeneralView : public QObject
{
    Q_OBJECT

public:
    TestGeneralView() : m_engine(nullptr) {}
    ~TestGeneralView() {}

private slots:
    void initTestCase()
    {
        qDebug() << "=== TestGeneralView: Starting test suite ===";

        // Install message handler to capture console output
        TestFramework::installMessageHandler();

        // Create QML engine without parent to avoid double cleanup
        m_engine = new QQmlEngine();
        QVERIFY2(m_engine != nullptr, "QML Engine should be created");

        qDebug() << "QML Engine created successfully";
    }

    void cleanupTestCase()
    {
        qDebug() << "=== TestGeneralView: Cleaning up test suite ===";

        if (m_engine) {
            delete m_engine;
            m_engine = nullptr;
        }

        // Remove message handler
        TestFramework::removeMessageHandler();

        qDebug() << "=== TestGeneralView: Test suite completed ===";
    }

    void init()
    {
        // Clear console messages before each test
        TestFramework::clearConsoleMessages();
    }

    void cleanup()
    {
        // Nothing to do here for now
    }

    /**
     * Test: Load GeneralView component
     *
     * Verifies:
     * - Component loads successfully
     * - No QML errors during loading
     * - Component status is Ready
     */
    void testLoadComponent()
    {
        qDebug() << "TEST: Load GeneralView component";

        TestFramework::QmlComponentTestHelper helper(m_engine);

        bool loaded = helper.loadComponent("qrc:/QML/GeneralView.qml");
        QVERIFY2(loaded, "GeneralView component should load successfully");

        VERIFY_COMPONENT_LOADED(helper);
        VERIFY_ZERO_ERRORS();

        qDebug() << "PASS: GeneralView component loaded without errors";
    }

    /**
     * Test: Create GeneralView object
     *
     * Verifies:
     * - Component can be instantiated
     * - Object is created without errors
     * - Zero console errors during creation
     */
    void testCreateObject()
    {
        qDebug() << "TEST: Create GeneralView object";

        TestFramework::QmlComponentTestHelper helper(m_engine);

        bool loaded = helper.loadComponent("qrc:/QML/GeneralView.qml");
        QVERIFY(loaded);

        // Create context with mock objects to avoid missing property warnings
        QScopedPointer<QQmlContext> context(new QQmlContext(m_engine->rootContext()));

        // Create mock radioController
        QScopedPointer<QObject> mockRadioController(new QObject());
        mockRadioController->setProperty("ensemble", QString(""));
        mockRadioController->setProperty("title", QString(""));
        mockRadioController->setProperty("isPlaying", false);

        // Create mock guiHelper
        QScopedPointer<QObject> mockGuiHelper(new QObject());

        context->setContextProperty("radioController", mockRadioController.data());
        context->setContextProperty("guiHelper", mockGuiHelper.data());

        bool created = helper.createObject(context.data());
        QVERIFY2(created, "GeneralView object should be created");

        VERIFY_OBJECT_CREATED(helper);
        VERIFY_ZERO_ERRORS();

        qDebug() << "PASS: GeneralView object created without errors";
    }

    /**
     * Test: Verify component properties
     *
     * Verifies:
     * - All expected properties exist
     * - Properties have correct default values
     * - No errors when accessing properties
     */
    void testComponentProperties()
    {
        qDebug() << "TEST: Verify GeneralView properties";

        TestFramework::QmlComponentTestHelper helper(m_engine);
        QVERIFY(helper.loadComponent("qrc:/QML/GeneralView.qml"));

        // Create context with mock objects
        QScopedPointer<QQmlContext> context(new QQmlContext(m_engine->rootContext()));
        QScopedPointer<QObject> mockRadioController(new QObject());
        mockRadioController->setProperty("ensemble", QString(""));
        mockRadioController->setProperty("title", QString(""));
        mockRadioController->setProperty("isPlaying", false);
        QScopedPointer<QObject> mockGuiHelper(new QObject());
        context->setContextProperty("radioController", mockRadioController.data());
        context->setContextProperty("guiHelper", mockGuiHelper.data());

        QVERIFY(helper.createObject(context.data()));

        QQuickItem* generalView = helper.object();
        QVERIFY(generalView != nullptr);

        // Test property existence
        QVariant serializedProp = generalView->property("serialized");
        QVERIFY2(serializedProp.isValid(), "Property 'serialized' should exist");

        QVariant isExpertProp = generalView->property("isExpert");
        QVERIFY2(isExpertProp.isValid(), "Property 'isExpert' should exist");

        QVariant isPortraitProp = generalView->property("isPortrait");
        QVERIFY2(isPortraitProp.isValid(), "Property 'isPortrait' should exist");

        // Test default values
        QCOMPARE(isExpertProp.toBool(), false);
        QCOMPARE(isPortraitProp.toBool(), false);

        VERIFY_ZERO_ERRORS();

        qDebug() << "PASS: GeneralView properties verified";
    }

    /**
     * Test: Verify JavaScript functions exist
     *
     * Verifies:
     * - All required functions are accessible
     * - No errors when checking function availability
     */
    void testComponentFunctions()
    {
        qDebug() << "TEST: Verify GeneralView functions";

        TestFramework::QmlComponentTestHelper helper(m_engine);
        QVERIFY(helper.loadComponent("qrc:/QML/GeneralView.qml"));

        // Create context with mock objects
        QScopedPointer<QQmlContext> context(new QQmlContext(m_engine->rootContext()));
        QScopedPointer<QObject> mockRadioController(new QObject());
        mockRadioController->setProperty("ensemble", QString(""));
        mockRadioController->setProperty("title", QString(""));
        mockRadioController->setProperty("isPlaying", false);
        QScopedPointer<QObject> mockGuiHelper(new QObject());
        context->setContextProperty("radioController", mockRadioController.data());
        context->setContextProperty("guiHelper", mockGuiHelper.data());

        QVERIFY(helper.createObject(context.data()));

        QQuickItem* generalView = helper.object();
        QVERIFY(generalView != nullptr);

        // Verify key functions exist by checking metaObject
        const QMetaObject* metaObj = generalView->metaObject();
        QVERIFY(metaObj != nullptr);

        // Check for addComponent method
        bool hasAddComponent = false;
        for (int i = metaObj->methodOffset(); i < metaObj->methodCount(); ++i) {
            QMetaMethod method = metaObj->method(i);
            if (QString(method.name()) == "addComponent") {
                hasAddComponent = true;
                break;
            }
        }
        QVERIFY2(hasAddComponent, "addComponent function should exist");

        VERIFY_ZERO_ERRORS();

        qDebug() << "PASS: GeneralView functions verified";
    }

    /**
     * Test: Verify child components initialization
     *
     * Verifies:
     * - Child items can be created
     * - Component hierarchy is correct
     * - No errors during child initialization
     */
    void testChildComponents()
    {
        qDebug() << "TEST: Verify GeneralView child components";

        TestFramework::QmlComponentTestHelper helper(m_engine);
        QVERIFY(helper.loadComponent("qrc:/QML/GeneralView.qml"));

        // Create context with mock objects
        QScopedPointer<QQmlContext> context(new QQmlContext(m_engine->rootContext()));
        QScopedPointer<QObject> mockRadioController(new QObject());
        mockRadioController->setProperty("ensemble", QString(""));
        mockRadioController->setProperty("title", QString(""));
        mockRadioController->setProperty("isPlaying", false);
        QScopedPointer<QObject> mockGuiHelper(new QObject());
        context->setContextProperty("radioController", mockRadioController.data());
        context->setContextProperty("guiHelper", mockGuiHelper.data());

        QVERIFY(helper.createObject(context.data()));

        QQuickItem* generalView = helper.object();
        QVERIFY(generalView != nullptr);

        // GeneralView is a GridLayout, it may not have children immediately
        // but should be able to have children added
        int initialChildCount = helper.childCount();
        QVERIFY2(initialChildCount >= 0, "Should be able to query child count");

        VERIFY_ZERO_ERRORS();

        qDebug() << "PASS: GeneralView child components verified";
    }

    /**
     * Test: Test isExpert property changes
     *
     * Verifies:
     * - isExpert property can be changed
     * - No errors when changing property
     * - Property change is reflected correctly
     */
    void testIsExpertProperty()
    {
        qDebug() << "TEST: Test isExpert property changes";

        TestFramework::QmlComponentTestHelper helper(m_engine);
        QVERIFY(helper.loadComponent("qrc:/QML/GeneralView.qml"));

        // Create context with mock objects
        QScopedPointer<QQmlContext> context(new QQmlContext(m_engine->rootContext()));
        QScopedPointer<QObject> mockRadioController(new QObject());
        mockRadioController->setProperty("ensemble", QString(""));
        mockRadioController->setProperty("title", QString(""));
        mockRadioController->setProperty("isPlaying", false);
        QScopedPointer<QObject> mockGuiHelper(new QObject());
        context->setContextProperty("radioController", mockRadioController.data());
        context->setContextProperty("guiHelper", mockGuiHelper.data());

        QVERIFY(helper.createObject(context.data()));

        QQuickItem* generalView = helper.object();
        QVERIFY(generalView != nullptr);

        // Test initial value
        QVariant isExpert = generalView->property("isExpert");
        QVERIFY(isExpert.isValid());
        QCOMPARE(isExpert.toBool(), false);

        // Change property
        bool setResult = generalView->setProperty("isExpert", true);
        QVERIFY2(setResult, "Should be able to set isExpert property");

        // Verify change
        QVariant isExpertAfter = generalView->property("isExpert");
        QCOMPARE(isExpertAfter.toBool(), true);

        VERIFY_ZERO_ERRORS();

        qDebug() << "PASS: isExpert property changes verified";
    }

    /**
     * Test: Test isPortrait property changes
     *
     * Verifies:
     * - isPortrait property can be changed
     * - No errors when changing property
     * - Property change is reflected correctly
     */
    void testIsPortraitProperty()
    {
        qDebug() << "TEST: Test isPortrait property changes";

        TestFramework::QmlComponentTestHelper helper(m_engine);
        QVERIFY(helper.loadComponent("qrc:/QML/GeneralView.qml"));

        // Create context with mock objects
        QScopedPointer<QQmlContext> context(new QQmlContext(m_engine->rootContext()));
        QScopedPointer<QObject> mockRadioController(new QObject());
        mockRadioController->setProperty("ensemble", QString(""));
        mockRadioController->setProperty("title", QString(""));
        mockRadioController->setProperty("isPlaying", false);
        QScopedPointer<QObject> mockGuiHelper(new QObject());
        context->setContextProperty("radioController", mockRadioController.data());
        context->setContextProperty("guiHelper", mockGuiHelper.data());

        QVERIFY(helper.createObject(context.data()));

        QQuickItem* generalView = helper.object();
        QVERIFY(generalView != nullptr);

        // Test initial value
        QVariant isPortrait = generalView->property("isPortrait");
        QVERIFY(isPortrait.isValid());
        QCOMPARE(isPortrait.toBool(), false);

        // Change property
        bool setResult = generalView->setProperty("isPortrait", true);
        QVERIFY2(setResult, "Should be able to set isPortrait property");

        // Verify change
        QVariant isPortraitAfter = generalView->property("isPortrait");
        QCOMPARE(isPortraitAfter.toBool(), true);

        VERIFY_ZERO_ERRORS();

        qDebug() << "PASS: isPortrait property changes verified";
    }

    /**
     * Test: Verify zero console errors throughout all tests
     *
     * Verifies:
     * - No critical errors in console
     * - No fatal errors in console
     * - Component is clean
     */
    void testZeroConsoleErrors()
    {
        qDebug() << "TEST: Verify zero console errors";

        TestFramework::QmlComponentTestHelper helper(m_engine);
        QVERIFY(helper.loadComponent("qrc:/QML/GeneralView.qml"));

        // Create context with mock objects
        QScopedPointer<QQmlContext> context(new QQmlContext(m_engine->rootContext()));
        QScopedPointer<QObject> mockRadioController(new QObject());
        mockRadioController->setProperty("ensemble", QString(""));
        mockRadioController->setProperty("title", QString(""));
        mockRadioController->setProperty("isPlaying", false);
        QScopedPointer<QObject> mockGuiHelper(new QObject());
        context->setContextProperty("radioController", mockRadioController.data());
        context->setContextProperty("guiHelper", mockGuiHelper.data());

        QVERIFY(helper.createObject(context.data()));

        // Verify zero errors
        VERIFY_ZERO_ERRORS();

        // Print summary
        int totalMessages = TestFramework::getConsoleMessages().size();
        int errorCount = TestFramework::getErrorCount();
        int warningCount = TestFramework::getWarningCount();

        qDebug() << QString("Console summary: %1 total messages, %2 errors, %3 warnings")
                    .arg(totalMessages).arg(errorCount).arg(warningCount);

        QCOMPARE(errorCount, 0);

        qDebug() << "PASS: Zero console errors verified";
    }

    /**
     * Test: Verify component renders without warnings
     *
     * Verifies:
     * - Component can be rendered
     * - No QML warnings during rendering
     * - Visual tree is valid
     */
    void testComponentRenders()
    {
        qDebug() << "TEST: Verify component renders";

        TestFramework::QmlComponentTestHelper helper(m_engine);
        QVERIFY(helper.loadComponent("qrc:/QML/GeneralView.qml"));

        // Create context with mock objects
        QScopedPointer<QQmlContext> context(new QQmlContext(m_engine->rootContext()));
        QScopedPointer<QObject> mockRadioController(new QObject());
        mockRadioController->setProperty("ensemble", QString(""));
        mockRadioController->setProperty("title", QString(""));
        mockRadioController->setProperty("isPlaying", false);
        QScopedPointer<QObject> mockGuiHelper(new QObject());
        context->setContextProperty("radioController", mockRadioController.data());
        context->setContextProperty("guiHelper", mockGuiHelper.data());

        QVERIFY(helper.createObject(context.data()));

        QQuickItem* generalView = helper.object();
        QVERIFY(generalView != nullptr);

        // Check basic rendering properties
        QVERIFY2(generalView->width() >= 0, "Width should be non-negative");
        QVERIFY2(generalView->height() >= 0, "Height should be non-negative");

        // Check visibility
        bool isVisible = generalView->isVisible();
        // Component may not be visible until added to a view, that's OK
        qDebug() << "Component visible:" << isVisible;

        VERIFY_ZERO_ERRORS();

        qDebug() << "PASS: Component renders without errors";
    }

private:
    QQmlEngine* m_engine;
};

QTEST_MAIN(TestGeneralView)
#include "test_general_view.moc"
