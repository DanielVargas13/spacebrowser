#ifndef DB_BACKEND_H_
#define DB_BACKEND_H_

#include <QObject>
#include <QSettings>
#include <QVariant>

namespace db
{

/**
 * Database backend abstraction
 */
class Backend : public QObject
{
    Q_OBJECT

    struct connData_t
    {
        QString connName;
        QString driverType;
        QString hostname;
        QString dbName;
        QString username;
        QString password;
        bool isEncrypted = false;
    };

public:
    Backend();
    ~Backend();

public slots:
    void configureDbConnection(QObject* dialog, bool encReady);
    void reconfigureDbConnection(QObject* dialog, QString err);
    void dbConfigured(QVariant connData);
    bool connectDatabases();


private:
    void writeConnectionEntry(QSettings& settings, const struct connData_t& connData);
    struct connData_t readConnetionEntry(QSettings& settings);
    void writeAllConnectionEntries(QSettings& settings,
        const std::vector<struct connData_t>& connData);
    std::vector<struct connData_t> readAllConnectionEntries(QSettings& settings);
};


}

#endif /* DB_BACKEND_H_ */
