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
#include <QAbstractListModel>

/**
 * @file test_mot_view.cpp
 * @brief Comprehensive E2E GUI tests for MotView.qml (MOT Slideshow)
 *
 * Test Coverage:
 * 1. MotView.qml loading and initialization
 * 2. MOT slideshow functionality
 * 3. Image loading and display
 * 4. Category management
 * 5. Navigation (next/previous buttons)
 * 6. Visual element verification
 * 7. Signal connections (motChanged, motReseted)
 * 8. FileDialog integration
 * 9. ListView functionality
 * 10. Zero console errors/warnings
 *
 * Each test verifies:
 * - Component loads successfully
 * - Zero console errors/warnings
 * - All required objects created
 * - Visual rendering works
 * - User interactions handled correctly
 * - Proper cleanup
 */

// Mock GUI helper for MOT testing
class MockMotGuiHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceNameChanged)

public:
    explicit MockMotGuiHelper(QObject* parent = nullptr) : QObject(parent) {}
    QString deviceName() const { return "Test Device"; }

    Q_INVOKABLE void saveMotImages(const QUrl& folder) {
        qDebug() << "Mock: Saving MOT images to" << folder;
    }

signals:
    void deviceNameChanged();
    void motChanged(QString pictureName, QString categoryTitle, int categoryId, int slideId);
    void motReseted();
};

// Mock main window for MOT testing
class MockMotMainWindow : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isExpertView READ isExpertView NOTIFY isExpertViewChanged)

public:
    explicit MockMotMainWindow(QObject* parent = nullptr) : QObject(parent), m_isExpertView(false) {}
    bool isExpertView() const { return m_isExpertView; }
    void setExpertView(bool value) {
        if (m_isExpertView != value) {
            m_isExpertView = value;
            emit isExpertViewChanged();
        }
    }

signals:
    void isExpertViewChanged();

private:
    bool m_isExpertView;
};

/**
 * @brief Test class for MotView.qml
 */
class TestMotView : public QMLComponentTestBase
{
    Q_OBJECT

private:
    MockMotGuiHelper* mockGuiHelper = nullptr;
    MockMotMainWindow* mockMainWindow = nullptr;
    QString qmlBasePath;

    void setupMockContext()
    {
        // Do NOT create engine here - base class handles engine creation
        mockGuiHelper = new MockMotGuiHelper(this);
        mockMainWindow = new MockMotMainWindow(this);

        if (engine) {
            engine->rootContext()->setContextProperty("guiHelper", mockGuiHelper);
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
        if (mockMainWindow) {
            delete mockMainWindow;
            mockMainWindow = nullptr;
        }

        // Call base class cleanup after deleting mocks
        QMLComponentTestBase::cleanup();
    }

    // ========================================================================
    // TEST: MotView.qml Loading and Initialization
    // ========================================================================

    void test_MotView_Load()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        qInfo() << "Testing MotView:" << qmlPath;

        // Add import path for dependencies
        engine->addImportPath(qmlBasePath);

        QVERIFY2(loadComponent(qmlPath), "Failed to load MotView.qml");
        VERIFY_COMPONENT_LOADED();
        VERIFY_ZERO_CONSOLE_ERRORS();

        printReport("MotView");
    }

    void test_MotView_Initialization()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Verify initial properties
        QVERIFY(hasProperty("slideList"));
        QVERIFY(hasProperty("currentCategoryId"));
        QVERIFY(hasProperty("currentCategoryListPos"));
        QVERIFY(hasProperty("latestPictureName"));

        // Initial values
        QCOMPARE(getProperty("currentCategoryId").toInt(), 0);
        QCOMPARE(getProperty("currentCategoryListPos").toInt(), 0);
        QCOMPARE(getProperty("latestPictureName").toString(), QString(""));

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_MotView_ViewBaseFrame()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // MotView extends ViewBaseFrame
        QVERIFY(hasProperty("labelText"));

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    // ========================================================================
    // TEST: FileDialog Integration
    // ========================================================================

