#ifndef VIEWHANDLER_TEST_H_
#define VIEWHANDLER_TEST_H_

#include <QObject>
#include <QQuickItem>
#include <QtTest/QtTest>

#include <test/ViewHandler_test_mock.h>

#include <ViewHandler.h>

#include <iostream>

class ViewHandler_test: public QObject
{
    Q_OBJECT

private:
    void fillWithThree(ViewHandler& vh)
    {
        using namespace testing;

        /// Make sure model is empty
        QCOMPARE(vh.tabsModel.rowCount(), 0);
        QCOMPARE(vh.flatModel.rowCount(), 0);
        QCOMPARE(vh.views2.size(), 0);

        EXPECT_CALL(vh.tabsDb, createTab()).Times(3)
            .WillOnce(Return(1)).WillOnce(Return(2)).WillOnce(Return(3));
        EXPECT_CALL(vh.tabsDb, setParent(1, 0));
        EXPECT_CALL(vh.tabsDb, setParent(2, 0));
        EXPECT_CALL(vh.tabsDb, setParent(3, 0));

        vh.createTab(0);
        QCOMPARE(vh.tabsModel.rowCount(), 1);
        QCOMPARE(vh.flatModel.rowCount(), 1);
        QCOMPARE(vh.views2.size(), 1);

        vh.createTab(0);
        QCOMPARE(vh.tabsModel.rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 2);
        QCOMPARE(vh.views2.size(), 2);

        vh.createTab(0);
        QCOMPARE(vh.tabsModel.rowCount(), 3);
        QCOMPARE(vh.flatModel.rowCount(), 3);
        QCOMPARE(vh.views2.size(), 3);

        /// Check internals of flatModel
        QCOMPARE(vh.flatModel.toSource.size(), 3);
        QCOMPARE(vh.flatModel.fromSource.size(), 3);
        QCOMPARE(vh.flatModel.viewId2ModelId.size(), 3);

        QCOMPARE(vh.flatModel.toSource[0]->data().toInt(), 1);
        QCOMPARE(vh.flatModel.toSource[1]->data().toInt(), 2);
        QCOMPARE(vh.flatModel.toSource[2]->data().toInt(), 3);

        for (unsigned int i = 0; i < vh.flatModel.toSource.size(); ++i)
        {
            auto& item = vh.flatModel.toSource[i];
            QCOMPARE(vh.flatModel.fromSource[item], i);
        }

        QCOMPARE(vh.flatModel.viewId2ModelId[1], 0);
        QCOMPARE(vh.flatModel.viewId2ModelId[2], 1);
        QCOMPARE(vh.flatModel.viewId2ModelId[3], 2);
    }

    void fillWithThreePlusChildren(ViewHandler& vh, int parentId)
    {
        using namespace testing;

        /// Make sure model is empty
        QCOMPARE(vh.tabsModel.rowCount(), 0);
        QCOMPARE(vh.flatModel.rowCount(), 0);
        QCOMPARE(vh.views2.size(), 0);

        EXPECT_CALL(vh.tabsDb, createTab()).Times(6)
            .WillOnce(Return(1)).WillOnce(Return(2)).WillOnce(Return(3))
            .WillOnce(Return(4)).WillOnce(Return(5)).WillOnce(Return(6));
        EXPECT_CALL(vh.tabsDb, setParent(1, 0));
        EXPECT_CALL(vh.tabsDb, setParent(2, 0));
        EXPECT_CALL(vh.tabsDb, setParent(3, 0));
        EXPECT_CALL(vh.tabsDb, setParent(4, parentId));
        EXPECT_CALL(vh.tabsDb, setParent(5, parentId));
        EXPECT_CALL(vh.tabsDb, setParent(6, parentId));

        vh.createTab(0);
        QCOMPARE(vh.tabsModel.rowCount(), 1);
        QCOMPARE(vh.flatModel.rowCount(), 1);
        QCOMPARE(vh.views2.size(), 1);

        vh.createTab(0);
        QCOMPARE(vh.tabsModel.rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 2);
        QCOMPARE(vh.views2.size(), 2);

        vh.createTab(0);
        QCOMPARE(vh.tabsModel.rowCount(), 3);
        QCOMPARE(vh.flatModel.rowCount(), 3);
        QCOMPARE(vh.views2.size(), 3);

        vh.createTab(parentId);
        QCOMPARE(vh.tabsModel.rowCount(), 3);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(parentId-1)->rowCount(), 1);
        QCOMPARE(vh.flatModel.rowCount(), 4);
        QCOMPARE(vh.views2.size(), 4);

