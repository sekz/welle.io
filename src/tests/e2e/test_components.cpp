/*
 *    Copyright (C) 2024
 *    welle.io E2E GUI Component Tests
 *
 *    This file is part of the welle.io E2E testing suite.
 *
 *    welle.io is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    CRITICAL FIX: Tests now load ACTUAL component files instead of inline QML
 *    - Loads from: /home/user/welle.io/src/welle-gui/QML/components/
 *    - Proper import paths configured
 *    - Mock context properties for dependencies
 *    - Added missing component tests
 */

#include <QtTest>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSignalSpy>
#include "test_framework.h"

/**
 * @brief Enhanced QML Component Test Base with actual file loading support
 */
class ComponentTestFramework : public QMLComponentTestBase
{
protected:
    QObject* rootObject = nullptr;  // For non-visual components

    /**
     * @brief Load actual QML component file with proper setup
     * @param componentFileName The component file name (e.g., "WButton.qml")
     * @return True if component loaded successfully
     */
    bool loadActualComponent(const QString& componentFileName)
    {
        // Clean up previous instances
        if (rootItem) {
            delete rootItem;
            rootItem = nullptr;
        }
        if (rootObject) {
            delete rootObject;
            rootObject = nullptr;
        }

        setupConsoleCapture();

        engine = std::make_unique<QQmlEngine>();

        // Add import paths for QML modules
        engine->addImportPath("/home/user/welle.io/src/welle-gui/QML");
        engine->addImportPath("/home/user/welle.io/src/welle-gui/QML/components");
        engine->addImportPath("/home/user/welle.io/src/welle-gui/QML/texts");

        // Setup mock context properties
        setupMockContext();

        // Load component from file
        QString componentPath = "/home/user/welle.io/src/welle-gui/QML/components/" + componentFileName;
        QUrl componentUrl = QUrl::fromLocalFile(componentPath);

        component = std::make_unique<QQmlComponent>(engine.get(), componentUrl);

        if (component->isError()) {
            qWarning() << "Component load errors for" << componentFileName << ":";
            for (const auto& error : component->errors()) {
                qWarning() << "  " << error.toString();
                consoleErrors << error.toString();
            }
            return false;
        }

        QObject* obj = component->create();
        if (!obj) {
            consoleErrors << "Failed to create " + componentFileName + " instance";
            return false;
        }

        // Try to cast to QQuickItem
        rootItem = qobject_cast<QQuickItem*>(obj);
        if (!rootItem) {
            // Not a visual component, store as QObject (for singletons, models, etc.)
            rootObject = obj;
            qInfo() << componentFileName << "loaded as non-visual component (QObject)";
            return true;
        }

        qInfo() << componentFileName << "loaded successfully as visual component";
        return true;
    }

    /**
     * @brief Setup mock context properties that components might need
     */
    void setupMockContext()
    {
        if (!engine) return;

        auto* context = engine->rootContext();
        if (!context) return;

        // Mock guiHelper object
        QObject* mockGuiHelper = new QObject(engine.get());
        mockGuiHelper->setProperty("dummy", true);
        // Add updateMprisStationList method
        context->setContextProperty("guiHelper", mockGuiHelper);

        // Mock radioController object
        QObject* mockRadioController = new QObject(engine.get());
        mockRadioController->setProperty("dummy", true);
        context->setContextProperty("radioController", mockRadioController);

        // Mock infoMessagePopup object with required properties
        QObject* mockInfoMessagePopup = new QObject(engine.get());
        mockInfoMessagePopup->setProperty("text", "");
        context->setContextProperty("infoMessagePopup", mockInfoMessagePopup);

        // Mock qsTr function for translations
        context->setContextProperty("qsTr", QVariant::fromValue([](const QString& text) {
            return text;
        }));
    }

    /**
     * @brief Override cleanup to handle both visual and non-visual components
     */
    void cleanup() override
    {
        if (rootItem) {
            delete rootItem;
            rootItem = nullptr;
        }
        if (rootObject) {
            delete rootObject;
            rootObject = nullptr;
        }
        component.reset();
        engine.reset();
        teardownConsoleCapture();
    }

