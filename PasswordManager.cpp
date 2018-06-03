#include <PasswordManager.h>

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QMetaObject>
#include <QString>

#include <iostream>

PasswordManager::PasswordManager()
{
    QFile file(":/js/formExtractor.js");
    file.open(QIODevice::ReadOnly);
    formExtractor = file.readAll();

    QFile file2(":/qtwebchannel/qwebchannel.js");
    file2.open(QIODevice::ReadOnly);
    qWebChannel= file2.readAll();
}

PasswordManager::~PasswordManager()
{

}

void PasswordManager::loadSucceeded(QVariant view)
{
    bool s = QMetaObject::invokeMethod(qvariant_cast<QObject *>(view),
            "runJavaScript", Q_ARG(QString, formExtractor));

    s = QMetaObject::invokeMethod(qvariant_cast<QObject *>(view),
            "runJavaScript", Q_ARG(QString, qWebChannel));

    auto ctx = gpg.createContext();
    auto keys = ctx.listSecretKeys();
    std::cout << "No. of keys: " << keys.size() << std::endl;

    for (auto& key: keys)
    {
        std::cout << "FPK: " << key.getFingerprintOfPK() << std::endl;
        for (auto& uid: key.getUids())
        {
            std::cout << "UID: " << uid.getUid() << std::endl;
            std::cout << "NAME: " << uid.getName() << std::endl;
            std::cout << "EMAIL: " << uid.getEmail() << std::endl;
            std::cout << "CMT: " << uid.getComment() << std::endl;
            std::cout << "ADDR: " << uid.getAddress() << std::endl;
        }
    }
}

void PasswordManager::saveAccepted(QString url, bool accepted)
{
    std::cout << "SAVE ACCEPTED WITH: " << accepted << std::endl;

    if (!accepted)
    {
        if (tempStore.count(url))
            tempStore.erase(url);
        return;
    }

    if (tempStore.count(url) == 0)
    {
        std::cout << "ERROR: password not found in tempStore\n";
        return;
    }

    const auto& entry = tempStore.at(url);

    switch (pwds.isSaved(entry))
    {
    case Passwords::SaveState::Outdated:
    case Passwords::SaveState::Absent:
    {
        pwds.saveOrUpdate(entry);
    } break;
    default:
    {
        // nothing to be done
    }
    }
}

bool PasswordManager::savePassword(QVariant fields_qv)
{
    QJsonArray f = fields_qv.toJsonArray();

    Passwords::entry_t fields;

    for (const auto& field: f)
    {
        if (!field.isObject())
            continue;

        const auto& obj = field.toObject();
        if (obj["type"] == "text" || obj["type"] == "email")
            fields.login = obj["value"].toString();
        else if (obj["type"] == "password" && !obj["value"].toString().isEmpty())
            fields.password = encrypt(obj["value"].toString());
        else if (obj["type"] == "host")
            fields.host = obj["value"].toString();
        else if (obj["type"] == "path")
            fields.path = obj["value"].toString();
    }

    if (fields.login.isEmpty() || fields.password.isEmpty() || fields.host.isEmpty())
    {
        std::cout << "PasswordManager::savePassword(): one of the fields was empty\n";
        std::cout << "PasswordManager::savePassword(): login: " << fields.login.toStdString() << std::endl;
        std::cout << "PasswordManager::savePassword(): pass_len: " << fields.password.length() << std::endl;
        std::cout << "PasswordManager::savePassword(): url: " << fields.host.toStdString()
                << "/" << fields.path.toStdString() << std::endl;

        return false;
    }

    tempStore[fields.host + "/" +fields.path] = fields;

    switch (pwds.isSaved(fields))
    {
    case Passwords::SaveState::Outdated:
    {
        emit shouldBeUpdated(fields.host + "/" +fields.path, fields.login);
    } break;
    case Passwords::SaveState::Absent:
    {
        emit shouldBeSaved(fields.host + "/" +fields.path, fields.login);
    } break;
    default:
    {
        // nothing to be done
    }
    }

    return true;
}

QString PasswordManager::encrypt(QString text)
{
    return text;
    //FIXME: add public key encryption
}

bool PasswordManager::isEncryptionReady()
{
    return false;
}