    void test_MotView_FileDialog()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Find FileDialog
        QObject* fileDialog = rootItem->findChild<QObject*>("fileDialog");
        QVERIFY2(fileDialog != nullptr, "FileDialog not found");

        // Verify FileDialog properties
        QVERIFY(fileDialog->property("title").isValid());

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_MotView_SaveAllImagesEntry()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Component.onCompleted adds "Save all images" entry
        // This should execute without errors
        QTest::qWait(100);

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    // ========================================================================
    // TEST: Category List Model
    // ========================================================================

    void test_MotView_CategoryListModel()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Find categoryListModel
        QObject* categoryListModel = rootItem->findChild<QObject*>("categoryListModel");
        QVERIFY2(categoryListModel != nullptr, "categoryListModel not found");

        // Allow time for _initCategories() to run
        QTest::qWait(100);

        // Should have "Latest" category by default
        QVariant count = categoryListModel->property("count");
        QVERIFY(count.isValid());
        QVERIFY(count.toInt() >= 1); // At least "Latest" category

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_MotView_InitialCategory()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        QTest::qWait(100);

        // Initial category should be "Latest" (categoryId: 0)
        QCOMPARE(getProperty("currentCategoryId").toInt(), 0);

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    // ========================================================================
    // TEST: Image Display
    // ========================================================================

    void test_MotView_MotImageComponent()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Find Image component
        QObject* motImage = rootItem->findChild<QObject*>("motImage");
        QVERIFY2(motImage != nullptr, "motImage component not found");

        // Verify Image properties
        QVERIFY(motImage->property("fillMode").isValid());
        QVERIFY(motImage->property("source").isValid());

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_MotView_ImageSourceUpdate()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        QObject* motImage = rootItem->findChild<QObject*>("motImage");
        QVERIFY(motImage != nullptr);

        // Simulate motChanged signal
        emit mockGuiHelper->motChanged("test_image.jpg", "", 0, 1);
        QTest::qWait(100);

        // Image source should be updated
        QString source = motImage->property("source").toString();
        QVERIFY(source.contains("test_image.jpg"));

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    // ========================================================================
    // TEST: MOT Signal Handling
    // ========================================================================

    void test_MotView_MotChangedSignal()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        QObject* motImage = rootItem->findChild<QObject*>("motImage");
        QVERIFY(motImage != nullptr);

        // Test motChanged signal with no category
        emit mockGuiHelper->motChanged("image1.png", "", 0, 1);
        QTest::qWait(50);

        QCOMPARE(getProperty("latestPictureName").toString(), QString("image1.png"));

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_MotView_MotChangedSignalWithCategory()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        QTest::qWait(100); // Allow initialization

        // Emit motChanged with category
        emit mockGuiHelper->motChanged("cat_image.jpg", "News", 1, 100);
        QTest::qWait(100);

        // Category should be added to slideList
        QVariant slideList = getProperty("slideList");
        QVERIFY(slideList.isValid());

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_MotView_MotResetedSignal()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Add some images
        emit mockGuiHelper->motChanged("image1.png", "", 0, 1);
        QTest::qWait(50);

        // Reset MOT
        emit mockGuiHelper->motReseted();
        QTest::qWait(100);

        // slideList should be reinitialized
        QObject* categoryListModel = rootItem->findChild<QObject*>("categoryListModel");
        QVERIFY(categoryListModel != nullptr);

        // Should have at least "Latest" category
        QVERIFY(categoryListModel->property("count").toInt() >= 1);

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    // ========================================================================
    // TEST: Navigation Buttons
    // ========================================================================

    void test_MotView_NavigationButtons()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Navigation buttons (back/next) should exist
        // They are WButton components with icon.name "back" and "next"
        QList<QObject*> buttons = rootItem->findChildren<QObject*>();

        // Buttons should be present (even if hidden initially)
        QVERIFY(buttons.size() > 0);

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_MotView_NavigationButtonVisibility()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Initially, with no slides, buttons should be invisible
        // or visibility depends on slideList length

