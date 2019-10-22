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


    bool isEncryptionReady();
    QStringList keysList() const;

signals:
    void shouldBeSaved(QVariant url, QVariant login);
    void shouldBeUpdated(QVariant url, QVariant login);

public slots:
    void fillPassword(QVariant view);
    void keySelected(QString id);
    void loadSucceeded(QVariant view);
    void saveAccepted(QString url, bool accepted);
    bool savePassword(QVariant fields);
    QVariant getCredentials(QVariant host, QVariant path) noexcept;

private:
    gnupgpp::GnupgPP gpg;

    std::map<QString, struct db::Passwords2::entry_t> tempStore;
    QString formExtractor;
    QString formFiller;
    QString qWebChannel;

    QString encrypt(QString text);
    std::shared_ptr<db::DbGroup> getDefDbGroup();
};

#endif /* PASSWORDMANAGER_H_ */
