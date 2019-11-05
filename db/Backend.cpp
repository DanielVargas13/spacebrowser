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

std::future<Backend::funRet_t> Backend::performQuery(std::function<funRet_t()> fun)
{
    std::lock_guard<std::mutex> lock(mQueries);
    std::promise<funRet_t> p;
    std::future<funRet_t> f = p.get_future();

    queries.push_back(funQuery_t(fun, std::move(p)));

    cv.notify_all();

    return f;
}

void Backend::performQueryNR(std::function<void()> fun)
{
    std::lock_guard<std::mutex> lock(mQueries);

    queries.push_back(funQueryNR_t(fun));
    cv.notify_all();

    return;
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
        QVariant schemaName = settings.value(conf::Databases::array::schemaName);
        QVariant username = settings.value(conf::Databases::array::username);
        QVariant isEncrypted = settings.value(conf::Databases::array::isEncrypted);
        QVariant connIcon = settings.value(conf::Databases::array::connIcon);
        QVariant port = settings.value(conf::Databases::array::port);

        QJsonObject obj;
        obj.insert(conf::Databases::array::connName, connName.toString());
        obj.insert(conf::Databases::array::driverType, driverType.toString());
        obj.insert(conf::Databases::array::hostname, hostname.toString());
        obj.insert(conf::Databases::array::dbName, dbName.toString());
        obj.insert(conf::Databases::array::schemaName, schemaName.toString());
        obj.insert(conf::Databases::array::username, username.toString());
        obj.insert(conf::Databases::array::password, "");
        obj.insert(conf::Databases::array::isEncrypted, isEncrypted.toBool());
        obj.insert(conf::Databases::array::connIcon, connIcon.toString());
        obj.insert(conf::Databases::array::port, port.toInt());

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

    struct connData_t newEntry;
    newEntry.connName = cd.value(conf::Databases::array::connName).toString();
    newEntry.driverType = cd.value(conf::Databases::array::driverType).toString();
    newEntry.hostname = cd.value(conf::Databases::array::hostname).toString();
    newEntry.dbName = cd.value(conf::Databases::array::dbName).toString();
    newEntry.schemaName = cd.value(conf::Databases::array::schemaName).toString();
    newEntry.username = cd.value(conf::Databases::array::username).toString();
    newEntry.isEncrypted = cd.value(conf::Databases::array::isEncrypted).toBool();
    newEntry.password = cd.value(conf::Databases::array::password).toString();
    newEntry.connIcon = cd.value(conf::Databases::array::connIcon).toString();
    newEntry.port = cd.value(conf::Databases::array::port).toUInt();

    if (newEntry.isEncrypted) {
        // FIXME: encrypt
        // refactor PasswordManager, extract encryption to
        // EncryptionManager or sth.
        qCDebug(dbLogs) << "Encryption requested";
    }

    QSettings settings;
    auto connections = readAllConnectionEntries(settings);
    struct connData_t oldEntry = newEntry;
    bool shouldConnect = true;

    connections.erase(std::remove_if(connections.begin(), connections.end(),
        [&newEntry, &oldEntry, &shouldConnect](const struct connData_t& e)
            {
                if (e.connName == newEntry.connName)
                {
                    qCDebug(dbLogs, "Found entry to be replaced");
                    oldEntry = e;
                    shouldConnect = Backend::shouldReconnect(oldEntry, newEntry);
                    return true;
                }

                return false;
            }), connections.end());

    connections.push_back(newEntry);
    writeAllConnectionEntries(settings, connections);

    if (shouldConnect)
    {
        // FIXME: if connection already exists it should be disconnected and removed first
        performQueryNR(
            [newEntry, this]()->void
            {
                connectDb(newEntry);
            });
    }
    else
    {
        if (oldEntry.connIcon != newEntry.connIcon)
        {
            qCDebug(dbLogs, "emitting iconUpdated");
            emit iconUpdated(newEntry.connName, newEntry.connIcon);
        }
    }

}

bool Backend::connectDatabases()
{
    qCDebug(dbLogs, "Backend::connectDatabases()");
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
                cd.hostname.isEmpty() || cd.dbName.isEmpty() ||
                cd.schemaName.isEmpty())
                continue;

            connectDb(cd);
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

                if (std::holds_alternative<funQuery_t>(vq))
                {
                    funQuery_t& q = std::get<funQuery_t>(vq);
                    std::function<funRet_t()>& fun = q.first;
                    std::promise<funRet_t>& prom = q.second;

                    prom.set_value(fun());
                }
                else if (std::holds_alternative<funQueryNR_t>(vq))
                {
                    funQueryNR_t& q = std::get<funQueryNR_t>(vq);
                    q();
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

void Backend::connectDb(const struct connData_t& cd)
{
    QSqlDatabase db = QSqlDatabase::addDatabase(cd.driverType, cd.connName);

    db.setHostName(cd.hostname);
    db.setDatabaseName(cd.dbName);
    if (!cd.username.isEmpty())
        db.setUserName(cd.username);

    if (!cd.password.isEmpty())
    {
        if (cd.isEncrypted)
        {
            QString decrypted;
            // FIXME: decrypt

            db.setPassword(decrypted);
        }
        else
            db.setPassword(cd.password);
    }

    if (cd.port != 0)
        db.setPort(cd.port);

    qCDebug(dbLogs) << "Connecting to: "
                    << cd.connName.toStdString().c_str();
    if (!db.open())
        qCCritical(dbLogs) << db.lastError().text().toStdString().c_str();
    else
    {
        emit dbConnected(cd.connName, cd.schemaName);
    }
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
    settings.setValue(conf::Databases::array::schemaName, connData.schemaName);
    settings.setValue(conf::Databases::array::port, connData.port);
}

struct Backend::connData_t Backend::readConnectionEntry(QSettings& settings)
{
    struct connData_t result;

    QVariant connName = settings.value(conf::Databases::array::connName);
    QVariant driverType = settings.value(conf::Databases::array::driverType);
    QVariant hostname = settings.value(conf::Databases::array::hostname);
    QVariant dbName = settings.value(conf::Databases::array::dbName);
    QVariant schemaName = settings.value(conf::Databases::array::schemaName);
    QVariant username = settings.value(conf::Databases::array::username);
    QVariant password = settings.value(conf::Databases::array::password);
    QVariant isEncrypted = settings.value(conf::Databases::array::isEncrypted);
    QVariant connIcon = settings.value(conf::Databases::array::connIcon);
    QVariant port = settings.value(conf::Databases::array::port);

    if (connName.isValid())
        result.connName = connName.toString();
    if (driverType.isValid())
        result.driverType = driverType.toString();
    if (hostname.isValid())
        result.hostname = hostname.toString();
    if (dbName.isValid())
        result.dbName = dbName.toString();
    if (schemaName.isValid())
        result.schemaName = schemaName.toString();
    if (username.isValid())
        result.username = username.toString();
    if (password.isValid())
        result.password = password.toString();
    if (isEncrypted.isValid())
        result.isEncrypted = isEncrypted.toBool();
    if (connIcon.isValid())
        result.connIcon = connIcon.toString();
    if (port.isValid())
        result.port = port.toInt();

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

bool Backend::shouldReconnect(const struct connData_t& oldConn, const struct connData_t& newConn)
{
    if ((oldConn.dbName != newConn.dbName) ||
        (oldConn.driverType != newConn.driverType) ||
        (oldConn.hostname != newConn.hostname) ||
        (oldConn.schemaName != newConn.schemaName) ||
        (oldConn.username != newConn.username))
        return true;

    return false;
}

}