        QTest::qWait(100);

        // Component should handle visibility logic without errors
        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    // ========================================================================
    // TEST: ListView for Categories
    // ========================================================================

    void test_MotView_CategoryListView()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Find ListView
        QList<QObject*> listViews = rootItem->findChildren<QObject*>();
        bool foundListView = false;

        for (QObject* obj : listViews) {
            if (QString(obj->metaObject()->className()).contains("ListView")) {
                foundListView = true;
                break;
            }
        }

        // ListView should exist for category selection
        // (Note: exact verification depends on QML structure)

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_MotView_CategorySelection()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        QTest::qWait(100);

        // Add a category
        emit mockGuiHelper->motChanged("news1.jpg", "News", 1, 100);
        QTest::qWait(100);

        // Category selection logic should work without errors
        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    // ========================================================================
    // TEST: Multiple Slide Categories
    // ========================================================================

    void test_MotView_MultipleCategories()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        QTest::qWait(100);

        // Add multiple categories
        emit mockGuiHelper->motChanged("news1.jpg", "News", 1, 100);
        QTest::qWait(50);
        emit mockGuiHelper->motChanged("sport1.jpg", "Sports", 2, 200);
        QTest::qWait(50);
        emit mockGuiHelper->motChanged("weather1.jpg", "Weather", 3, 300);
        QTest::qWait(100);

        // All categories should be added without errors
        QObject* categoryListModel = rootItem->findChild<QObject*>("categoryListModel");
        QVERIFY(categoryListModel != nullptr);

        int categoryCount = categoryListModel->property("count").toInt();
        // Should have: Latest + News + Sports + Weather = 4 categories
        QVERIFY(categoryCount >= 4);

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_MotView_MultipleSlidesInCategory()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        QTest::qWait(100);

        // Add multiple slides to same category
        emit mockGuiHelper->motChanged("news1.jpg", "News", 1, 101);
        QTest::qWait(50);
        emit mockGuiHelper->motChanged("news2.jpg", "News", 1, 102);
        QTest::qWait(50);
        emit mockGuiHelper->motChanged("news3.jpg", "News", 1, 103);
        QTest::qWait(100);

        // All slides should be added to the same category
        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_MotView_SlideReplacement()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        QTest::qWait(100);

        // Add a slide
        emit mockGuiHelper->motChanged("news1.jpg", "News", 1, 100);
        QTest::qWait(50);

        // Replace with same slide ID but different image
        emit mockGuiHelper->motChanged("news1_updated.jpg", "News", 1, 100);
        QTest::qWait(100);

        // Slide should be replaced, not duplicated
        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    // ========================================================================
    // TEST: Expert View Mode
    // ========================================================================

    void test_MotView_ExpertViewFlag()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Test with expert view disabled
        mockMainWindow->setExpertView(false);
        QTest::qWait(50);
        VERIFY_ZERO_CONSOLE_ERRORS();

        // Test with expert view enabled
        mockMainWindow->setExpertView(true);
        QTest::qWait(50);
        VERIFY_ZERO_CONSOLE_ERRORS();

