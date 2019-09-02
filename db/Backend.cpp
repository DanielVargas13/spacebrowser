#include <db/Backend.h>

#include <conf/conf.h>
#include <db/DbClient.h>
#include <db/DbGroup.h>

#include <QJSEngine>
#include <QJSValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include <QVariant>
#include <QVariantMap>

#include <algorithm>
#include <chrono>

namespace db
{

Backend::Backend()
{

}

Backend::~Backend()
{
    terminate = true;
    connThread.join();
}

std::future<QSqlQuery> Backend::performQuery(QString dbName, QString queryStr)
{
    std::lock_guard<std::mutex> lock(mQueries);
    std::promise<QSqlQuery> p;
    std::future<QSqlQuery> f = p.get_future();

    queries.push_back(strQuery_t(dbName, queryStr, std::move(p)));

    cv.notify_all();

    return f;
}

std::future<Backend::funRet_t> Backend::performQuery(std::function<funRet_t()> fun)
{
    std::lock_guard<std::mutex> lock(mQueries);
    std::promise<funRet_t> p;
    std::future<funRet_t> f = p.get_future();

    queries.push_back(funQuery_t(fun, std::move(p)));

    cv.notify_all();

    return f;
}

void Backend::configureDbConnection(QObject* dialog, bool encReady)
{
    QJsonArray dbData;

    QSettings settings;
    unsigned int dbCount = settings.beginReadArray(conf::Databases::dbArray);
    for (unsigned int i = 0; i < dbCount; ++i)
    {
        settings.setArrayIndex(i);

        QVariant connName = settings.value(conf::Databases::array::connName);
        QVariant driverType = settings.value(conf::Databases::array::driverType);
        QVariant hostname = settings.value(conf::Databases::array::hostname);
        QVariant dbName = settings.value(conf::Databases::array::dbName);
        QVariant username = settings.value(conf::Databases::array::username);
        QVariant isEncrypted = settings.value(conf::Databases::array::isEncrypted);

        QJsonObject obj;
        obj.insert(conf::Databases::array::connName, connName.toString());
        obj.insert(conf::Databases::array::driverType, driverType.toString());
        obj.insert(conf::Databases::array::hostname, hostname.toString());
        obj.insert(conf::Databases::array::dbName, dbName.toString());
        obj.insert(conf::Databases::array::username, username.toString());
        obj.insert(conf::Databases::array::password, "");
        obj.insert(conf::Databases::array::isEncrypted, isEncrypted.toBool());

        dbData.append(obj);
    }
    settings.endArray();

    QMetaObject::invokeMethod(dialog, "configureDbConnection",
        Qt::ConnectionType::QueuedConnection,
        Q_ARG(QVariant, QVariant(dbData)),
        Q_ARG(QVariant, QVariant(QSqlDatabase::drivers())),
        Q_ARG(QVariant, encReady));
}

void Backend::reconfigureDbConnection(QObject* dialog, QString err)
{
    QMetaObject::invokeMethod(dialog, "reconfigureDbConnection",
        Q_ARG(QVariant, err));
}

void Backend::dbConfigured(QVariant connData)
{
    if (connData.isNull())
        return;

    QVariantMap cd = qvariant_cast<QVariantMap>(
        qvariant_cast<QJSValue>(connData).toVariant());

    QString connName = cd.value(conf::Databases::array::connName).toString();
    QString driverType = cd.value(conf::Databases::array::driverType).toString();
    QSqlDatabase db = QSqlDatabase::addDatabase(driverType, connName);

    QString hostname = cd.value(conf::Databases::array::hostname).toString();
    QString dbName = cd.value(conf::Databases::array::dbName).toString();
    QString username = cd.value(conf::Databases::array::username).toString();
    QString password = cd.value(conf::Databases::array::password).toString();
    bool isEncrypted = cd.value(conf::Databases::array::isEncrypted).toBool();
    db.setHostName(hostname);
    db.setDatabaseName(dbName);
    db.setUserName(username);
    db.setPassword(password);

    if (!db.open())
    {
        reconfigureDbConnection(sender(), db.lastError().text());
        return;
    }

    struct connData_t newEntry;
    newEntry.connName = connName;
    newEntry.driverType = driverType;
    newEntry.hostname = hostname;
    newEntry.dbName = dbName;
    newEntry.username = username;

    if (isEncrypted) {
        // FIXME: encrypt
        // refactor PasswordManager, extract encryption to
        // EncryptionManager or sth.
        qCDebug(dbLogs) << "Encryption requested";
    }
    else
        newEntry.password = password;

    QSettings settings;
    auto connections = readAllConnectionEntries(settings);

    connections.erase(std::remove_if(connections.begin(), connections.end(),
            [&connName](const struct connData_t& e)
            {
                if (e.connName == connName)
                    return true;

                return false;
            }), connections.end());

    connections.push_back(newEntry);
    writeAllConnectionEntries(settings, connections);
}

bool Backend::connectDatabases()
{
    if (connThread.joinable())
        return true;

    connThread = std::thread([this]()
    {
        QSettings settings;
        unsigned int dbCount = settings.beginReadArray(conf::Databases::dbArray);

        for (unsigned int i = 0; i < dbCount; ++i)
        {
            settings.setArrayIndex(i);

            struct connData_t cd = readConnectionEntry(settings);

            if (cd.connName.isEmpty() || cd.driverType.isEmpty() ||
            cd.hostname.isEmpty() || cd.dbName.isEmpty())
                continue;

            QSqlDatabase db = QSqlDatabase::addDatabase(cd.driverType, cd.connName);

            db.setHostName(cd.hostname);
            db.setDatabaseName(cd.dbName);
            if (!cd.username.isEmpty())
                db.setUserName(cd.username);

            if (cd.isEncrypted)
            {
                QString decrypted;
                // FIXME: decrypt

                db.setPassword(decrypted);
            }
            else
                db.setPassword(cd.password);

            qCDebug(dbLogs) << "Connecting to: "
                            << cd.connName.toStdString().c_str();
            if (!db.open())
                qCCritical(dbLogs) << db.lastError().text().toStdString().c_str();
            else
            {
                qCDebug(dbLogs, "Connected to backend: %s", cd.connName.toStdString().c_str());
                emit dbConnected(cd.connName);
                qCDebug(dbLogs, "Signal emitted");
            }
        }
        settings.endArray();


        while (!terminate)
        {
            /// Process prepared QSqlQuery
            while (!queries.empty())
            {
                std::lock_guard<std::mutex> lock(mQueries);

                if (queries.empty())
                    break;

                vquery_t& vq = queries.front();

                if (std::holds_alternative<strQuery_t>(vq))
                {
                    strQuery_t& q = std::get<strQuery_t>(vq);
                    QString& dbName = std::get<0>(q);
                    QString& queryString = std::get<1>(q);
                    std::promise<QSqlQuery>& prom = std::get<2>(q);

                    QSqlQuery query(QSqlDatabase::database(dbName));

                    query.exec(queryString);
                    prom.set_value(std::move(query));
                }
                else if (std::holds_alternative<funQuery_t>(vq))
                {
                    funQuery_t& q = std::get<funQuery_t>(vq);
                    std::function<funRet_t()>& fun = q.first;
                    std::promise<funRet_t>& prom = q.second;

                    prom.set_value(fun());
                }
                

                queries.pop_front();
            }

            /// Wait for new queries
            {
                using namespace std::chrono_literals;
                std::unique_lock<std::mutex> lock(mCv);
                cv.wait_for(lock, 500ms);
            }
        }
    });

    return true;
}

void Backend::writeConnectionEntry(QSettings& settings,
    const struct connData_t& connData)
{
    settings.setValue(conf::Databases::array::connName, connData.connName);
    settings.setValue(conf::Databases::array::driverType, connData.driverType);
    settings.setValue(conf::Databases::array::hostname, connData.hostname);
    settings.setValue(conf::Databases::array::dbName, connData.dbName);
    settings.setValue(conf::Databases::array::username, connData.username);
    settings.setValue(conf::Databases::array::password, connData.password);
    settings.setValue(conf::Databases::array::isEncrypted, connData.isEncrypted);
    settings.setValue(conf::Databases::array::connIcon, connData.connIcon);
}

struct Backend::connData_t Backend::readConnectionEntry(QSettings& settings)
{
    struct connData_t result;

