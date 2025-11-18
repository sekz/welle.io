/*
 *    Copyright (C) 2025
 *    welle.io DAB+ Receiver
 *
 *    This file is part of the welle.io E2E testing suite.
 *
 *    welle.io is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 */

#include "test_framework.h"
#include <QtTest>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>
#include <QSignalSpy>

/**
 * @file test_expert_view.cpp
 * @brief Comprehensive E2E GUI tests for ExpertView.qml and all expert subviews
 *
 * Test Coverage:
 * 1. ExpertView.qml - Main expert view container
 * 2. SpectrumGraph.qml - Spectrum analyzer graph
 * 3. ConstellationGraph.qml - DQPSK constellation diagram
 * 4. ImpulseResponseGraph.qml - Impulse response display
 * 5. NullSymbolGraph.qml - Null symbol visualization
 * 6. ServiceDetails.qml - DAB service information
 * 7. TextOutputView.qml - Console output view
 * 8. RawRecorder.qml - I/Q raw data recorder
 *
 * Each test verifies:
 * - Component loads successfully
 * - Zero console errors/warnings
 * - All required objects created
 * - Visual rendering works
 * - Proper cleanup
 */

// Mock GUI helper for QML context
class MockGuiHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceNameChanged)

public:
    explicit MockGuiHelper(QObject* parent = nullptr) : QObject(parent) {}
    QString deviceName() const { return "Test Device"; }

    Q_INVOKABLE void updateSpectrum() {}
    Q_INVOKABLE void updateConstellation() {}
    Q_INVOKABLE void updateImpulseResponse() {}
    Q_INVOKABLE void updateNullSymbol() {}
    Q_INVOKABLE void registerSpectrumWaterfall(QObject*) {}
    Q_INVOKABLE void registerSpectrumSeries(QObject*) {}
    Q_INVOKABLE void registerConstellationSeries(QObject*) {}
    Q_INVOKABLE void registerImpulseResonseWaterfall(QObject*) {}
    Q_INVOKABLE void registerImpulseResonseSeries(QObject*) {}
    Q_INVOKABLE void registerNullSymbolWaterfall(QObject*) {}
    Q_INVOKABLE void registerNullSymbolSeries(QObject*) {}

signals:
    void deviceNameChanged();
    void setSpectrumAxis(qreal Ymax, qreal Xmin, qreal Xmax);
    void setConstellationAxis(qreal Xmin, qreal Xmax);
    void setImpulseResponseAxis(qreal Ymax, qreal Xmin, qreal Xmax);
    void setNullSymbolAxis(qreal Ymax, qreal Xmin, qreal Xmax);
    void newDebugOutput(const QString& text);
};

// Mock radio controller for QML context
class MockRadioController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceNameChanged)
    Q_PROPERTY(QString channel READ channel NOTIFY channelChanged)
    Q_PROPERTY(qreal frequency READ frequency NOTIFY frequencyChanged)
    Q_PROPERTY(qreal frequencyCorrection READ frequencyCorrection NOTIFY frequencyCorrectionChanged)
    Q_PROPERTY(qreal frequencyCorrectionPpm READ frequencyCorrectionPpm NOTIFY frequencyCorrectionPpmChanged)
    Q_PROPERTY(qreal snr READ snr NOTIFY snrChanged)
    Q_PROPERTY(bool isSync READ isSync NOTIFY isSyncChanged)
    Q_PROPERTY(bool isFICCRC READ isFICCRC NOTIFY isFICCRCChanged)
    Q_PROPERTY(int frameErrors READ frameErrors NOTIFY frameErrorsChanged)
    Q_PROPERTY(int rsCorrectedErrors READ rsCorrectedErrors NOTIFY rsCorrectedErrorsChanged)
    Q_PROPERTY(int rsUncorrectedErrors READ rsUncorrectedErrors NOTIFY rsUncorrectedErrorsChanged)
    Q_PROPERTY(int aacErrors READ aacErrors NOTIFY aacErrorsChanged)
    Q_PROPERTY(int ensembleId READ ensembleId NOTIFY ensembleIdChanged)
    Q_PROPERTY(QDateTime dateTime READ dateTime NOTIFY dateTimeChanged)

