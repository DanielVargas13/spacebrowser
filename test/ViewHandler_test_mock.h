#ifndef VIEWHANDLER_TEST_MOCK_H_
#define VIEWHANDLER_TEST_MOCK_H_

#include <QQuickItem>

#include <db/Config.h>
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
};

namespace db
{
class Tabs_mock : public Tabs
{
public:

    MOCK_METHOD0(createTab, int32_t());
    MOCK_METHOD1(closeTab, void(int viewId));
    MOCK_METHOD0(getAllTabs, std::vector<TabInfo>());
};

class Config_mock : public Config
{
public:
    MOCK_METHOD2(setProperty, void(const std::string& name, const std::string& value));
};

class ScriptBlock_mock : public ScriptBlock
{
};

}

#endif /* VIEWHANDLER_TEST_MOCK_H_ */
