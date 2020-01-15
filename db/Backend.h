#ifndef DB_BACKEND_H_
#define DB_BACKEND_H_

#include <QObject>
#include <QSettings>
#include <QString>
#include <QVariant>

#include <condition_variable>
#include <deque>
#include <future>
#include <mutex>
#include <thread>
#include <utility>
#include <variant>

class QSqlQuery;

namespace db
{

/**
 * Database backend abstraction
 */
class Backend : public QObject
{
    Q_OBJECT

public:
    struct connData_t
    {
        QString connName;
        QString driverType;
        QString hostname;
        QString dbName;
        QString username;
        QString password;
        bool isEncrypted = false;
        QString connIcon;
        QString schemaName;
        unsigned int port = 0;
    };

    Backend();
    ~Backend();

    typedef std::variant<bool, QVariant> funRet_t;
    std::future<funRet_t> performQuery(std::function<funRet_t()> fun);
    void performQueryNR(std::function<void()> fun);

    static std::vector<struct connData_t> readAllConnectionEntries(QSettings& settings);

signals:
    void dbConnected(QString dbName, QString schemaName);
    void iconUpdated(QString dbName, QString iconPath);

public slots:
    void configureDbConnection(QObject* dialog, bool encReady);
    void reconfigureDbConnection(QObject* dialog, QString err);
    void dbConfigured(QVariant connData);
    bool connectDatabases();
    void connectDb(const struct connData_t& cd);


private:
    static bool shouldReconnect(const struct connData_t& oldConn, const struct connData_t& newConn);
    void writeConnectionEntry(QSettings& settings, const struct connData_t& connData);
    static struct connData_t readConnectionEntry(QSettings& settings);
    void writeAllConnectionEntries(QSettings& settings,
        const std::vector<struct connData_t>& connData);

private:
    std::thread connThread;
    typedef std::tuple<QString, QString, std::promise<QSqlQuery>> strQuery_t; // dbName, sqlString
    typedef std::pair<std::function<funRet_t()>, std::promise<funRet_t>> funQuery_t;
    typedef std::function<void()> funQueryNR_t;
    typedef std::variant<funQueryNR_t, funQuery_t> vquery_t;
    std::deque<vquery_t> queries;
    std::mutex mQueries;
    std::condition_variable cv;
    std::mutex mCv;
    bool terminate = false;
};

}

#endif /* DB_BACKEND_H_ */