        vh.createTab(parentId);
        QCOMPARE(vh.tabsModel.rowCount(), 3);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(parentId-1)->rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 5);
        QCOMPARE(vh.views2.size(), 5);

        vh.createTab(parentId);
        QCOMPARE(vh.tabsModel.rowCount(), 3);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(parentId-1)->rowCount(), 3);
        QCOMPARE(vh.flatModel.rowCount(), 6);
        QCOMPARE(vh.views2.size(), 6);


        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0))->getId(), 1);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1))->getId(), 2);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(2))->getId(), 3);

        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(parentId-1)->child(0))->getId(), 4);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(parentId-1)->child(1))->getId(), 5);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(parentId-1)->child(2))->getId(), 6);


        QCOMPARE(vh.flatModel.getModelId(1), 0);
        switch(parentId)
        {
            case 1: {
                QCOMPARE(vh.flatModel.getModelId(2), 4);
                QCOMPARE(vh.flatModel.getModelId(3), 5);
                QCOMPARE(vh.flatModel.getModelId(4), 1);
                QCOMPARE(vh.flatModel.getModelId(5), 2);
                QCOMPARE(vh.flatModel.getModelId(6), 3);
            } break;

            case 2: {
                QCOMPARE(vh.flatModel.getModelId(2), 1);
                QCOMPARE(vh.flatModel.getModelId(3), 5);
                QCOMPARE(vh.flatModel.getModelId(4), 2);
                QCOMPARE(vh.flatModel.getModelId(5), 3);
                QCOMPARE(vh.flatModel.getModelId(6), 4);
            } break;

            case 3: {
                QCOMPARE(vh.flatModel.getModelId(2), 1);
                QCOMPARE(vh.flatModel.getModelId(3), 2);
                QCOMPARE(vh.flatModel.getModelId(4), 3);
                QCOMPARE(vh.flatModel.getModelId(5), 4);
                QCOMPARE(vh.flatModel.getModelId(6), 5);
            } break;
        }

        QCOMPARE(vh.views2.count(1), 1);
        QCOMPARE(vh.views2.count(2), 1);
        QCOMPARE(vh.views2.count(3), 1);
        QCOMPARE(vh.views2.count(4), 1);
        QCOMPARE(vh.views2.count(5), 1);
        QCOMPARE(vh.views2.count(6), 1);

        QCOMPARE(vh.views2.at(1).tabData->getId(), 1);
        QCOMPARE(vh.views2.at(2).tabData->getId(), 2);
        QCOMPARE(vh.views2.at(3).tabData->getId(), 3);
        QCOMPARE(vh.views2.at(4).tabData->getId(), 4);
        QCOMPARE(vh.views2.at(5).tabData->getId(), 5);
        QCOMPARE(vh.views2.at(6).tabData->getId(), 6);

        /// Check internals of flatModel
        QCOMPARE(vh.flatModel.toSource.size(), 6);
        QCOMPARE(vh.flatModel.fromSource.size(), 6);
        QCOMPARE(vh.flatModel.viewId2ModelId.size(), 6);

        for (unsigned int i = 0; i < vh.flatModel.toSource.size(); ++i)
        {
            auto& item = vh.flatModel.toSource[i];
            QCOMPARE(vh.flatModel.fromSource[item], i);
        }

        switch(parentId)
        {
            case 1: {
                QCOMPARE(vh.flatModel.toSource[0]->data().toInt(), 1);
                QCOMPARE(vh.flatModel.toSource[1]->data().toInt(), 4);
                QCOMPARE(vh.flatModel.toSource[2]->data().toInt(), 5);
                QCOMPARE(vh.flatModel.toSource[3]->data().toInt(), 6);
                QCOMPARE(vh.flatModel.toSource[4]->data().toInt(), 2);
                QCOMPARE(vh.flatModel.toSource[5]->data().toInt(), 3);

                QCOMPARE(vh.flatModel.viewId2ModelId[1], 0);
                QCOMPARE(vh.flatModel.viewId2ModelId[2], 4);
                QCOMPARE(vh.flatModel.viewId2ModelId[3], 5);
                QCOMPARE(vh.flatModel.viewId2ModelId[4], 1);
                QCOMPARE(vh.flatModel.viewId2ModelId[5], 2);
                QCOMPARE(vh.flatModel.viewId2ModelId[6], 3);
            } break;

            case 2: {
                QCOMPARE(vh.flatModel.toSource[0]->data().toInt(), 1);
                QCOMPARE(vh.flatModel.toSource[1]->data().toInt(), 2);
                QCOMPARE(vh.flatModel.toSource[2]->data().toInt(), 4);
                QCOMPARE(vh.flatModel.toSource[3]->data().toInt(), 5);
                QCOMPARE(vh.flatModel.toSource[4]->data().toInt(), 6);
                QCOMPARE(vh.flatModel.toSource[5]->data().toInt(), 3);

                QCOMPARE(vh.flatModel.viewId2ModelId[1], 0);
                QCOMPARE(vh.flatModel.viewId2ModelId[2], 1);
                QCOMPARE(vh.flatModel.viewId2ModelId[3], 5);
                QCOMPARE(vh.flatModel.viewId2ModelId[4], 2);
                QCOMPARE(vh.flatModel.viewId2ModelId[5], 3);
                QCOMPARE(vh.flatModel.viewId2ModelId[6], 4);
            } break;

            case 3: {
                QCOMPARE(vh.flatModel.toSource[0]->data().toInt(), 1);
                QCOMPARE(vh.flatModel.toSource[1]->data().toInt(), 2);
                QCOMPARE(vh.flatModel.toSource[2]->data().toInt(), 3);
                QCOMPARE(vh.flatModel.toSource[3]->data().toInt(), 4);
                QCOMPARE(vh.flatModel.toSource[4]->data().toInt(), 5);
                QCOMPARE(vh.flatModel.toSource[5]->data().toInt(), 6);

                QCOMPARE(vh.flatModel.viewId2ModelId[1], 0);
                QCOMPARE(vh.flatModel.viewId2ModelId[2], 1);
                QCOMPARE(vh.flatModel.viewId2ModelId[3], 2);
                QCOMPARE(vh.flatModel.viewId2ModelId[4], 3);
                QCOMPARE(vh.flatModel.viewId2ModelId[5], 4);
                QCOMPARE(vh.flatModel.viewId2ModelId[6], 5);
            } break;
        }
    }