    /**
     * @brief Get property from either rootItem or rootObject
     */
    QVariant getPropertyUniversal(const QString& propertyName)
    {
        if (rootItem) {
            return rootItem->property(propertyName.toUtf8().constData());
        } else if (rootObject) {
            return rootObject->property(propertyName.toUtf8().constData());
        }
        return QVariant();
    }

    /**
     * @brief Check if property exists on either rootItem or rootObject
     */
    bool hasPropertyUniversal(const QString& propertyName)
    {
        if (rootItem) {
            return rootItem->property(propertyName.toUtf8().constData()).isValid();
        } else if (rootObject) {
            return rootObject->property(propertyName.toUtf8().constData()).isValid();
        }
        return false;
    }
};

/**
 * @brief E2E tests for reusable QML components - LOADING ACTUAL COMPONENT FILES
 */
class ComponentTests : public ComponentTestFramework
{
    Q_OBJECT

private slots:
    void init() {
        consoleMessages.clear();
        consoleWarnings.clear();
        consoleErrors.clear();
    }

    void cleanup() {
        ComponentTestFramework::cleanup();
    }

    // ========================================================================
    // WButton.qml Tests - LOADING ACTUAL FILE
    // ========================================================================
    void testWButton_loadActualComponent()
    {
        QVERIFY2(loadActualComponent("WButton.qml"), "Failed to load ACTUAL WButton.qml component file");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("WButton - Actual Component Load");
    }

    void testWButton_properties()
    {
        QVERIFY(loadActualComponent("WButton.qml"));
        VERIFY_COMPONENT_LOADED();

        // WButton extends QtQuick.Controls.Button
        VERIFY_PROPERTY_EXISTS("text");
        VERIFY_PROPERTY_EXISTS("enabled");
        VERIFY_PROPERTY_EXISTS("font");

        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("WButton - Properties");
    }

    // ========================================================================
    // WComboBox.qml Tests - LOADING ACTUAL FILE
    // ========================================================================
    void testWComboBox_loadActualComponent()
    {
        QVERIFY2(loadActualComponent("WComboBox.qml"), "Failed to load ACTUAL WComboBox.qml component file");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("WComboBox - Actual Component Load");
    }

    void testWComboBox_properties()
    {
        QVERIFY(loadActualComponent("WComboBox.qml"));
        VERIFY_COMPONENT_LOADED();

        // Check custom properties
        VERIFY_PROPERTY_EXISTS("sizeToContents");
        VERIFY_PROPERTY_EXISTS("modelWidth");
        VERIFY_PROPERTY_EXISTS("font");

        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("WComboBox - Properties");
    }

    // ========================================================================
    // WComboBoxList.qml Tests - LOADING ACTUAL FILE (NEW)
    // ========================================================================
    void testWComboBoxList_loadActualComponent()
    {
        QVERIFY2(loadActualComponent("WComboBoxList.qml"), "Failed to load ACTUAL WComboBoxList.qml component file");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("WComboBoxList - Actual Component Load");
    }

    void testWComboBoxList_properties()
    {
        QVERIFY(loadActualComponent("WComboBoxList.qml"));
        VERIFY_COMPONENT_LOADED();

        // Check custom properties
        VERIFY_PROPERTY_EXISTS("sizeToContents");
        VERIFY_PROPERTY_EXISTS("modelWidth");
        VERIFY_PROPERTY_EXISTS("font");

        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("WComboBoxList - Properties");
    }

    // ========================================================================
    // WSwitch.qml Tests - LOADING ACTUAL FILE
    // ========================================================================
    void testWSwitch_loadActualComponent()
    {
        QVERIFY2(loadActualComponent("WSwitch.qml"), "Failed to load ACTUAL WSwitch.qml component file");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("WSwitch - Actual Component Load");
    }

