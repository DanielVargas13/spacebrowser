#ifndef VIEWHANDLER_TEST_H_
#define VIEWHANDLER_TEST_H_

#include <QObject>
#include <QQuickItem>
#include <QtTest/QtTest>

#include <test/ViewHandler_test_mock.h>

#include <ViewHandler.h>



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
            .WillOnce(Return(0)).WillOnce(Return(1)).WillOnce(Return(2));
        EXPECT_CALL(vh.tabsDb, setParent(0, 0));
        EXPECT_CALL(vh.tabsDb, setParent(1, 0));
        EXPECT_CALL(vh.tabsDb, setParent(2, 0));

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
    }


private slots:
    void initTestCase()
    {

//        qDebug("Called before everything else.");
    }

    /// Test adding tabs, then removing them
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

        /// Expected call will set second view as current (after closing first)
        auto v = dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()->
                                        child(1))->getView();
        EXPECT_CALL(*vh.webViewContainer, setProperty(std::string("currentView"),
                                                      v));
        /// And will select second tab (no 1) as current
        EXPECT_CALL(vh.configDb, setProperty(std::string("currentTab"),
                                             std::string("1")));

        vh.closeTab(0);
        QCOMPARE(vh.tabsModel.rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 2);
        QCOMPARE(vh.views2.size(), 2);

        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0))->getId(), 1);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1))->getId(), 2);


//        QCOMPARE(dynamic_cast<Tab*>(vh.flatModel.index(0, 0))
//                                    ->getId(), 1);
//        QCOMPARE(dynamic_cast<Tab*>(vh.flatModel.invisibleRootItem()
//                                    ->child(1))->getId(), 2);



    }


    void cleanupTestCase()
    {
//        qDebug("Called after myFirstTest and mySecondTest.");
    }
};


#endif /* VIEWHANDLER_TEST_H_ */