private slots:
    void initTestCase()
    {

//        qDebug("Called before everything else.");
    }

    /// Test adding tabs, then removing first
    ///
    /// |        =>      |
    /// \_1              \_2
    /// \_2              \_3
    /// \_3
    ///
    void addThreeRemoveFirst()
    {
        using namespace testing;

        // QVERIFY(true); // check that a condition is satisfied
        ContentFilter cf;
        std::shared_ptr<QQuickView> view(new QQuickView);
        ViewHandler vh(&cf, view);

        /// This is normally called in loadTabs():
        vh.flatModel.setSourceModel(&vh.tabsModel);

        fillWithThree(vh);

        /// getProperty will be called and must return currently selected tab
        EXPECT_CALL(vh.configDb, getProperty(std::string("currentTab")))
            .Times(1).WillOnce(Return(std::string("2")));

        vh.closeTab(1);

        /// Check all models have correct row count
        QCOMPARE(vh.tabsModel.rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 2);
        QCOMPARE(vh.views2.size(), 2);

        /// Check that items have correct ids, and their order was not changed
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0))->getId(), 2);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1))->getId(), 3);

        EXPECT_THROW(vh.flatModel.getModelId(1), std::out_of_range);
        QCOMPARE(vh.flatModel.getModelId(2), 0);
        QCOMPARE(vh.flatModel.getModelId(3), 1);

        QCOMPARE(vh.views2.count(1), 0);
        QCOMPARE(vh.views2.count(2), 1);
        QCOMPARE(vh.views2.count(3), 1);

        QCOMPARE(vh.views2.at(2).tabData->getId(), 2);
        QCOMPARE(vh.views2.at(3).tabData->getId(), 3);

        /// Check internals of flatModel
        QCOMPARE(vh.flatModel.toSource.size(), 2);
        QCOMPARE(vh.flatModel.fromSource.size(), 2);
        QCOMPARE(vh.flatModel.viewId2ModelId.size(), 2);

        QCOMPARE(vh.flatModel.toSource[0]->data().toInt(), 2);
        QCOMPARE(vh.flatModel.toSource[1]->data().toInt(), 3);

        for (unsigned int i = 0; i < vh.flatModel.toSource.size(); ++i)
        {
            auto& item = vh.flatModel.toSource[i];
            QCOMPARE(vh.flatModel.fromSource[item], i);
        }

        QCOMPARE(vh.flatModel.viewId2ModelId[2], 0);
        QCOMPARE(vh.flatModel.viewId2ModelId[3], 1);

    }

    /// Test adding tabs, then removing second
    ///
    /// |        =>      |
    /// \_1              \_1
    /// \_2              \_3
    /// \_3
    ///
    void addThreeRemoveSecond()
    {
        using namespace testing;

        ContentFilter cf;
        std::shared_ptr<QQuickView> view(new QQuickView);
        ViewHandler vh(&cf, view);

        /// This is normally called in loadTabs():
        vh.flatModel.setSourceModel(&vh.tabsModel);

        fillWithThree(vh);

        /// Expected call will set first view as current (after closing second)
        QVariant v = dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()->
                                        child(0))->getView();
        EXPECT_CALL(*vh.webViewContainer,
                    setProperty(std::string("currentView"), v));

        /// And will select first tab as current
        EXPECT_CALL(vh.configDb,
                    setProperty(std::string("currentTab"),
                                std::string("1")));

        /// getProperty will be called and must return currently selected tab
        EXPECT_CALL(vh.configDb, getProperty(std::string("currentTab")))
            .Times(1).WillOnce(Return(std::string("2")));

        vh.closeTab(2);

        /// Check all models have correct row count
        QCOMPARE(vh.tabsModel.rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 2);
        QCOMPARE(vh.views2.size(), 2);

        /// Check that items have correct ids, and their order was not changed
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0))->getId(), 1);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1))->getId(), 3);

        QCOMPARE(vh.flatModel.getModelId(1), 0);
        EXPECT_THROW(vh.flatModel.getModelId(2), std::out_of_range);
        QCOMPARE(vh.flatModel.getModelId(3), 1);

        QCOMPARE(vh.views2.count(1), 1);
        QCOMPARE(vh.views2.count(2), 0);
        QCOMPARE(vh.views2.count(3), 1);

        QCOMPARE(vh.views2.at(1).tabData->getId(), 1);
        QCOMPARE(vh.views2.at(3).tabData->getId(), 3);

        /// Check internals of flatModel
        QCOMPARE(vh.flatModel.toSource.size(), 2);
        QCOMPARE(vh.flatModel.fromSource.size(), 2);
        QCOMPARE(vh.flatModel.viewId2ModelId.size(), 2);

        QCOMPARE(vh.flatModel.toSource[0]->data().toInt(), 1);
        QCOMPARE(vh.flatModel.toSource[1]->data().toInt(), 3);

        for (unsigned int i = 0; i < vh.flatModel.toSource.size(); ++i)
        {
            auto& item = vh.flatModel.toSource[i];
            QCOMPARE(vh.flatModel.fromSource[item], i);
        }

        QCOMPARE(vh.flatModel.viewId2ModelId[1], 0);
        QCOMPARE(vh.flatModel.viewId2ModelId[3], 1);

    }

    /// Test adding tabs, then removing third
    ///
    /// |        =>      |
    /// \_1              \_1
    /// \_2              \_2
    /// \_3
    ///
    void addThreeRemoveThird()
    {
        using namespace testing;

        ContentFilter cf;
        std::shared_ptr<QQuickView> view(new QQuickView);
        ViewHandler vh(&cf, view);

        /// This is normally called in loadTabs():
        vh.flatModel.setSourceModel(&vh.tabsModel);

        fillWithThree(vh);

        /// getProperty will be called and must return currently selected tab
        EXPECT_CALL(vh.configDb, getProperty(std::string("currentTab")))
            .Times(1).WillOnce(Return(std::string("2")));

        vh.closeTab(3);

        /// Check all models have correct row count
        QCOMPARE(vh.tabsModel.rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 2);
        QCOMPARE(vh.views2.size(), 2);

        /// Check that items have correct ids, and their order was not changed
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0))->getId(), 1);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1))->getId(), 2);

        QCOMPARE(vh.flatModel.getModelId(1), 0);
        QCOMPARE(vh.flatModel.getModelId(2), 1);
        EXPECT_THROW(vh.flatModel.getModelId(3), std::out_of_range);

        QCOMPARE(vh.views2.count(1), 1);
        QCOMPARE(vh.views2.count(2), 1);
        QCOMPARE(vh.views2.count(3), 0);

        QCOMPARE(vh.views2.at(1).tabData->getId(), 1);
        QCOMPARE(vh.views2.at(2).tabData->getId(), 2);

        /// Check internals of flatModel
        QCOMPARE(vh.flatModel.toSource.size(), 2);
        QCOMPARE(vh.flatModel.fromSource.size(), 2);
        QCOMPARE(vh.flatModel.viewId2ModelId.size(), 2);

        QCOMPARE(vh.flatModel.toSource[0]->data().toInt(), 1);
        QCOMPARE(vh.flatModel.toSource[1]->data().toInt(), 2);

        for (unsigned int i = 0; i < vh.flatModel.toSource.size(); ++i)
        {
            auto& item = vh.flatModel.toSource[i];
            QCOMPARE(vh.flatModel.fromSource[item], i);
        }

        QCOMPARE(vh.flatModel.viewId2ModelId[1], 0);
        QCOMPARE(vh.flatModel.viewId2ModelId[2], 1);

    }

    /// Test adding 2-level hierarchy of tabs, then removing one parent
    ///
    /// |        =>      |
    /// \_1              \_4
    ///  \_4             \_5
    ///  \_5             \_6
    ///  \_6             \_2
    /// \_2              \_3
    /// \_3
    ///
    void removeParentWithChildrenFirst()
    {
        using namespace testing;

        ContentFilter cf;
        std::shared_ptr<QQuickView> view(new QQuickView);
        ViewHandler vh(&cf, view);

        /// This is normally called in loadTabs():
        vh.flatModel.setSourceModel(&vh.tabsModel);

        fillWithThreePlusChildren(vh, 1);

        /// getProperty will be called and must return currently selected tab
        EXPECT_CALL(vh.configDb, getProperty(std::string("currentTab")))
            .Times(1).WillOnce(Return(std::string("1")));

        /// Expected call will set fourth (new first) view as current
        /// (after closing first)
        QVariant v = dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()->
                                        child(0)->child(0))->getView();
        EXPECT_CALL(*vh.webViewContainer,
                    setProperty(std::string("currentView"), v));


        /// And will select fourth (new first) tab as current
        EXPECT_CALL(vh.configDb,
                    setProperty(std::string("currentTab"),
                                std::string("4")));

        std::cout << "------------------------------------------\n";
        vh.closeTab(1);

        /// Check all models have correct row count
        QCOMPARE(vh.tabsModel.rowCount(), 5);
        for (int i = 0; i < vh.tabsModel.rowCount(); ++i)
        {
            QCOMPARE(vh.tabsModel.invisibleRootItem()->child(i)->rowCount(), 0);
        }
        QCOMPARE(vh.flatModel.rowCount(), 5);
        QCOMPARE(vh.flatModel.toSource.size(), 5);
        QCOMPARE(vh.flatModel.fromSource.size(), 5);
        QCOMPARE(vh.flatModel.viewId2ModelId.size(), 5);
        QCOMPARE(vh.views2.size(), 5);

        /// Check all rows are on correct positions
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0))->getId(), 4);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1))->getId(), 5);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(2))->getId(), 6);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(3))->getId(), 2);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(4))->getId(), 3);

        EXPECT_THROW(vh.flatModel.getModelId(1), std::out_of_range);
        QCOMPARE(vh.flatModel.getModelId(2), 3);
        QCOMPARE(vh.flatModel.getModelId(3), 4);
        QCOMPARE(vh.flatModel.getModelId(4), 0);
        QCOMPARE(vh.flatModel.getModelId(5), 1);
        QCOMPARE(vh.flatModel.getModelId(6), 2);

    }

    /// Test adding 2-level hierarchy of tabs, then removing one parent
    ///
    /// |        =>      |
    /// \_1              \_1
    /// \_2              \_4
    ///  \_4             \_5
    ///  \_5             \_6
    ///  \_6             \_3
    /// \_3
    ///
    void removeParentWithChildrenSecond()
    {
        using namespace testing;

        ContentFilter cf;
        std::shared_ptr<QQuickView> view(new QQuickView);
        ViewHandler vh(&cf, view);

        /// This is normally called in loadTabs():
        vh.flatModel.setSourceModel(&vh.tabsModel);

        fillWithThreePlusChildren(vh, 2);

        /// getProperty will be called and must return currently selected tab
        EXPECT_CALL(vh.configDb, getProperty(std::string("currentTab")))
            .Times(1).WillOnce(Return(std::string("1")));

        std::cout << "------------------------------------------\n";
        vh.closeTab(2);

        /// Check all models have correct row count
        QCOMPARE(vh.tabsModel.rowCount(), 5);
        for (int i = 0; i < vh.tabsModel.rowCount(); ++i)
        {
            QCOMPARE(vh.tabsModel.invisibleRootItem()->child(i)->rowCount(), 0);
        }
        QCOMPARE(vh.flatModel.rowCount(), 5);
        QCOMPARE(vh.flatModel.toSource.size(), 5);
        QCOMPARE(vh.flatModel.fromSource.size(), 5);
        QCOMPARE(vh.flatModel.viewId2ModelId.size(), 5);
        QCOMPARE(vh.views2.size(), 5);

        /// Check all rows are on correct positions
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0))->getId(), 1);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1))->getId(), 4);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(2))->getId(), 5);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(3))->getId(), 6);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(4))->getId(), 3);

        QCOMPARE(vh.flatModel.getModelId(1), 0);
        EXPECT_THROW(vh.flatModel.getModelId(2), std::out_of_range);
        QCOMPARE(vh.flatModel.getModelId(3), 4);
        QCOMPARE(vh.flatModel.getModelId(4), 1);
        QCOMPARE(vh.flatModel.getModelId(5), 2);
        QCOMPARE(vh.flatModel.getModelId(6), 3);

    }

    /// Test adding 2-level hierarchy of tabs, then removing one parent
    ///
    /// |        =>      |
    /// \_1              \_1
    /// \_2              \_2
    /// \_3              \_4
    ///  \_4             \_5
    ///  \_5             \_6
    ///  \_6
    ///
    void removeParentWithChildrenThird()
    {
        using namespace testing;

        ContentFilter cf;
        std::shared_ptr<QQuickView> view(new QQuickView);
        ViewHandler vh(&cf, view);

        /// This is normally called in loadTabs():
        vh.flatModel.setSourceModel(&vh.tabsModel);

        fillWithThreePlusChildren(vh, 3);

    }


    void cleanupTestCase()
    {
//        qDebug("Called after myFirstTest and mySecondTest.");
    }
};


#endif /* VIEWHANDLER_TEST_H_ */