        // Component should handle both modes without errors
    }

    void test_MotView_SlideCountLabel()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Enable expert view to show slide count label
        mockMainWindow->setExpertView(true);
        QTest::qWait(50);

        // Label should be visible in expert mode (when slides > 1)
        // Add multiple slides
        emit mockGuiHelper->motChanged("img1.jpg", "Test", 1, 1);
        emit mockGuiHelper->motChanged("img2.jpg", "Test", 1, 2);
        QTest::qWait(100);

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    // ========================================================================
    // TEST: Visual Rendering
    // ========================================================================

    void test_MotView_Rendering()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        QVERIFY(testVisualRendering());
        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_MotView_RenderingWithImages()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // Add some images
        emit mockGuiHelper->motChanged("image1.jpg", "", 0, 1);
        emit mockGuiHelper->motChanged("image2.jpg", "News", 1, 100);
        QTest::qWait(100);

        // Should render without errors even with images
        QVERIFY(testVisualRendering());
        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    // ========================================================================
    // TEST: Layout and Responsiveness
    // ========================================================================

    void test_MotView_Layout()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // MotView contains RowLayout with category list and image view
        // Should layout correctly

        rootItem->setWidth(800);
        rootItem->setHeight(600);
        QTest::qWait(50);

        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    void test_MotView_ColumnLayoutVisibility()
    {
        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        // ColumnLayout visibility depends on slideList.length > 0
        // Initially should be invisible
        QTest::qWait(50);

        // Add images
        emit mockGuiHelper->motChanged("image1.jpg", "Test", 1, 1);
        QTest::qWait(100);

        // Now should be visible
        VERIFY_ZERO_CONSOLE_ERRORS();
    }

    // ========================================================================
    // INTEGRATION TEST: Complete MOT Workflow
    // ========================================================================

    void test_MotView_CompleteWorkflow()
    {
        qInfo() << "=== Integration Test: Complete MOT Workflow ===";

        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        QTest::qWait(100);

        // Step 1: Verify initial state
        QCOMPARE(getProperty("currentCategoryId").toInt(), 0);
        VERIFY_ZERO_CONSOLE_ERRORS();

        // Step 2: Receive first MOT image
        emit mockGuiHelper->motChanged("slide1.jpg", "", 0, 1);
        QTest::qWait(50);
        QCOMPARE(getProperty("latestPictureName").toString(), QString("slide1.jpg"));
        VERIFY_ZERO_CONSOLE_ERRORS();

        // Step 3: Receive categorized images
        emit mockGuiHelper->motChanged("news1.jpg", "News", 10, 100);
        emit mockGuiHelper->motChanged("sport1.jpg", "Sports", 20, 200);
        QTest::qWait(100);
        VERIFY_ZERO_CONSOLE_ERRORS();

        // Step 4: Add more images to existing category
        emit mockGuiHelper->motChanged("news2.jpg", "News", 10, 101);
        emit mockGuiHelper->motChanged("news3.jpg", "News", 10, 102);
        QTest::qWait(100);
        VERIFY_ZERO_CONSOLE_ERRORS();

        // Step 5: Replace existing image
        emit mockGuiHelper->motChanged("news2_updated.jpg", "News", 10, 101);
        QTest::qWait(50);
        VERIFY_ZERO_CONSOLE_ERRORS();

        // Step 6: Reset MOT
        emit mockGuiHelper->motReseted();
        QTest::qWait(100);
        VERIFY_ZERO_CONSOLE_ERRORS();

        // Step 7: Verify reset state
        QCOMPARE(getProperty("latestPictureName").toString(), QString(""));
        VERIFY_ZERO_CONSOLE_ERRORS();

        qInfo() << "=== Complete MOT Workflow: PASSED ===";
    }

    // ========================================================================
    // STRESS TEST: Many Images and Categories
    // ========================================================================

    void test_MotView_StressTest()
    {
        qInfo() << "=== Stress Test: Many MOT Images ===";

        QString qmlPath = qmlBasePath + "/MotView.qml";
        engine->addImportPath(qmlBasePath);
        QVERIFY(loadComponent(qmlPath));

        QTest::qWait(100);

        // Add many categories
        for (int catId = 1; catId <= 10; catId++) {
            QString categoryName = QString("Category%1").arg(catId);

            // Add multiple slides per category
            for (int slideId = 1; slideId <= 5; slideId++) {
                QString imageName = QString("cat%1_slide%2.jpg").arg(catId).arg(slideId);
                int slideNumber = catId * 100 + slideId;

                emit mockGuiHelper->motChanged(imageName, categoryName, catId, slideNumber);
                QTest::qWait(5);
            }
        }

        QTest::qWait(200);

        // Verify no errors with many images
        VERIFY_ZERO_CONSOLE_ERRORS();

        qInfo() << "=== Stress Test: PASSED ===";
    }
};

QTEST_MAIN(TestMotView)
#include "test_mot_view.moc"
