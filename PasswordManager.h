#ifndef PASSWORDMANAGER_H_
#define PASSWORDMANAGER_H_

#include <db/Passwords.h>
#include <gnupgpp/GnupgPP.h>

#include <QObject>
#include <QString>
#include <QVariant>

#include <map>

class PasswordManager : public QObject
{
    Q_OBJECT
public:
    PasswordManager();
    virtual ~PasswordManager();


    bool isEncryptionReady();

signals:
    void shouldBeSaved(QVariant url, QVariant login);
    void shouldBeUpdated(QVariant url, QVariant login);

public slots:
    void loadSucceeded(QVariant view);
    void saveAccepted(QString url, bool accepted);
    bool savePassword(QVariant fields);

private:
    gnupgpp::GnupgPP gpg;
    Passwords pwds;
    std::map<QString, struct Passwords::entry_t> tempStore;
    QString formExtractor;
    QString qWebChannel;

    QString encrypt(QString text);
};

#endif /* PASSWORDMANAGER_H_ */
