#include <PasswordManager.h>

#include <conf/conf.h>
#include <db/DbGroup.h>

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QMetaObject>
#include <QString>
#include <QUrl>
#include <QVariantMap>

Q_LOGGING_CATEGORY(passManLog, "passManLog")

PasswordManager::PasswordManager()
{
    QFile file(":/js/formExtractor.js");
    file.open(QIODevice::ReadOnly);
    formExtractor = file.readAll();

    QFile file2(":/qtwebchannel/qwebchannel.js");
    file2.open(QIODevice::ReadOnly);
    qWebChannel= file2.readAll();

    QFile file3(":/js/formFiller.js");
    file3.open(QIODevice::ReadOnly);
    formFiller = file3.readAll();

    qCDebug(passManLog, "PasswordManager initialized");
}

PasswordManager::~PasswordManager()
{
    qCDebug(passManLog, "PasswordManager destroyed");
}

void PasswordManager::fillPassword(QVariant view)
{
    qCDebug(passManLog, "fillPassword");
    QObject* v = qvariant_cast<QObject *>(view);

    bool ok = QMetaObject::invokeMethod(v, "runJavaScript",
                                        Q_ARG(QString, formFiller));

    if (!ok)
    {
        qCWarning(passManLog, "Failed to run formFiller script");
    }
}

void PasswordManager::fillPassword(QVariant dbName, QVariant view)
{
    qCDebug(passManLog, "fillPassword(dbName=%s)", dbName.toString().toStdString().c_str());

    QObject* v = qvariant_cast<QObject *>(view);

    QObject* wc = qvariant_cast<QObject *>(v->property("webChannel"));

    std::shared_ptr<QObject> name(new QObject());
    name->setObjectName(dbName.toString());

    if (dbNameCache.count(dbName.toString()) == 0)
        dbNameCache[dbName.toString()] = name;

    bool ok = QMetaObject::invokeMethod(wc, "registerObject",
                                        Q_ARG(QString, "dbName_other"),
                                        Q_ARG(QObject*, dbNameCache.at(dbName.toString()).get()));

    if (!ok)
    {
        qCWarning(passManLog, "Failed to provide correct dbName to formFiller script");
    }

    fillPassword(view);
}

void PasswordManager::keySelected(QString id, QString dbName)
{
    qCDebug(passManLog, "keySelected(id=%s, dbName=%s)",
            id.toStdString().c_str(), dbName.toStdString().c_str());
    QStringList parts = id.split(' ');

    if (parts.size() < 1)
    {
        qCWarning(passManLog, "keySelected(id='%s'): invalid id format",
                id.toStdString().c_str());
        return;
    }

    auto dbg = db::DbGroup::getGroup(dbName);

    if (dbg)
    {
        dbg->keys.addKey(parts[0].toStdString(), true);
    }
}

void PasswordManager::loadSucceeded(QVariant view)
{
    qCDebug(passManLog, "loadSucceeded");
    QObject* v = qvariant_cast<QObject *>(view);

    QObject* wc = qvariant_cast<QObject *>(v->property("webChannel"));

    bool ok = QMetaObject::invokeMethod(wc, "registerObject",
                                        Q_ARG(QString, "pwManager"), Q_ARG(QObject*, this));
    if (!ok)
    {
        qCCritical(passManLog, "Failed to register pwManager");
    }

    QVariant dbv = v->property("dbName");
    QString dbName = dbv.toString();
    std::shared_ptr<QObject> name(new QObject());
    name->setObjectName(dbName);

    if (dbNameCache.count(dbName) == 0)
        dbNameCache[dbName] = name;

    ok = QMetaObject::invokeMethod(wc, "registerObject",
                                  Q_ARG(QString, "dbName"),
                                  Q_ARG(QObject*, dbNameCache.at(dbName).get()));
    if (!ok)
    {
        qCCritical(passManLog, "Failed to register dbName");
    }

    ok = QMetaObject::invokeMethod(v, "runJavaScript", Q_ARG(QString, formExtractor));
    if (!ok)
    {
        qCCritical(passManLog, "Failed to run formExtractor");
    }

    QUrl url = v->property("url").toUrl();
    QString host = url.host();
    QString path = url.path();

    auto groups = db::DbGroup::getGroupMap();

    int passCount = 0;
    QStringList dbNames;
    for (auto& grp: groups)
    {
        if (grp.second)
        {
            if (int count = grp.second->pwds.countSavedCredentials(host, path))
            {
                passCount += count;
                dbNames += grp.first;
            }
        }
    }

    qCDebug(passManLog, "%i passwords available", passCount);

    ok = QMetaObject::invokeMethod(v, "passAvailable",
                                   Q_ARG(QVariant, QVariant(passCount)),
                                   Q_ARG(QVariant, QVariant(dbNames)));
    if (!ok)
    {
        qCCritical(passManLog, "Failed to propagate available passwords count");
    }

// FIXME: is this necessary? If so, comment what for
    ok = QMetaObject::invokeMethod(v, "runJavaScript", Q_ARG(QString, qWebChannel));
    if (!ok)
    {
        qCCritical(passManLog, "Failed to run qWebChannel");
    }

    qCDebug(passManLog, "loadSucceeded finished");
}