    QVariant connName = settings.value(conf::Databases::array::connName);
    QVariant driverType = settings.value(conf::Databases::array::driverType);
    QVariant hostname = settings.value(conf::Databases::array::hostname);
    QVariant dbName = settings.value(conf::Databases::array::dbName);
    QVariant username = settings.value(conf::Databases::array::username);
    QVariant password = settings.value(conf::Databases::array::password);
    QVariant isEncrypted = settings.value(conf::Databases::array::isEncrypted);
    QVariant connIcon = settings.value(conf::Databases::array::connIcon);

    if (connName.isValid())
        result.connName = connName.toString();
    if (driverType.isValid())
        result.driverType = driverType.toString();
    if (hostname.isValid())
        result.hostname = hostname.toString();
    if (dbName.isValid())
        result.dbName = dbName.toString();
    if (username.isValid())
        result.username = username.toString();
    if (password.isValid())
        result.password = password.toString();
    if (isEncrypted.isValid())
        result.isEncrypted = isEncrypted.toBool();
    if (connIcon.isValid())
        result.connIcon = connIcon.toString();

    return result;
}

std::vector<struct Backend::connData_t> Backend::readAllConnectionEntries(
    QSettings& settings)
{
    unsigned int dbCount = settings.beginReadArray(conf::Databases::dbArray);
    std::vector<struct connData_t> result;

    for (unsigned int i = 0; i < dbCount; ++i)
    {
        settings.setArrayIndex(i);

        result.push_back(readConnectionEntry(settings));
    }

    settings.endArray();

    return result;
}

void Backend::writeAllConnectionEntries(QSettings& settings,
    const std::vector<struct connData_t>& connData)
{
    settings.beginWriteArray(conf::Databases::dbArray);

    unsigned int i = 0;
    for (const auto& cd: connData)
    {
        settings.setArrayIndex(i++);
        writeConnectionEntry(settings, cd);
    }

    settings.endArray();
}

}