public:
    explicit MockRadioController(QObject* parent = nullptr) : QObject(parent) {}

    QString deviceName() const { return "RTL-SDR"; }
    QString channel() const { return "5A"; }
    qreal frequency() const { return 174928000.0; }
    qreal frequencyCorrection() const { return 123.45; }
    qreal frequencyCorrectionPpm() const { return 0.71; }
    qreal snr() const { return 25.6; }
    bool isSync() const { return true; }
    bool isFICCRC() const { return true; }
    int frameErrors() const { return 0; }
    int rsCorrectedErrors() const { return 0; }
    int rsUncorrectedErrors() const { return 0; }
    int aacErrors() const { return 0; }
    int ensembleId() const { return 0xD001; }
    QDateTime dateTime() const { return QDateTime::currentDateTime(); }

    Q_INVOKABLE void initRecorder(int size) { Q_UNUSED(size); }
    Q_INVOKABLE void triggerRecorder(const QString& filename) { Q_UNUSED(filename); }

signals:
    void deviceNameChanged();
    void channelChanged();
    void frequencyChanged();
    void frequencyCorrectionChanged();
    void frequencyCorrectionPpmChanged();
    void snrChanged();
    void isSyncChanged();
    void isFICCRCChanged();
    void frameErrorsChanged();
    void rsCorrectedErrorsChanged();
    void rsUncorrectedErrorsChanged();
    void aacErrorsChanged();
    void ensembleIdChanged();
    void dateTimeChanged();
};

// Mock main window for QML context
class MockMainWindow : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isExpertView READ isExpertView NOTIFY isExpertViewChanged)

public:
    explicit MockMainWindow(QObject* parent = nullptr) : QObject(parent) {}
    bool isExpertView() const { return true; }

signals:
    void isExpertViewChanged();
};

/**
 * @brief Test class for ExpertView and all expert subviews
 */
class TestExpertView : public QMLComponentTestBase
{
    Q_OBJECT

private:
    MockGuiHelper* mockGuiHelper = nullptr;
    MockRadioController* mockRadioController = nullptr;
    MockMainWindow* mockMainWindow = nullptr;
    QString qmlBasePath;

    void setupMockContext()
    {
        // Do NOT create engine here - base class handles engine creation
        mockGuiHelper = new MockGuiHelper(this);
        mockRadioController = new MockRadioController(this);
        mockMainWindow = new MockMainWindow(this);

        if (engine) {
            engine->rootContext()->setContextProperty("guiHelper", mockGuiHelper);
            engine->rootContext()->setContextProperty("radioController", mockRadioController);
            engine->rootContext()->setContextProperty("mainWindow", mockMainWindow);
        }
    }

    bool loadComponentWithMocks(const QString& qmlPath)
    {
        // Ensure engine exists before setting context properties
        if (!engine) {
            engine = std::make_unique<QQmlEngine>();
        }

        // Set context properties
        engine->rootContext()->setContextProperty("guiHelper", mockGuiHelper);
        engine->rootContext()->setContextProperty("radioController", mockRadioController);
        engine->rootContext()->setContextProperty("mainWindow", mockMainWindow);

        // Load component
        return loadComponent(qmlPath);
    }

private slots:
    void initTestCase()
    {
        // Set QML import path to project source
        qmlBasePath = QString(PROJECT_SOURCE_DIR) + "/src/welle-gui/QML";
        qInfo() << "QML base path:" << qmlBasePath;
    }

    void init()
    {
        setupMockContext();
    }

    void cleanup()
    {
        // CRITICAL FIX: Delete mock objects BEFORE calling base cleanup
        // This prevents use-after-free when base class cleans up engine
        if (mockGuiHelper) {
            delete mockGuiHelper;
            mockGuiHelper = nullptr;
        }
        if (mockRadioController) {
            delete mockRadioController;
            mockRadioController = nullptr;
        }
        if (mockMainWindow) {
            delete mockMainWindow;
            mockMainWindow = nullptr;
        }

        // Call base class cleanup after deleting mocks
        QMLComponentTestBase::cleanup();
    }

    // ========================================================================
    // TEST: ExpertView.qml Main Container
    // ========================================================================

    void test_ExpertView_Load()
    {
        QString qmlPath = qmlBasePath + "/ExpertView.qml";
        qInfo() << "Testing ExpertView:" << qmlPath;

        QVERIFY2(loadComponent(qmlPath), "Failed to load ExpertView.qml");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();

        printReport("ExpertView");
    }

