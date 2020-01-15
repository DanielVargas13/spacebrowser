#ifndef PASSWORDMANAGER_H_
#define PASSWORDMANAGER_H_

#include <gnupgpp/GnupgPP.h>

#include <db/Passwords2.h>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <map>

namespace db
{
struct DbGroup;
}

class PasswordManager : public QObject
{
    Q_OBJECT
public:
    PasswordManager();
    virtual ~PasswordManager();


    bool isEncryptionReady(QString dbName);
    QStringList keysList() const;

signals:
    void encryptionReady(QVariant dbName, QVariant ready);
    void shouldBeSaved(QVariant dbName, QVariant url, QVariant login);
    void shouldBeUpdated(QVariant dbName, QVariant url, QVariant login);

public slots:
    void checkIfEncryptionReady(QString dbName);
    void fillPassword(QVariant view);
    void keySelected(QString id, QString dbName);
    void loadSucceeded(QVariant view);
    void saveAccepted(QString dbName, QString url, bool accepted);
    bool savePassword(QString dbName, QVariant fields);
    QVariant getCredentials(QString dbName, QVariant host, QVariant path) noexcept;

private:
    gnupgpp::GnupgPP gpg;

    std::map<QString, struct db::Passwords2::entry_t> tempStore;
    QString formExtractor;
    QString formFiller;
    QString qWebChannel;

    std::map<QString, std::shared_ptr<QObject>> dbNameCache;

    QString encrypt(QString dbName, QString text);
};

#endif /* PASSWORDMANAGER_H_ */
