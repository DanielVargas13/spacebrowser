#ifndef TABMODEL_TEST_MOCK_H_
#define TABMODEL_TEST_MOCK_H_

#include <QQuickItem>
#include <QString>
#include <QVariant>

#include <db/Tabs2.h>
#include <db/ScriptBlock2.h>

#include <gmock/gmock.h>


class ContentFilter
{
public:
    MOCK_METHOD1(getUrlsFor, std::set<std::string>(const std::string& url));
};


class QQuickItem_mock : public QQuickItem
{
    Q_OBJECT

public:
    // Mock of setProperty will not work as it is not virtual in QObject
    //MOCK_METHOD2(setProperty, bool(const char* name, const QVariant& value));

signals:
    void updateTitle(int, QString);
    void updateIcon(int, QString);
    void updateUrl(int, QString);

public slots:
    Q_INVOKABLE QVariant createViewObject(QVariant obj)
    {
        return QVariant::fromValue<QObject*>(new QQuickItem_mock());
    }

    // These definitions are necessary to suppress
    // "No such method" warnings from QMetaObject::invokeMethod
    Q_INVOKABLE QVariant destroyView(QVariant viewId) { return QVariant(); };
    Q_INVOKABLE void scrollToCurrent() { };
    Q_INVOKABLE void selectView(QVariant viewId) { };


};


namespace db
{

class Tabs_mock
{
public:

    MOCK_METHOD0(createTab, int32_t());
    MOCK_METHOD1(closeTab, void(int viewId));
    MOCK_METHOD0(getAllTabs, std::vector<Tabs2::TabInfo>());
    MOCK_METHOD0(getAllTabsMap, std::map<int, Tabs2::TabInfo>());
    MOCK_METHOD2(setParent, void(int viewId, int parentId));
    MOCK_METHOD2(setUrl, void(int viewId, QString url));
    MOCK_METHOD2(setTitle, void(int viewId, QString title));
    MOCK_METHOD2(setIcon, void(int viewId, QString icon));
};

class Config_mock
{
public:
    MOCK_METHOD2(setProperty, bool(QString name, QVariant value));
    MOCK_METHOD1(getProperty, QVariant(QString name));
};

class ScriptBlock_mock
{
public:
    MOCK_METHOD3(isAllowed, db::ScriptBlock2::State(const std::string& site,
            const std::string& url, bool earlyReturn));
};

struct DbGroup
{
    Config_mock config;
    ScriptBlock_mock scb;
    Tabs_mock tabs;


private:

};


}

#endif /* TABMODEL_TEST_MOCK_H_ */