    void testWSwitch_properties()
    {
        QVERIFY(loadActualComponent("WSwitch.qml"));
        VERIFY_COMPONENT_LOADED();

        VERIFY_PROPERTY_EXISTS("checked");
        VERIFY_PROPERTY_EXISTS("text");
        VERIFY_PROPERTY_EXISTS("font");

        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("WSwitch - Properties");
    }

    // ========================================================================
    // WDialog.qml Tests - LOADING ACTUAL FILE
    // ========================================================================
    void testWDialog_loadActualComponent()
    {
        QVERIFY2(loadActualComponent("WDialog.qml"), "Failed to load ACTUAL WDialog.qml component file");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("WDialog - Actual Component Load");
    }

    void testWDialog_properties()
    {
        QVERIFY(loadActualComponent("WDialog.qml"));
        VERIFY_COMPONENT_LOADED();

        VERIFY_PROPERTY_EXISTS("title");
        VERIFY_PROPERTY_EXISTS("modal");
        VERIFY_PROPERTY_EXISTS("visible");

        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("WDialog - Properties");
    }

    // ========================================================================
    // WMenu.qml Tests - LOADING ACTUAL FILE
    // ========================================================================
    void testWMenu_loadActualComponent()
    {
        QVERIFY2(loadActualComponent("WMenu.qml"), "Failed to load ACTUAL WMenu.qml component file");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("WMenu - Actual Component Load");
    }

    void testWMenu_properties()
    {
        QVERIFY(loadActualComponent("WMenu.qml"));
        VERIFY_COMPONENT_LOADED();

        VERIFY_PROPERTY_EXISTS("title");
        VERIFY_PROPERTY_EXISTS("sizeToContents");

        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("WMenu - Properties");
    }

    // ========================================================================
    // WSpectrum.qml Tests - LOADING ACTUAL FILE
    // ========================================================================
    void testWSpectrum_loadActualComponent()
    {
        QVERIFY2(loadActualComponent("WSpectrum.qml"), "Failed to load ACTUAL WSpectrum.qml component file");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("WSpectrum - Actual Component Load");
    }

    void testWSpectrum_properties()
    {
        QVERIFY(loadActualComponent("WSpectrum.qml"));
        VERIFY_COMPONENT_LOADED();

        VERIFY_PROPERTY_EXISTS("frequencyScale");
        VERIFY_PROPERTY_EXISTS("displayGrid");

        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("WSpectrum - Properties");
    }

    // ========================================================================
    // WToolTip.qml Tests - LOADING ACTUAL FILE
    // ========================================================================
    void testWToolTip_loadActualComponent()
    {
        QVERIFY2(loadActualComponent("WToolTip.qml"), "Failed to load ACTUAL WToolTip.qml component file");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("WToolTip - Actual Component Load");
    }

    void testWToolTip_properties()
    {
        QVERIFY(loadActualComponent("WToolTip.qml"));
        VERIFY_COMPONENT_LOADED();

        VERIFY_PROPERTY_EXISTS("text");
        VERIFY_PROPERTY_EXISTS("visible");
        VERIFY_PROPERTY_EXISTS("font");

        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("WToolTip - Properties");
    }

    // ========================================================================
    // WTumbler.qml Tests - LOADING ACTUAL FILE
    // ========================================================================
    void testWTumbler_loadActualComponent()
    {
        QVERIFY2(loadActualComponent("WTumbler.qml"), "Failed to load ACTUAL WTumbler.qml component file");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("WTumbler - Actual Component Load");
    }

    void testWTumbler_properties()
    {
        QVERIFY(loadActualComponent("WTumbler.qml"));
        VERIFY_COMPONENT_LOADED();

        VERIFY_PROPERTY_EXISTS("model");
        VERIFY_PROPERTY_EXISTS("currentIndex");
        VERIFY_PROPERTY_EXISTS("font");

        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("WTumbler - Properties");
    }

