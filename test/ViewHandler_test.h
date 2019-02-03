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


private slots:
    void initTestCase()
    {
//        qDebug("Called before everything else.");
    }

    void myFirstTest()
    {
//        QVERIFY(true); // check that a condition is satisfied
//        QCOMPARE(1, 1); // compare two values
        ContentFilter cf;
        std::shared_ptr<QQuickView> view(new QQuickView);
        ViewHandler vh(&cf, view);
    }


    void cleanupTestCase()
    {
//        qDebug("Called after myFirstTest and mySecondTest.");
    }
};


#endif /* VIEWHANDLER_TEST_H_ */