void PasswordManager::saveAccepted(QString dbName, QString url, bool accepted)
{
    qCDebug(passManLog, "saveAccepted(dbName=%s, url=%s, accepted=%i)",
            dbName.toStdString().c_str(), url.toStdString().c_str(), accepted);

    if (!accepted)
    {
        if (tempStore.count(url))
            tempStore.erase(url);
        return;
    }

    if (tempStore.count(url) == 0)
    {
        qCCritical(passManLog(), "Password not found in tempStore");
        return;
    }

    const auto& entry = tempStore.at(url);

    auto dbg = db::DbGroup::getGroup(dbName);

    if (dbg)
    {
        switch (dbg->pwds.isSaved(entry))
        {
            case db::Passwords2::SaveState::Outdated:
            case db::Passwords2::SaveState::Absent:
            {
                dbg->pwds.saveOrUpdate(entry);
            } break;
            default:
            {
                // nothing to be done
            }
        }
    }
}

bool PasswordManager::savePassword(QString dbName, QVariant fields_qv)
{
    qCDebug(passManLog, "savePassword(dbName=%s)", dbName.toStdString().c_str());

    std::shared_ptr<db::DbGroup> dbg = db::DbGroup::getGroup(dbName);

    if (!dbg)
        return false;

    QJsonArray f = fields_qv.toJsonArray();

    db::Passwords2::entry_t fields;

    for (const auto& field: f)
    {
        if (!field.isObject())
            continue;

        const auto& obj = field.toObject();
        if (obj["type"] == "text" || obj["type"] == "email")
            fields.login = obj["value"].toString();
        else if (obj["type"] == "password" && !obj["value"].toString().isEmpty())
        {
            fields.password = encrypt(dbName, obj["value"].toString());
            fields.fingerprint = dbg->keys.getDefaultKey();
        }
        else if (obj["type"] == "host")
            fields.host = obj["value"].toString();
        else if (obj["type"] == "path")
            fields.path = obj["value"].toString();
    }

    if (fields.login.isEmpty() || fields.password.isEmpty() || fields.host.isEmpty())
    {
        qCCritical(passManLog, "Failed to save password");
        qCDebug(passManLog, "savePassword(): one of the fields was empty");
        qCDebug(passManLog, "login: %s", fields.login.toStdString().c_str());
        qCDebug(passManLog, "pass_len: %s", fields.password.length() ? "non-zero" : "zero");
        qCDebug(passManLog, "url: %s%s",fields.host.toStdString().c_str(),
                fields.path.toStdString().c_str());

        return false;
    }

    tempStore[fields.host + fields.path] = fields;

    switch (dbg->pwds.isSaved(fields))
    {
    case db::Passwords2::SaveState::Outdated:
    {
        qCDebug(passManLog, "emit shouldBeUpdated");
        emit shouldBeUpdated(dbName, fields.host + fields.path, fields.login);
    } break;
    case db::Passwords2::SaveState::Absent:
    {
        qCDebug(passManLog, "emit shouldBeSaved");
        emit shouldBeSaved(dbName, fields.host + fields.path, fields.login);
    } break;
    default:
    {
        // nothing to be done
    }
    }

    return true;
}