    // ========================================================================
    // MessagePopup.qml Tests - LOADING ACTUAL FILE
    // ========================================================================
    void testMessagePopup_loadActualComponent()
    {
        QVERIFY2(loadActualComponent("MessagePopup.qml"), "Failed to load ACTUAL MessagePopup.qml component file");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("MessagePopup - Actual Component Load");
    }

    void testMessagePopup_properties()
    {
        QVERIFY(loadActualComponent("MessagePopup.qml"));
        VERIFY_COMPONENT_LOADED();

        VERIFY_PROPERTY_EXISTS("text");
        VERIFY_PROPERTY_EXISTS("color");
        VERIFY_PROPERTY_EXISTS("visible");

        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("MessagePopup - Properties");
    }

    // ========================================================================
    // AnnouncementIndicator.qml Tests - LOADING ACTUAL FILE
    // ========================================================================
    void testAnnouncementIndicator_loadActualComponent()
    {
        QVERIFY2(loadActualComponent("AnnouncementIndicator.qml"), "Failed to load ACTUAL AnnouncementIndicator.qml component file");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("AnnouncementIndicator - Actual Component Load");
    }

    void testAnnouncementIndicator_properties()
    {
        QVERIFY(loadActualComponent("AnnouncementIndicator.qml"));
        VERIFY_COMPONENT_LOADED();

        VERIFY_PROPERTY_EXISTS("active");
        VERIFY_PROPERTY_EXISTS("announcementType");
        VERIFY_PROPERTY_EXISTS("color");

        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("AnnouncementIndicator - Properties");
    }

    // ========================================================================
    // AnnouncementHistory.qml Tests - LOADING ACTUAL FILE
    // ========================================================================
    void testAnnouncementHistory_loadActualComponent()
    {
        QVERIFY2(loadActualComponent("AnnouncementHistory.qml"), "Failed to load ACTUAL AnnouncementHistory.qml component file");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("AnnouncementHistory - Actual Component Load");
    }

    void testAnnouncementHistory_properties()
    {
        QVERIFY(loadActualComponent("AnnouncementHistory.qml"));
        VERIFY_COMPONENT_LOADED();

        VERIFY_PROPERTY_EXISTS("announcements");
        VERIFY_PROPERTY_EXISTS("maxHistory");

        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("AnnouncementHistory - Properties");
    }

    // ========================================================================
    // StationDelegate.qml Tests - LOADING ACTUAL FILE
    // ========================================================================
    void testStationDelegate_loadActualComponent()
    {
        QVERIFY2(loadActualComponent("StationDelegate.qml"), "Failed to load ACTUAL StationDelegate.qml component file");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("StationDelegate - Actual Component Load");
    }

    void testStationDelegate_properties()
    {
        QVERIFY(loadActualComponent("StationDelegate.qml"));
        VERIFY_COMPONENT_LOADED();

        VERIFY_PROPERTY_EXISTS("stationName");
        VERIFY_PROPERTY_EXISTS("isFavorite");
        VERIFY_PROPERTY_EXISTS("signalStrength");

        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("StationDelegate - Properties");
    }

    // ========================================================================
    // StationListModel.qml Tests - LOADING ACTUAL FILE (NEW)
    // ========================================================================
    void testStationListModel_loadActualComponent()
    {
        QVERIFY2(loadActualComponent("StationListModel.qml"), "Failed to load ACTUAL StationListModel.qml component file");
        // StationListModel is a ListModel, which is a non-visual component
        QVERIFY2(rootObject != nullptr || rootItem != nullptr, "StationListModel failed to load");
        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("StationListModel - Actual Component Load");
    }

    void testStationListModel_properties()
    {
        QVERIFY(loadActualComponent("StationListModel.qml"));

        // Check custom properties
        QVERIFY2(hasPropertyUniversal("serialized"), "Property 'serialized' does not exist");
        QVERIFY2(hasPropertyUniversal("type"), "Property 'type' does not exist");

        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("StationListModel - Properties");
    }

