#ifndef TABMODEL_TEST_H_
#define TABMODEL_TEST_H_

#include <QObject>
#include <QtTest/QtTest>

#include <QStandardPaths>

#include <test/TabModel_test_mock.h>

#include <Tab.h>
#include <TabModel.h>

#include <iostream>

class TabModel_test: public QObject
{
    Q_OBJECT

    void fillWithThree(TabModel& tabModel, std::shared_ptr<db::DbGroup> dbg)
    {
        using namespace testing;

        /// Make sure model is empty
        QCOMPARE(tabModel.rowCount(), 0);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 0);
        QCOMPARE(tabModel.views2.size(), 0);

        EXPECT_CALL(dbg->tabs, createTab()).Times(3)
            .WillOnce(Return(1)).WillOnce(Return(2)).WillOnce(Return(3));
        EXPECT_CALL(dbg->tabs, setParent(1, 0));
        EXPECT_CALL(dbg->tabs, setParent(2, 0));
        EXPECT_CALL(dbg->tabs, setParent(3, 0));

        tabModel.createTab(0);
        QCOMPARE(tabModel.rowCount(), 1);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 1);
        QCOMPARE(tabModel.views2.size(), 1);

        tabModel.createTab(0);
        QCOMPARE(tabModel.rowCount(), 2);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 2);
        QCOMPARE(tabModel.views2.size(), 2);

        tabModel.createTab(0);
        QCOMPARE(tabModel.rowCount(), 3);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 3);
        QCOMPARE(tabModel.views2.size(), 3);

        /// Check internals of flatModel
        QCOMPARE(tabModel.getFlatModel()->toSource.size(), 3);
        QCOMPARE(tabModel.getFlatModel()->fromSource.size(), 3);
        QCOMPARE(tabModel.getFlatModel()->viewId2ModelId.size(), 3);

        QCOMPARE(tabModel.getFlatModel()->toSource[0]->data().toInt(), 1);
        QCOMPARE(tabModel.getFlatModel()->toSource[1]->data().toInt(), 2);
        QCOMPARE(tabModel.getFlatModel()->toSource[2]->data().toInt(), 3);

        for (unsigned int i = 0; i < tabModel.getFlatModel()->toSource.size(); ++i)
        {
            auto& item = tabModel.getFlatModel()->toSource[i];
            QCOMPARE(tabModel.getFlatModel()->fromSource[item], i);
        }

        QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[1], 0);
        QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[2], 1);
        QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[3], 2);
    }

    void fillWithThreePlusChildren(TabModel& tabModel, std::shared_ptr<db::DbGroup> dbg, int parentId)
    {
        using namespace testing;

        /// Make sure model is empty
        QCOMPARE(tabModel.rowCount(), 0);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 0);
        QCOMPARE(tabModel.views2.size(), 0);

        EXPECT_CALL(dbg->tabs, createTab()).Times(6)
            .WillOnce(Return(1)).WillOnce(Return(2)).WillOnce(Return(3))
            .WillOnce(Return(4)).WillOnce(Return(5)).WillOnce(Return(6));
        EXPECT_CALL(dbg->tabs, setParent(1, 0));
        EXPECT_CALL(dbg->tabs, setParent(2, 0));
        EXPECT_CALL(dbg->tabs, setParent(3, 0));
        EXPECT_CALL(dbg->tabs, setParent(4, parentId));
        EXPECT_CALL(dbg->tabs, setParent(5, parentId));
        EXPECT_CALL(dbg->tabs, setParent(6, parentId));

        tabModel.createTab(0);
        QCOMPARE(tabModel.rowCount(), 1);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 1);
        QCOMPARE(tabModel.views2.size(), 1);

        tabModel.createTab(0);
        QCOMPARE(tabModel.rowCount(), 2);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 2);
        QCOMPARE(tabModel.views2.size(), 2);

        tabModel.createTab(0);
        QCOMPARE(tabModel.rowCount(), 3);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 3);
        QCOMPARE(tabModel.views2.size(), 3);

        tabModel.createTab(parentId);
        QCOMPARE(tabModel.rowCount(), 3);
        QCOMPARE(tabModel.invisibleRootItem()->child(parentId-1)->rowCount(), 1);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 4);
        QCOMPARE(tabModel.views2.size(), 4);

        tabModel.createTab(parentId);
        QCOMPARE(tabModel.rowCount(), 3);
        QCOMPARE(tabModel.invisibleRootItem()->child(parentId-1)->rowCount(), 2);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 5);
        QCOMPARE(tabModel.views2.size(), 5);

        tabModel.createTab(parentId);
        QCOMPARE(tabModel.rowCount(), 3);
        QCOMPARE(tabModel.invisibleRootItem()->child(parentId-1)->rowCount(), 3);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 6);
        QCOMPARE(tabModel.views2.size(), 6);


        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0))->getId(), 1);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(1))->getId(), 2);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(2))->getId(), 3);

        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(parentId-1)->child(0))->getId(), 4);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(parentId-1)->child(1))->getId(), 5);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(parentId-1)->child(2))->getId(), 6);


        QCOMPARE(tabModel.getFlatModel()->getModelId(1), 0);
        switch(parentId)
        {
            case 1: {
                QCOMPARE(tabModel.getFlatModel()->getModelId(2), 4);
                QCOMPARE(tabModel.getFlatModel()->getModelId(3), 5);
                QCOMPARE(tabModel.getFlatModel()->getModelId(4), 1);
                QCOMPARE(tabModel.getFlatModel()->getModelId(5), 2);
                QCOMPARE(tabModel.getFlatModel()->getModelId(6), 3);
            } break;

            case 2: {
                QCOMPARE(tabModel.getFlatModel()->getModelId(2), 1);
                QCOMPARE(tabModel.getFlatModel()->getModelId(3), 5);
                QCOMPARE(tabModel.getFlatModel()->getModelId(4), 2);
                QCOMPARE(tabModel.getFlatModel()->getModelId(5), 3);
                QCOMPARE(tabModel.getFlatModel()->getModelId(6), 4);
            } break;

            case 3: {
                QCOMPARE(tabModel.getFlatModel()->getModelId(2), 1);
                QCOMPARE(tabModel.getFlatModel()->getModelId(3), 2);
                QCOMPARE(tabModel.getFlatModel()->getModelId(4), 3);
                QCOMPARE(tabModel.getFlatModel()->getModelId(5), 4);
                QCOMPARE(tabModel.getFlatModel()->getModelId(6), 5);
            } break;
        }

        QCOMPARE(tabModel.views2.count(1), 1);
        QCOMPARE(tabModel.views2.count(2), 1);
        QCOMPARE(tabModel.views2.count(3), 1);
        QCOMPARE(tabModel.views2.count(4), 1);
        QCOMPARE(tabModel.views2.count(5), 1);
        QCOMPARE(tabModel.views2.count(6), 1);

        QCOMPARE(tabModel.views2.at(1).tabData->getId(), 1);
        QCOMPARE(tabModel.views2.at(2).tabData->getId(), 2);
        QCOMPARE(tabModel.views2.at(3).tabData->getId(), 3);
        QCOMPARE(tabModel.views2.at(4).tabData->getId(), 4);
        QCOMPARE(tabModel.views2.at(5).tabData->getId(), 5);
        QCOMPARE(tabModel.views2.at(6).tabData->getId(), 6);

        /// Check internals of flatModel
        QCOMPARE(tabModel.getFlatModel()->toSource.size(), 6);
        QCOMPARE(tabModel.getFlatModel()->fromSource.size(), 6);
        QCOMPARE(tabModel.getFlatModel()->viewId2ModelId.size(), 6);

        for (unsigned int i = 0; i < tabModel.getFlatModel()->toSource.size(); ++i)
        {
            auto& item = tabModel.getFlatModel()->toSource[i];
            QCOMPARE(tabModel.getFlatModel()->fromSource[item], i);
        }

        switch(parentId)
        {
            case 1: {
                QCOMPARE(tabModel.getFlatModel()->toSource[0]->data().toInt(), 1);
                QCOMPARE(tabModel.getFlatModel()->toSource[1]->data().toInt(), 4);
                QCOMPARE(tabModel.getFlatModel()->toSource[2]->data().toInt(), 5);
                QCOMPARE(tabModel.getFlatModel()->toSource[3]->data().toInt(), 6);
                QCOMPARE(tabModel.getFlatModel()->toSource[4]->data().toInt(), 2);
                QCOMPARE(tabModel.getFlatModel()->toSource[5]->data().toInt(), 3);

                QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[1], 0);
                QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[2], 4);
                QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[3], 5);
                QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[4], 1);
                QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[5], 2);
                QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[6], 3);
            } break;

            case 2: {
                QCOMPARE(tabModel.getFlatModel()->toSource[0]->data().toInt(), 1);
                QCOMPARE(tabModel.getFlatModel()->toSource[1]->data().toInt(), 2);
                QCOMPARE(tabModel.getFlatModel()->toSource[2]->data().toInt(), 4);
                QCOMPARE(tabModel.getFlatModel()->toSource[3]->data().toInt(), 5);
                QCOMPARE(tabModel.getFlatModel()->toSource[4]->data().toInt(), 6);
                QCOMPARE(tabModel.getFlatModel()->toSource[5]->data().toInt(), 3);

                QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[1], 0);
                QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[2], 1);
                QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[3], 5);
                QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[4], 2);
                QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[5], 3);
                QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[6], 4);
            } break;

            case 3: {
                QCOMPARE(tabModel.getFlatModel()->toSource[0]->data().toInt(), 1);
                QCOMPARE(tabModel.getFlatModel()->toSource[1]->data().toInt(), 2);
                QCOMPARE(tabModel.getFlatModel()->toSource[2]->data().toInt(), 3);
                QCOMPARE(tabModel.getFlatModel()->toSource[3]->data().toInt(), 4);
                QCOMPARE(tabModel.getFlatModel()->toSource[4]->data().toInt(), 5);
                QCOMPARE(tabModel.getFlatModel()->toSource[5]->data().toInt(), 6);

                QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[1], 0);
                QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[2], 1);
                QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[3], 2);
                QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[4], 3);
                QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[5], 4);
                QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[6], 5);
            } break;
        }
    }


    /// |
    /// \_1
    ///  \_4
    ///   \_7
    ///   \_16
    ///   \_17
    ///  \_10
    ///   \_18
    ///   \_19
    ///   \_20
    ///  \_11
    ///   \_21
    ///   \_22
    ///   \_23
    /// \_2
    ///  \_5
    ///   \_8
    ///   \_24
    ///   \_25
    ///  \_12
    ///   \_26
    ///   \_27
    ///   \_28
    ///  \_13
    ///   \_29
    ///   \_30
    ///   \_31
    /// \_3
    ///  \_6
    ///   \_9
    ///  \_14
    ///  \_15
    void fillComplexStructure(TabModel& tabModel, std::shared_ptr<db::DbGroup> dbg)
    {
        using namespace testing;

        fillWithThree(tabModel, dbg);

        int i = 3;
        EXPECT_CALL(dbg->tabs, createTab()).Times(28).WillRepeatedly(Invoke([&i](){ return ++i; }));

        EXPECT_CALL(dbg->tabs, setParent(4, 1));
        EXPECT_CALL(dbg->tabs, setParent(5, 2));
        EXPECT_CALL(dbg->tabs, setParent(6, 3));

        tabModel.createTab(1);
        QCOMPARE(tabModel.invisibleRootItem()->child(0)->rowCount(), 1);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 4);
        QCOMPARE(tabModel.views2.size(), 4);

        tabModel.createTab(2);
        QCOMPARE(tabModel.invisibleRootItem()->child(1)->rowCount(), 1);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 5);
        QCOMPARE(tabModel.views2.size(), 5);

        tabModel.createTab(3);
        QCOMPARE(tabModel.invisibleRootItem()->child(2)->rowCount(), 1);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 6);
        QCOMPARE(tabModel.views2.size(), 6);

        EXPECT_CALL(dbg->tabs, setParent(7, 4));
        EXPECT_CALL(dbg->tabs, setParent(8, 5));
        EXPECT_CALL(dbg->tabs, setParent(9, 6));

        tabModel.createTab(4);
        QCOMPARE(tabModel.invisibleRootItem()->child(0)->child(0)->rowCount(), 1);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 7);
        QCOMPARE(tabModel.views2.size(), 7);

        tabModel.createTab(5);
        QCOMPARE(tabModel.invisibleRootItem()->child(1)->child(0)->rowCount(), 1);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 8);
        QCOMPARE(tabModel.views2.size(), 8);

        tabModel.createTab(6);
        QCOMPARE(tabModel.invisibleRootItem()->child(2)->child(0)->rowCount(), 1);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 9);
        QCOMPARE(tabModel.views2.size(), 9);

        EXPECT_CALL(dbg->tabs, setParent(10, 1));
        EXPECT_CALL(dbg->tabs, setParent(11, 1));

        tabModel.createTab(1);
        QCOMPARE(tabModel.invisibleRootItem()->child(0)->rowCount(), 2);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 10);
        QCOMPARE(tabModel.views2.size(), 10);

        tabModel.createTab(1);
        QCOMPARE(tabModel.invisibleRootItem()->child(0)->rowCount(), 3);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 11);
        QCOMPARE(tabModel.views2.size(), 11);

        EXPECT_CALL(dbg->tabs, setParent(12, 2));
        EXPECT_CALL(dbg->tabs, setParent(13, 2));

        tabModel.createTab(2);
        QCOMPARE(tabModel.invisibleRootItem()->child(1)->rowCount(), 2);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 12);
        QCOMPARE(tabModel.views2.size(), 12);

        tabModel.createTab(2);
        QCOMPARE(tabModel.invisibleRootItem()->child(1)->rowCount(), 3);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 13);
        QCOMPARE(tabModel.views2.size(), 13);

        EXPECT_CALL(dbg->tabs, setParent(14, 3));
        EXPECT_CALL(dbg->tabs, setParent(15, 3));

        tabModel.createTab(3);
        QCOMPARE(tabModel.invisibleRootItem()->child(2)->rowCount(), 2);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 14);
        QCOMPARE(tabModel.views2.size(), 14);

        tabModel.createTab(3);
        QCOMPARE(tabModel.invisibleRootItem()->child(2)->rowCount(), 3);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 15);
        QCOMPARE(tabModel.views2.size(), 15);

        EXPECT_CALL(dbg->tabs, setParent(16, 4));
        EXPECT_CALL(dbg->tabs, setParent(17, 4));

        tabModel.createTab(4);
        QCOMPARE(tabModel.invisibleRootItem()->child(0)->child(0)->rowCount(), 2);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 16);
        QCOMPARE(tabModel.views2.size(), 16);

        tabModel.createTab(4);
        QCOMPARE(tabModel.invisibleRootItem()->child(0)->child(0)->rowCount(), 3);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 17);
        QCOMPARE(tabModel.views2.size(), 17);

        EXPECT_CALL(dbg->tabs, setParent(18, 10));
        EXPECT_CALL(dbg->tabs, setParent(19, 10));
        EXPECT_CALL(dbg->tabs, setParent(20, 10));

        tabModel.createTab(10);
        QCOMPARE(tabModel.invisibleRootItem()->child(0)->child(1)->rowCount(), 1);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 18);
        QCOMPARE(tabModel.views2.size(), 18);

        tabModel.createTab(10);
        QCOMPARE(tabModel.invisibleRootItem()->child(0)->child(1)->rowCount(), 2);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 19);
        QCOMPARE(tabModel.views2.size(), 19);

        tabModel.createTab(10);
        QCOMPARE(tabModel.invisibleRootItem()->child(0)->child(1)->rowCount(), 3);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 20);
        QCOMPARE(tabModel.views2.size(), 20);

        EXPECT_CALL(dbg->tabs, setParent(21, 11));
        EXPECT_CALL(dbg->tabs, setParent(22, 11));
        EXPECT_CALL(dbg->tabs, setParent(23, 11));

        tabModel.createTab(11);
        QCOMPARE(tabModel.invisibleRootItem()->child(0)->child(2)->rowCount(), 1);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 21);
        QCOMPARE(tabModel.views2.size(), 21);

        tabModel.createTab(11);
        QCOMPARE(tabModel.invisibleRootItem()->child(0)->child(2)->rowCount(), 2);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 22);
        QCOMPARE(tabModel.views2.size(), 22);

        tabModel.createTab(11);
        QCOMPARE(tabModel.invisibleRootItem()->child(0)->child(2)->rowCount(), 3);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 23);
        QCOMPARE(tabModel.views2.size(), 23);

        EXPECT_CALL(dbg->tabs, setParent(24, 5));
        EXPECT_CALL(dbg->tabs, setParent(25, 5));

        tabModel.createTab(5);
        QCOMPARE(tabModel.invisibleRootItem()->child(1)->child(0)->rowCount(), 2);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 24);
        QCOMPARE(tabModel.views2.size(), 24);

        tabModel.createTab(5);
        QCOMPARE(tabModel.invisibleRootItem()->child(1)->child(0)->rowCount(), 3);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 25);
        QCOMPARE(tabModel.views2.size(), 25);

        EXPECT_CALL(dbg->tabs, setParent(26, 12));
        EXPECT_CALL(dbg->tabs, setParent(27, 12));
        EXPECT_CALL(dbg->tabs, setParent(28, 12));

        tabModel.createTab(12);
        QCOMPARE(tabModel.invisibleRootItem()->child(1)->child(1)->rowCount(), 1);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 26);
        QCOMPARE(tabModel.views2.size(), 26);

        tabModel.createTab(12);
        QCOMPARE(tabModel.invisibleRootItem()->child(1)->child(1)->rowCount(), 2);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 27);
        QCOMPARE(tabModel.views2.size(), 27);

        tabModel.createTab(12);
        QCOMPARE(tabModel.invisibleRootItem()->child(1)->child(1)->rowCount(), 3);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 28);
        QCOMPARE(tabModel.views2.size(), 28);

        EXPECT_CALL(dbg->tabs, setParent(29, 13));
        EXPECT_CALL(dbg->tabs, setParent(30, 13));
        EXPECT_CALL(dbg->tabs, setParent(31, 13));

        tabModel.createTab(13);
        QCOMPARE(tabModel.invisibleRootItem()->child(1)->child(2)->rowCount(), 1);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 29);
        QCOMPARE(tabModel.views2.size(), 29);

        tabModel.createTab(13);
        QCOMPARE(tabModel.invisibleRootItem()->child(1)->child(2)->rowCount(), 2);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 30);
        QCOMPARE(tabModel.views2.size(), 30);

        tabModel.createTab(13);
        QCOMPARE(tabModel.invisibleRootItem()->child(1)->child(2)->rowCount(), 3);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 31);
        QCOMPARE(tabModel.views2.size(), 31);

        QCOMPARE(tabModel.rowCount(), 3);
    }

    void checkComplexStructure(TabModel& tabModel, int deleted = -1)
    {
        std::map<int, int> flatId2ViewId =
            {{0, 1}, {1, 4}, {2, 7}, {3, 16}, {4, 17}, {5, 10}, {6, 18}, {7, 19}, {8, 20}, {9, 11},
             {10, 21}, {11, 22}, {12, 23}, {13, 2}, {14, 5}, {15, 8}, {16, 24}, {17, 25}, {18, 12}, {19, 26},
             {20, 27}, {21, 28}, {22, 13}, {23, 29}, {24, 30}, {25, 31}, {26, 3}, {27, 6}, {28, 9}, {29, 14},
             {30, 15}};

        /// Check flat model IDs
        int offset = 0;
        for (int i = 0; i < tabModel.getFlatModel()->rowCount(); ++i)
        {
            int viewId = ((Tab*)tabModel.getFlatModel()->toSource.at(i))->getId();

            //std::cout << "id: " << i << " viewId: " << viewId << std::endl;

            if (viewId == deleted)
            {
                //std::cout << "---> offsetting\n";
                offset = 1;
                continue;
            }

            int mViewId = flatId2ViewId.at(i+offset);

            QCOMPARE(viewId, mViewId);
        }

        /// Check internal mappings in flatModel
        for (unsigned int i = 0; i < tabModel.getFlatModel()->toSource.size(); ++i)
        {
            auto& item = tabModel.getFlatModel()->toSource[i];
            QCOMPARE(tabModel.getFlatModel()->fromSource[item], i);
        }


        /// Check tabsModel structure
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0))->getId(), 1);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(1))->getId(), 2);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(2))->getId(), 3);


        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0)->child(0))->getId(), 4);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0)->child(1))->getId(), 10);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0)->child(2))->getId(), 11);

        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(1)->child(0))->getId(), 5);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(1)->child(1))->getId(), 26);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(1)->child(2))->getId(), 27);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(1)->child(3))->getId(), 28);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(1)->child(4))->getId(), 13);

        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(2)->child(0))->getId(), 6);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(2)->child(1))->getId(), 14);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(2)->child(2))->getId(), 15);


        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0)->child(0)->child(0))->getId(), 7);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0)->child(0)->child(1))->getId(), 16);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0)->child(0)->child(2))->getId(), 17);

        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0)->child(1)->child(0))->getId(), 18);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0)->child(1)->child(1))->getId(), 19);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0)->child(1)->child(2))->getId(), 20);

        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0)->child(2)->child(0))->getId(), 21);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0)->child(2)->child(1))->getId(), 22);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0)->child(2)->child(2))->getId(), 23);


        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(1)->child(0)->child(0))->getId(), 8);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(1)->child(0)->child(1))->getId(), 24);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(1)->child(0)->child(2))->getId(), 25);

        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(1)->child(4)->child(0))->getId(), 29);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(1)->child(4)->child(1))->getId(), 30);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(1)->child(4)->child(2))->getId(), 31);

    }

