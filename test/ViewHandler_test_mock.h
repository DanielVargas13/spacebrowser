#ifndef VIEWHANDLER_TEST_MOCK_H_
#define VIEWHANDLER_TEST_MOCK_H_

#include <QQuickItem>
#include <QVariant>

//#include <db/Config.h>
#include <db/Tabs.h>
#include <db/ScriptBlock.h>

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
    MOCK_METHOD2(setProperty, void(const std::string& name, const QVariant& value));

public slots:
    Q_INVOKABLE QVariant createViewObject(QVariant obj)
    {
        return QVariant::fromValue<QObject*>(new QObject());
    }

};

namespace db
{
class Tabs_mock
{
public:

    MOCK_METHOD0(createTab, int32_t());
    MOCK_METHOD1(closeTab, void(int viewId));
    MOCK_METHOD0(getAllTabs, std::vector<Tabs::TabInfo>());
    MOCK_METHOD0(getAllTabsMap, std::map<int, Tabs::TabInfo>());
    MOCK_METHOD2(setParent, void(int viewId, int parentId));
    MOCK_METHOD2(setUrl, void(int viewId, std::string url));
    MOCK_METHOD2(setTitle, void(int viewId, std::string title));
    MOCK_METHOD2(setIcon, void(int viewId, std::string icon));
};

class Config_mock
{
public:
    MOCK_METHOD2(setProperty, void(const std::string& name, const std::string& value));
    MOCK_METHOD1(getProperty, std::string(const std::string& name));
};

class ScriptBlock_mock
{
public:
    MOCK_METHOD3(isAllowed, db::ScriptBlock::State(const std::string& site,
            const std::string& url, bool earlyReturn));
};

}

#endif /* VIEWHANDLER_TEST_MOCK_H_ */