    // ========================================================================
    // SettingSection.qml Tests - LOADING ACTUAL FILE (NEW)
    // ========================================================================
    void testSettingSection_loadActualComponent()
    {
        QVERIFY2(loadActualComponent("SettingSection.qml"), "Failed to load ACTUAL SettingSection.qml component file");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("SettingSection - Actual Component Load");
    }

    void testSettingSection_properties()
    {
        QVERIFY(loadActualComponent("SettingSection.qml"));
        VERIFY_COMPONENT_LOADED();

        // Check aliased properties
        VERIFY_PROPERTY_EXISTS("isNotFirst");
        VERIFY_PROPERTY_EXISTS("text");

        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("SettingSection - Properties");
    }

    // ========================================================================
    // Units.qml Tests - LOADING ACTUAL FILE (NEW)
    // ========================================================================
    void testUnits_loadActualComponent()
    {
        QVERIFY2(loadActualComponent("Units.qml"), "Failed to load ACTUAL Units.qml component file");
        // Units is a singleton QtObject (non-visual)
        QVERIFY2(rootObject != nullptr || rootItem != nullptr, "Units singleton failed to load");
        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("Units - Actual Singleton Load");
    }

    void testUnits_functions()
    {
        QVERIFY(loadActualComponent("Units.qml"));

        // Units provides dp() and em() functions
        // These are QML functions, so we can't directly test them from C++
        // But we can verify the object loaded successfully
        QVERIFY2(rootObject != nullptr || rootItem != nullptr, "Units singleton should be loaded");

        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("Units - Functions Available");
    }

    // ========================================================================
    // ViewBaseFrame.qml Tests - LOADING ACTUAL FILE (NEW)
    // ========================================================================
    void testViewBaseFrame_loadActualComponent()
    {
        QVERIFY2(loadActualComponent("ViewBaseFrame.qml"), "Failed to load ACTUAL ViewBaseFrame.qml component file");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("ViewBaseFrame - Actual Component Load");
    }

    void testViewBaseFrame_properties()
    {
        QVERIFY(loadActualComponent("ViewBaseFrame.qml"));
        VERIFY_COMPONENT_LOADED();

        // Check custom properties
        VERIFY_PROPERTY_EXISTS("labelText");
        VERIFY_PROPERTY_EXISTS("sourcePath");
        VERIFY_PROPERTY_EXISTS("isExpert");
        VERIFY_PROPERTY_EXISTS("isMaximize");

        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("ViewBaseFrame - Properties");
    }

    void testViewBaseFrame_signals()
    {
        QVERIFY(loadActualComponent("ViewBaseFrame.qml"));
        VERIFY_COMPONENT_LOADED();

        // Verify signals exist
        const QMetaObject* metaObj = rootItem->metaObject();
        bool hasRequestPositionChange = false;
        bool hasRequestMaximize = false;
        bool hasItemRemove = false;

        for (int i = metaObj->methodOffset(); i < metaObj->methodCount(); ++i) {
            QMetaMethod method = metaObj->method(i);
            if (method.methodType() == QMetaMethod::Signal) {
                QString sig = QString::fromLatin1(method.methodSignature());
                if (sig.startsWith("requestPositionChange")) hasRequestPositionChange = true;
                if (sig.startsWith("requestMaximize")) hasRequestMaximize = true;
                if (sig.startsWith("itemRemove")) hasItemRemove = true;
            }
        }

        QVERIFY2(hasRequestPositionChange, "requestPositionChange signal not found");
        QVERIFY2(hasRequestMaximize, "requestMaximize signal not found");
        QVERIFY2(hasItemRemove, "itemRemove signal not found");

        VERIFY_ZERO_CONSOLE_ERRORS();
        printReport("ViewBaseFrame - Signals");
    }
};

// Static member initialization (required for QMLComponentTestBase)
QtMessageHandler QMLComponentTestBase::originalHandler = nullptr;
QMLComponentTestBase* QMLComponentTestBase::currentTest = nullptr;

QTEST_MAIN(ComponentTests)
#include "test_components_FIXED.moc"