private slots:
    void initTestCase()
    {
        /// Initialize logging
        ///
        qSetMessagePattern("[%{category}] %{function}[:%{line}]: %{message}");

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

        QQuickItem_mock tabSelector;
        QQuickItem_mock tabSelectorPanel;
        QQuickItem_mock webViewContainer;
        std::shared_ptr<db::DbGroup> dbg = std::make_shared<db::DbGroup>();
        TabModel tabModel("test", &tabSelector, &tabSelectorPanel, &webViewContainer, nullptr, dbg);

        /// This is normally called in loadTabs():
        tabModel.getFlatModel()->setSourceModel(&tabModel);

        fillWithThree(tabModel, dbg);

        /// getProperty will be called and must return currently selected tab
        EXPECT_CALL(dbg->config, getProperty(QString("currentTab")))
            .Times(1).WillOnce(Return("2"));
        EXPECT_CALL(dbg->tabs, closeTab(1)).Times(1);

        tabModel.closeTab(1);

        /// Check all models have correct row count
        QCOMPARE(tabModel.rowCount(), 2);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 2);
        QCOMPARE(tabModel.views2.size(), 2);

        /// Check that items have correct ids, and their order was not changed
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0))->getId(), 2);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(1))->getId(), 3);

        EXPECT_THROW(tabModel.getFlatModel()->getModelId(1), std::out_of_range);
        QCOMPARE(tabModel.getFlatModel()->getModelId(2), 0);
        QCOMPARE(tabModel.getFlatModel()->getModelId(3), 1);

        QCOMPARE(tabModel.views2.count(1), 0);
        QCOMPARE(tabModel.views2.count(2), 1);
        QCOMPARE(tabModel.views2.count(3), 1);

        QCOMPARE(tabModel.views2.at(2).tabData->getId(), 2);
        QCOMPARE(tabModel.views2.at(3).tabData->getId(), 3);

        /// Check internals of flatModel
        QCOMPARE(tabModel.getFlatModel()->toSource.size(), 2);
        QCOMPARE(tabModel.getFlatModel()->fromSource.size(), 2);
        QCOMPARE(tabModel.getFlatModel()->viewId2ModelId.size(), 2);

        QCOMPARE(tabModel.getFlatModel()->toSource[0]->data().toInt(), 2);
        QCOMPARE(tabModel.getFlatModel()->toSource[1]->data().toInt(), 3);

        for (unsigned int i = 0; i < tabModel.getFlatModel()->toSource.size(); ++i)
        {
            auto& item = tabModel.getFlatModel()->toSource[i];
            QCOMPARE(tabModel.getFlatModel()->fromSource[item], i);
        }

        QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[2], 0);
        QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[3], 1);

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

        QQuickItem_mock tabSelector;
        QQuickItem_mock tabSelectorPanel;
        QQuickItem_mock webViewContainer;
        std::shared_ptr<db::DbGroup> dbg = std::make_shared<db::DbGroup>();
        TabModel tabModel("test", &tabSelector, &tabSelectorPanel, &webViewContainer, nullptr, dbg);

        /// This is normally called in loadTabs():
        tabModel.getFlatModel()->setSourceModel(&tabModel);

        fillWithThree(tabModel, dbg);

        /// Expected call will set first view as current (after closing second)
        /// This seems impossible to check, setProperty in QObject is not virtual thus can't be mocked
        //QVariant v = dynamic_cast<Tab*>(tabModel.invisibleRootItem()->
        //child(0))->getView();
        //EXPECT_CALL(webViewContainer, setProperty("currentView", v));

        /// And will select first tab as current
        EXPECT_CALL(dbg->config, setProperty(QString("currentTab"), QVariant(1)));

        /// getProperty will be called and must return currently selected tab
        EXPECT_CALL(dbg->config, getProperty(QString("currentTab")))
            .Times(1).WillOnce(Return(QVariant(2)));
        EXPECT_CALL(dbg->tabs, closeTab(2)).Times(1);

        tabModel.closeTab(2);

        /// Check all models have correct row count
        QCOMPARE(tabModel.rowCount(), 2);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 2);
        QCOMPARE(tabModel.views2.size(), 2);

        /// Check that items have correct ids, and their order was not changed
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0))->getId(), 1);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(1))->getId(), 3);

        QCOMPARE(tabModel.getFlatModel()->getModelId(1), 0);
        EXPECT_THROW(tabModel.getFlatModel()->getModelId(2), std::out_of_range);
        QCOMPARE(tabModel.getFlatModel()->getModelId(3), 1);

        QCOMPARE(tabModel.views2.count(1), 1);
        QCOMPARE(tabModel.views2.count(2), 0);
        QCOMPARE(tabModel.views2.count(3), 1);

        QCOMPARE(tabModel.views2.at(1).tabData->getId(), 1);
        QCOMPARE(tabModel.views2.at(3).tabData->getId(), 3);

        /// Check internals of flatModel
        QCOMPARE(tabModel.getFlatModel()->toSource.size(), 2);
        QCOMPARE(tabModel.getFlatModel()->fromSource.size(), 2);
        QCOMPARE(tabModel.getFlatModel()->viewId2ModelId.size(), 2);

        QCOMPARE(tabModel.getFlatModel()->toSource[0]->data().toInt(), 1);
        QCOMPARE(tabModel.getFlatModel()->toSource[1]->data().toInt(), 3);

        for (unsigned int i = 0; i < tabModel.getFlatModel()->toSource.size(); ++i)
        {
            auto& item = tabModel.getFlatModel()->toSource[i];
            QCOMPARE(tabModel.getFlatModel()->fromSource[item], i);
        }

        QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[1], 0);
        QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[3], 1);

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

        QQuickItem_mock tabSelector;
        QQuickItem_mock tabSelectorPanel;
        QQuickItem_mock webViewContainer;
        std::shared_ptr<db::DbGroup> dbg = std::make_shared<db::DbGroup>();
        TabModel tabModel("test", &tabSelector, &tabSelectorPanel, &webViewContainer, nullptr, dbg);

        /// This is normally called in loadTabs():
        tabModel.getFlatModel()->setSourceModel(&tabModel);

        fillWithThree(tabModel, dbg);

        /// getProperty will be called and must return currently selected tab
        EXPECT_CALL(dbg->config, getProperty(QString("currentTab")))
            .Times(1).WillOnce(Return(QVariant(2)));
        EXPECT_CALL(dbg->tabs, closeTab(3)).Times(1);

        tabModel.closeTab(3);

        /// Check all models have correct row count
        QCOMPARE(tabModel.rowCount(), 2);
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 2);
        QCOMPARE(tabModel.views2.size(), 2);

        /// Check that items have correct ids, and their order was not changed
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0))->getId(), 1);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(1))->getId(), 2);

        QCOMPARE(tabModel.getFlatModel()->getModelId(1), 0);
        QCOMPARE(tabModel.getFlatModel()->getModelId(2), 1);
        EXPECT_THROW(tabModel.getFlatModel()->getModelId(3), std::out_of_range);

        QCOMPARE(tabModel.views2.count(1), 1);
        QCOMPARE(tabModel.views2.count(2), 1);
        QCOMPARE(tabModel.views2.count(3), 0);

        QCOMPARE(tabModel.views2.at(1).tabData->getId(), 1);
        QCOMPARE(tabModel.views2.at(2).tabData->getId(), 2);

        /// Check internals of flatModel
        QCOMPARE(tabModel.getFlatModel()->toSource.size(), 2);
        QCOMPARE(tabModel.getFlatModel()->fromSource.size(), 2);
        QCOMPARE(tabModel.getFlatModel()->viewId2ModelId.size(), 2);

        QCOMPARE(tabModel.getFlatModel()->toSource[0]->data().toInt(), 1);
        QCOMPARE(tabModel.getFlatModel()->toSource[1]->data().toInt(), 2);

        for (unsigned int i = 0; i < tabModel.getFlatModel()->toSource.size(); ++i)
        {
            auto& item = tabModel.getFlatModel()->toSource[i];
            QCOMPARE(tabModel.getFlatModel()->fromSource[item], i);
        }

        QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[1], 0);
        QCOMPARE(tabModel.getFlatModel()->viewId2ModelId[2], 1);

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

        QQuickItem_mock tabSelector;
        QQuickItem_mock tabSelectorPanel;
        QQuickItem_mock webViewContainer;
        std::shared_ptr<db::DbGroup> dbg = std::make_shared<db::DbGroup>();
        TabModel tabModel("test", &tabSelector, &tabSelectorPanel, &webViewContainer, nullptr, dbg);

        /// This is normally called in loadTabs():
        tabModel.getFlatModel()->setSourceModel(&tabModel);
        fillWithThreePlusChildren(tabModel, dbg, 1);

        /// getProperty will be called and must return currently selected tab
        EXPECT_CALL(dbg->config, getProperty(QString("currentTab")))
            .Times(1).WillOnce(Return(QVariant(1)));

        /// Expected call will set fourth (new first) view as current
        /// (after closing first)
        /// This seems impossible to check, setProperty in QObject is not virtual thus can't be mocked
        //QVariant v = dynamic_cast<Tab*>(tabModel.invisibleRootItem()->
        //                                child(0)->child(0))->getView();
        //EXPECT_CALL(webViewContainer,
        //            setProperty(QString("currentView"), v));


        /// And will select fourth (new first) tab as current
        EXPECT_CALL(dbg->config,
                    setProperty(QString("currentTab"),
                                QVariant(4)));

        /// Reparenting of children tabs
        EXPECT_CALL(dbg->tabs, setParent(4, 0));
        EXPECT_CALL(dbg->tabs, setParent(5, 0));
        EXPECT_CALL(dbg->tabs, setParent(6, 0));

        EXPECT_CALL(dbg->tabs, closeTab(1)).Times(1);

        tabModel.closeTab(1);

        /// Check all models have correct row count
        QCOMPARE(tabModel.rowCount(), 5);
        for (int i = 0; i < tabModel.rowCount(); ++i)
        {
            QCOMPARE(tabModel.invisibleRootItem()->child(i)->rowCount(), 0);
        }
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 5);
        QCOMPARE(tabModel.getFlatModel()->toSource.size(), 5);
        QCOMPARE(tabModel.getFlatModel()->fromSource.size(), 5);
        QCOMPARE(tabModel.getFlatModel()->viewId2ModelId.size(), 5);
        QCOMPARE(tabModel.views2.size(), 5);

        /// Check all rows are on correct positions
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0))->getId(), 4);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(1))->getId(), 5);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(2))->getId(), 6);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(3))->getId(), 2);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(4))->getId(), 3);

        EXPECT_THROW(tabModel.getFlatModel()->getModelId(1), std::out_of_range);
        QCOMPARE(tabModel.getFlatModel()->getModelId(2), 3);
        QCOMPARE(tabModel.getFlatModel()->getModelId(3), 4);
        QCOMPARE(tabModel.getFlatModel()->getModelId(4), 0);
        QCOMPARE(tabModel.getFlatModel()->getModelId(5), 1);
        QCOMPARE(tabModel.getFlatModel()->getModelId(6), 2);

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

        QQuickItem_mock tabSelector;
        QQuickItem_mock tabSelectorPanel;
        QQuickItem_mock webViewContainer;
        std::shared_ptr<db::DbGroup> dbg = std::make_shared<db::DbGroup>();
        TabModel tabModel("test", &tabSelector, &tabSelectorPanel, &webViewContainer, nullptr, dbg);

        /// This is normally called in loadTabs():
        tabModel.getFlatModel()->setSourceModel(&tabModel);
        fillWithThreePlusChildren(tabModel, dbg, 2);

        /// getProperty will be called and must return currently selected tab
        EXPECT_CALL(dbg->config, getProperty(QString("currentTab")))
            .Times(1).WillOnce(Return(QVariant(1)));

        /// Reparenting of children tabs
        EXPECT_CALL(dbg->tabs, setParent(4, 0));
        EXPECT_CALL(dbg->tabs, setParent(5, 0));
        EXPECT_CALL(dbg->tabs, setParent(6, 0));

        EXPECT_CALL(dbg->tabs, closeTab(2)).Times(1);

        tabModel.closeTab(2);

        /// Check all models have correct row count
        QCOMPARE(tabModel.rowCount(), 5);
        for (int i = 0; i < tabModel.rowCount(); ++i)
        {
            QCOMPARE(tabModel.invisibleRootItem()->child(i)->rowCount(), 0);
        }
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 5);
        QCOMPARE(tabModel.getFlatModel()->toSource.size(), 5);
        QCOMPARE(tabModel.getFlatModel()->fromSource.size(), 5);
        QCOMPARE(tabModel.getFlatModel()->viewId2ModelId.size(), 5);
        QCOMPARE(tabModel.views2.size(), 5);

        /// Check all rows are on correct positions
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0))->getId(), 1);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(1))->getId(), 4);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(2))->getId(), 5);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(3))->getId(), 6);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(4))->getId(), 3);

        QCOMPARE(tabModel.getFlatModel()->getModelId(1), 0);
        EXPECT_THROW(tabModel.getFlatModel()->getModelId(2), std::out_of_range);
        QCOMPARE(tabModel.getFlatModel()->getModelId(3), 4);
        QCOMPARE(tabModel.getFlatModel()->getModelId(4), 1);
        QCOMPARE(tabModel.getFlatModel()->getModelId(5), 2);
        QCOMPARE(tabModel.getFlatModel()->getModelId(6), 3);

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

        QQuickItem_mock tabSelector;
        QQuickItem_mock tabSelectorPanel;
        QQuickItem_mock webViewContainer;
        std::shared_ptr<db::DbGroup> dbg = std::make_shared<db::DbGroup>();
        TabModel tabModel("test", &tabSelector, &tabSelectorPanel, &webViewContainer, nullptr, dbg);

        /// This is normally called in loadTabs():
        tabModel.getFlatModel()->setSourceModel(&tabModel);
        fillWithThreePlusChildren(tabModel, dbg, 3);

        using namespace testing;

        /// getProperty will be called and must return currently selected tab
        EXPECT_CALL(dbg->config, getProperty(QString("currentTab")))
            .Times(1).WillOnce(Return(QVariant(1)));

        /// Reparenting of children tabs
        EXPECT_CALL(dbg->tabs, setParent(4, 0));
        EXPECT_CALL(dbg->tabs, setParent(5, 0));
        EXPECT_CALL(dbg->tabs, setParent(6, 0));

        EXPECT_CALL(dbg->tabs, closeTab(3)).Times(1);

        tabModel.closeTab(3);

        /// Check all models have correct row count
        QCOMPARE(tabModel.rowCount(), 5);
        for (int i = 0; i < tabModel.rowCount(); ++i)
        {
            QCOMPARE(tabModel.invisibleRootItem()->child(i)->rowCount(), 0);
        }
        QCOMPARE(tabModel.getFlatModel()->rowCount(), 5);
        QCOMPARE(tabModel.getFlatModel()->toSource.size(), 5);
        QCOMPARE(tabModel.getFlatModel()->fromSource.size(), 5);
        QCOMPARE(tabModel.getFlatModel()->viewId2ModelId.size(), 5);
        QCOMPARE(tabModel.views2.size(), 5);

        /// Check all rows are on correct positions
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(0))->getId(), 1);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(1))->getId(), 2);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(2))->getId(), 4);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(3))->getId(), 5);
        QCOMPARE(dynamic_cast<Tab*>(tabModel.invisibleRootItem()
                                    ->child(4))->getId(), 6);

        QCOMPARE(tabModel.getFlatModel()->getModelId(1), 0);
        QCOMPARE(tabModel.getFlatModel()->getModelId(2), 1);
        EXPECT_THROW(tabModel.getFlatModel()->getModelId(3), std::out_of_range);
        QCOMPARE(tabModel.getFlatModel()->getModelId(4), 2);
        QCOMPARE(tabModel.getFlatModel()->getModelId(5), 3);
        QCOMPARE(tabModel.getFlatModel()->getModelId(6), 4);
    }

    /// Removing element from complex structure
    ///
    /// |        =>      |
    /// \_1
    ///  \_4
    ///   \_7
    ///   \_16
    ///   \_17
    ///  \_10
    ///   \_18
    ///   \_19
    ///   \_20
    ///  \_11
    ///   \_21
    ///   \_22
    ///   \_23
    /// \_2
    ///  \_5
    ///   \_8
    ///   \_24
    ///   \_25
    ///  \_12            | x
    ///   \_26
    ///   \_27
    ///   \_28
    ///  \_13
    ///   \_29
    ///   \_30
    ///   \_31
    /// \_3
    ///  \_6
    ///   \_9
    ///  \_14
    ///  \_15

    void complexRemoveMiddle()
    {
        using namespace testing;

        QQuickItem_mock tabSelector;
        QQuickItem_mock tabSelectorPanel;
        QQuickItem_mock webViewContainer;
        std::shared_ptr<db::DbGroup> dbg = std::make_shared<db::DbGroup>();
        TabModel tabModel("test", &tabSelector, &tabSelectorPanel, &webViewContainer, nullptr, dbg);

        /// This is normally called in loadTabs():
        tabModel.getFlatModel()->setSourceModel(&tabModel);

        fillComplexStructure(tabModel, dbg);

        /// getProperty will be called and must return currently selected tab
        EXPECT_CALL(dbg->config, getProperty(QString("currentTab")))
            .Times(1).WillOnce(Return(QVariant(1)));

        /// Reparenting of children tabs
        EXPECT_CALL(dbg->tabs, setParent(26, 2));
        EXPECT_CALL(dbg->tabs, setParent(27, 2));
        EXPECT_CALL(dbg->tabs, setParent(28, 2));

        EXPECT_CALL(dbg->tabs, closeTab(12)).Times(1);


        tabModel.closeTab(12);

        /// FIXME: currently need to supply element that is before the deleted one (on flatModel)
        checkComplexStructure(tabModel, 25);
    }

};

#endif
