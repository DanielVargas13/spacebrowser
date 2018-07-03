#ifndef PASSWORDMANAGER_H_
#define PASSWORDMANAGER_H_

#include <db/Keys.h>
#include <db/Passwords.h>
#include <gnupgpp/GnupgPP.h>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <map>

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
    void keySelected(QString id);
    void loadSucceeded(QVariant view);
    void saveAccepted(QString url, bool accepted);
    bool savePassword(QVariant fields);

private:
    gnupgpp::GnupgPP gpg;
    db::Passwords pwds;
    db::Keys keys;
    std::map<QString, struct db::Passwords::entry_t> tempStore;
    QString formExtractor;
    QString qWebChannel;

    QString encrypt(QString text);
};

#endif /* PASSWORDMANAGER_H_ */