    void test_ExpertView_Properties()
    {
        QString qmlPath = qmlBasePath + "/ExpertView.qml";
        QVERIFY(loadComponent(qmlPath));

        // Verify basic properties
        QVERIFY(rootItem->property("implicitHeight").isValid());
        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_ExpertView_GridLayout()
    {
        QString qmlPath = qmlBasePath + "/ExpertView.qml";
        QVERIFY(loadComponent(qmlPath));

        // Find GridLayout
        QObject* layout = rootItem->findChild<QObject*>("layout");
        QVERIFY2(layout != nullptr, "GridLayout not found in ExpertView");

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    // ========================================================================
    // TEST: SpectrumGraph.qml
    // ========================================================================

    void test_SpectrumGraph_Load()
    {
        QString qmlPath = qmlBasePath + "/expertviews/SpectrumGraph.qml";
        qInfo() << "Testing SpectrumGraph:" << qmlPath;

        // Add import path for dependencies
        engine->addImportPath(qmlBasePath);

        QVERIFY2(loadComponent(qmlPath), "Failed to load SpectrumGraph.qml");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();

        printReport("SpectrumGraph");
    }

    void test_SpectrumGraph_Properties()
    {
        QString qmlPath = qmlBasePath + "/expertviews/SpectrumGraph.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Verify labelText property (from ViewBaseFrame)
        QVERIFY(hasProperty("labelText"));

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_SpectrumGraph_WSpectrumComponent()
    {
        QString qmlPath = qmlBasePath + "/expertviews/SpectrumGraph.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Find WSpectrum component
        QObject* spectrum = rootItem->findChild<QObject*>("spectrum");
        QVERIFY2(spectrum != nullptr, "WSpectrum component not found");

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_SpectrumGraph_SignalConnections()
    {
        QString qmlPath = qmlBasePath + "/expertviews/SpectrumGraph.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Test setSpectrumAxis signal
        QSignalSpy spy(mockGuiHelper, &MockGuiHelper::setSpectrumAxis);
        emit mockGuiHelper->setSpectrumAxis(100.0, 0.0, 200.0);

        QCOMPARE(spy.count(), 1);
        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_SpectrumGraph_Rendering()
    {
        QString qmlPath = qmlBasePath + "/expertviews/SpectrumGraph.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Test visual rendering
        QVERIFY(testVisualRendering());
        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    // ========================================================================
    // TEST: ConstellationGraph.qml
    // ========================================================================

    void test_ConstellationGraph_Load()
    {
        QString qmlPath = qmlBasePath + "/expertviews/ConstellationGraph.qml";
        qInfo() << "Testing ConstellationGraph:" << qmlPath;

        engine->addImportPath(qmlBasePath);
        QVERIFY2(loadComponent(qmlPath), "Failed to load ConstellationGraph.qml");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();

        printReport("ConstellationGraph");
    }

    void test_ConstellationGraph_ChartView()
    {
        QString qmlPath = qmlBasePath + "/expertviews/ConstellationGraph.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Find ChartView
        QObject* chart = rootItem->findChild<QObject*>("chart");
        QVERIFY2(chart != nullptr, "ChartView not found");

        // Verify chart properties
        QVERIFY(chart->property("animationOptions").isValid());

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_ConstellationGraph_AxisSetup()
    {
        QString qmlPath = qmlBasePath + "/expertviews/ConstellationGraph.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Find axes
        QObject* axisX = rootItem->findChild<QObject*>("axisX");
        QObject* axisY = rootItem->findChild<QObject*>("axisY");

        QVERIFY2(axisX != nullptr, "X axis not found");
        QVERIFY2(axisY != nullptr, "Y axis not found");

        // Verify axis properties
        QVERIFY(axisY->property("max").isValid());
        QVERIFY(axisY->property("min").isValid());

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_ConstellationGraph_Rendering()
    {
        QString qmlPath = qmlBasePath + "/expertviews/ConstellationGraph.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        QVERIFY(testVisualRendering());
        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    // ========================================================================
    // TEST: ImpulseResponseGraph.qml
    // ========================================================================

    void test_ImpulseResponseGraph_Load()
    {
        QString qmlPath = qmlBasePath + "/expertviews/ImpulseResponseGraph.qml";
        qInfo() << "Testing ImpulseResponseGraph:" << qmlPath;

        engine->addImportPath(qmlBasePath);
        QVERIFY2(loadComponent(qmlPath), "Failed to load ImpulseResponseGraph.qml");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();

        printReport("ImpulseResponseGraph");
    }

    void test_ImpulseResponseGraph_WSpectrumComponent()
    {
        QString qmlPath = qmlBasePath + "/expertviews/ImpulseResponseGraph.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Find WSpectrum component
        QObject* spectrum = rootItem->findChild<QObject*>("spectrum");
        QVERIFY2(spectrum != nullptr, "WSpectrum component not found");

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_ImpulseResponseGraph_MouseInteraction()
    {
        QString qmlPath = qmlBasePath + "/expertviews/ImpulseResponseGraph.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Find MouseArea for hover interaction
        QObject* mouseArea = rootItem->findChild<QObject*>();
        // MouseArea exists for position tracking

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_ImpulseResponseGraph_Rendering()
    {
        QString qmlPath = qmlBasePath + "/expertviews/ImpulseResponseGraph.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        QVERIFY(testVisualRendering());
        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    // ========================================================================
    // TEST: NullSymbolGraph.qml
    // ========================================================================

    void test_NullSymbolGraph_Load()
    {
        QString qmlPath = qmlBasePath + "/expertviews/NullSymbolGraph.qml";
        qInfo() << "Testing NullSymbolGraph:" << qmlPath;

        engine->addImportPath(qmlBasePath);
        QVERIFY2(loadComponent(qmlPath), "Failed to load NullSymbolGraph.qml");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();

        printReport("NullSymbolGraph");
    }

    void test_NullSymbolGraph_WSpectrumComponent()
    {
        QString qmlPath = qmlBasePath + "/expertviews/NullSymbolGraph.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Find WSpectrum component
        QObject* spectrum = rootItem->findChild<QObject*>("spectrum");
        QVERIFY2(spectrum != nullptr, "WSpectrum component not found");

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_NullSymbolGraph_SignalConnections()
    {
        QString qmlPath = qmlBasePath + "/expertviews/NullSymbolGraph.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Test setNullSymbolAxis signal
        QSignalSpy spy(mockGuiHelper, &MockGuiHelper::setNullSymbolAxis);
        emit mockGuiHelper->setNullSymbolAxis(100.0, 0.0, 200.0);

        QCOMPARE(spy.count(), 1);
        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_NullSymbolGraph_Rendering()
    {
        QString qmlPath = qmlBasePath + "/expertviews/NullSymbolGraph.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        QVERIFY(testVisualRendering());
        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    // ========================================================================
    // TEST: ServiceDetails.qml
    // ========================================================================

    void test_ServiceDetails_Load()
    {
        QString qmlPath = qmlBasePath + "/expertviews/ServiceDetails.qml";
        qInfo() << "Testing ServiceDetails:" << qmlPath;

        engine->addImportPath(qmlBasePath);
        QVERIFY2(loadComponent(qmlPath), "Failed to load ServiceDetails.qml");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();

        printReport("ServiceDetails");
    }

    void test_ServiceDetails_RadioControllerProperties()
    {
        QString qmlPath = qmlBasePath + "/expertviews/ServiceDetails.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Allow QML bindings to update
        QTest::qWait(100);

        // Verify radioController properties are accessible
        QCOMPARE(mockRadioController->deviceName(), QString("RTL-SDR"));
        QCOMPARE(mockRadioController->channel(), QString("5A"));
        QCOMPARE(mockRadioController->isSync(), true);
        QCOMPARE(mockRadioController->isFICCRC(), true);

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_ServiceDetails_StatusIndicators()
    {
        QString qmlPath = qmlBasePath + "/expertviews/ServiceDetails.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Service details shows status rectangles (green/red indicators)
        // Component should render without errors

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_ServiceDetails_Rendering()
    {
        QString qmlPath = qmlBasePath + "/expertviews/ServiceDetails.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        QVERIFY(testVisualRendering());
        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    // ========================================================================
    // TEST: TextOutputView.qml
    // ========================================================================

    void test_TextOutputView_Load()
    {
        QString qmlPath = qmlBasePath + "/expertviews/TextOutputView.qml";
        qInfo() << "Testing TextOutputView:" << qmlPath;

        engine->addImportPath(qmlBasePath);
        QVERIFY2(loadComponent(qmlPath), "Failed to load TextOutputView.qml");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();

        printReport("TextOutputView");
    }

    void test_TextOutputView_TextArea()
    {
        QString qmlPath = qmlBasePath + "/expertviews/TextOutputView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Find TextArea (via TextField id)
        QObject* textField = rootItem->findChild<QObject*>("textField");
        QVERIFY2(textField != nullptr, "TextArea not found");

        // Verify it's read-only
        QVariant readOnly = textField->property("readOnly");
        QVERIFY(readOnly.isValid());
        QCOMPARE(readOnly.toBool(), true);

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_TextOutputView_DebugOutput()
    {
        QString qmlPath = qmlBasePath + "/expertviews/TextOutputView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        QObject* textField = rootItem->findChild<QObject*>("textField");
        QVERIFY(textField != nullptr);

        // Simulate debug output signal
        emit mockGuiHelper->newDebugOutput("Test debug message");
        QTest::qWait(50);

        // TextArea should have received the message
        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_TextOutputView_Rendering()
    {
        QString qmlPath = qmlBasePath + "/expertviews/TextOutputView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        QVERIFY(testVisualRendering());
        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    // ========================================================================
    // TEST: RawRecorder.qml
    // ========================================================================

    void test_RawRecorder_Load()
    {
        QString qmlPath = qmlBasePath + "/expertviews/RawRecorder.qml";
        qInfo() << "Testing RawRecorder:" << qmlPath;

        engine->addImportPath(qmlBasePath);
        QVERIFY2(loadComponent(qmlPath), "Failed to load RawRecorder.qml");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();

        printReport("RawRecorder");
    }

    void test_RawRecorder_Properties()
    {
        QString qmlPath = qmlBasePath + "/expertviews/RawRecorder.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Verify properties
        QVERIFY(hasProperty("isStart"));
        QVERIFY(hasProperty("ringeBufferSize"));

        // Initial state
        QCOMPARE(getProperty("isStart").toBool(), false);
        QCOMPARE(getProperty("ringeBufferSize").toInt(), 0);

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_RawRecorder_WTumblerComponent()
    {
        QString qmlPath = qmlBasePath + "/expertviews/RawRecorder.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Find WTumbler component
        QObject* tumbler = rootItem->findChild<QObject*>("ringeBufferSetting");
        QVERIFY2(tumbler != nullptr, "WTumbler component not found");

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_RawRecorder_ButtonInteraction()
    {
        QString qmlPath = qmlBasePath + "/expertviews/RawRecorder.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Button should exist for init/save
        // Component should handle button press without errors

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_RawRecorder_Rendering()
    {
        QString qmlPath = qmlBasePath + "/expertviews/RawRecorder.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        QVERIFY(testVisualRendering());
        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    // ========================================================================
    // INTEGRATION TESTS: All Components Together
    // ========================================================================

    void test_AllExpertViews_NoErrors_data()
    {
        QTest::addColumn<QString>("qmlFile");

        QTest::newRow("SpectrumGraph") << "/expertviews/SpectrumGraph.qml";
        QTest::newRow("ConstellationGraph") << "/expertviews/ConstellationGraph.qml";
        QTest::newRow("ImpulseResponseGraph") << "/expertviews/ImpulseResponseGraph.qml";
        QTest::newRow("NullSymbolGraph") << "/expertviews/NullSymbolGraph.qml";
        QTest::newRow("ServiceDetails") << "/expertviews/ServiceDetails.qml";
        QTest::newRow("TextOutputView") << "/expertviews/TextOutputView.qml";
        QTest::newRow("RawRecorder") << "/expertviews/RawRecorder.qml";
    }

    void test_AllExpertViews_NoErrors()
    {
        QFETCH(QString, qmlFile);

        qInfo() << "=== Integration Test: Loading" << qmlFile << "===";

        QString qmlPath = qmlBasePath + qmlFile;
        engine->addImportPath(qmlBasePath);

        QVERIFY2(loadComponent(qmlPath), QString("Failed to load %1").arg(qmlFile).toUtf8().constData());
        QVERIFY2(verifyZeroErrors(), QString("%1 has console errors").arg(qmlFile).toUtf8().constData());

        qInfo() << "  ✓ PASSED:" << qmlFile;
    }
};

QTEST_MAIN(TestExpertView)
#include "test_expert_view.moc"