QString PasswordManager::encrypt(QString dbName, QString text)
{
    qCDebug(passManLog, "encrypt(dbName=%s)", dbName.toStdString().c_str());
    auto dbg = db::DbGroup::getGroup(dbName);
    if (!dbg)
        return QString();

    try
    {
        gnupgpp::GpgContext ctx = gpg.createContext();
        ctx.setArmor(true);

        QString keyFp = dbg->keys.getDefaultKey();
        if (keyFp.isEmpty())
            return QString();

        std::shared_ptr<gnupgpp::GpgKey> key = ctx.getKey(keyFp.toStdString());
        if (!key)
        {
            qCWarning(passManLog, "Failed to fetch GPG key");
            return QString();
        }

        return QString(ctx.encrypt(text.toStdString(), key).c_str());

    }
    catch (std::exception& e)
    {
        qCCritical(passManLog, "caught exception: %s", e.what());
        qCCritical(passManLog, "If error above is 'unusable public key', check trust level "
                   "set for the selected key in GPG");
    }

    return QString();
}

bool PasswordManager::isEncryptionReady(QString dbName)
{
    qCDebug(passManLog, "isEncryptionReady(dbName=%s)", dbName.toStdString().c_str());
    std::shared_ptr<db::DbGroup> dbg;
    dbg = db::DbGroup::getGroup(dbName);

    if (!dbg)
        return false;

    QString keyFp = dbg->keys.getDefaultKey();
    if (keyFp.isEmpty())
        return false;

    gnupgpp::GpgContext ctx = gpg.createContext();
    std::shared_ptr<gnupgpp::GpgKey> key = ctx.getKey(keyFp.toStdString());
    if (!key)
        return false;

    std::shared_ptr<gnupgpp::GpgKey> sKey = ctx.getKey(keyFp.toStdString(), true);
    if (!sKey)
        return false;

    qCDebug(passManLog, "isEncryptionReady(dbName=%s): ready", dbName.toStdString().c_str());
    return true;
}

void PasswordManager::checkIfEncryptionReady(QString dbName)
{
    qCDebug(passManLog, "checkIfEncryptionReady(dbName=%s)", dbName.toStdString().c_str());
    if (isEncryptionReady(dbName))
        emit encryptionReady(dbName, true);
    else
        emit encryptionReady(dbName, false);
}

QStringList PasswordManager::keysList() const
{
    qCDebug(passManLog, "keysList");
    QStringList result;

    auto ctx = gpg.createContext();

    auto keys = ctx.listSecretKeys();

    for (auto& key: keys)
    {
        if (key.isRevoked()  || key.isExpired() ||
            key.isDisabled() || key.isInvalid())
            continue;

        QString entry;
        entry += QString(key.getFingerprintOfPK().c_str());

        for (auto& uid: key.getUids())
            entry += " " + QString(uid.getUid().c_str());


        for (auto& sk: key.getSubkeys())
        {
            if (sk.isRevoked()  || sk.isExpired() ||
                sk.isDisabled() || sk.isInvalid())
                continue;

            auto cno = sk.getCardNumber();
            if (!cno.empty())
            {
                entry += " Card No:" + QString(cno.c_str());
                break;
            }
        }
        result.push_back(entry);
    }

    return result;
}

QVariant PasswordManager::getCredentials(QString dbName, QVariant host, QVariant path) noexcept
{
    qCDebug(passManLog, "getCredentials(dbName=%s, host=%s, path=%s)",
            dbName.toStdString().c_str(), host.toString().toStdString().c_str(),
            path.toString().toStdString().c_str());

    if (!host.isValid())
        return QVariant();

    QString h = host.toString();
    QString p;

    path.isValid() ? p = path.toString() : p = "";

    auto dbg = db::DbGroup::getGroup(dbName);
    if (!dbg)
        return QJsonObject();

    auto creds = dbg->pwds.getCredentials(h, p);

    switch (creds.size())
    {
    case 0:
    {
        qCDebug(passManLog, "getCredentials(host=%s, path=%s): no credentials found",
                h.toStdString().c_str(), p.toStdString().c_str());
        return QJsonObject();
    }
    case 1:
    {
        auto ctx = gpg.createContext();
        try
        {
            QString pass = ctx.decrypt(creds[0].password.toStdString()).c_str();
            QJsonObject c { {"login", creds[0].login}, {"pass", pass} };
            return c;

        }
        catch (std::exception& e)
        {
            qCCritical(passManLog, "getCredentials() failed: %s", e.what());
        }
    } break;
    default:
    {
        // FIXME: display choice menu
    }
    }

    return QJsonObject();
}
