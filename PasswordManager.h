#ifndef PASSWORDMANAGER_H_
#define PASSWORDMANAGER_H_

#include <db/DbGroup.h>
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

    // FIXME: this is temporary, remove:
    void setGrp(db::DbGroup* grp)
    {
        dbh = grp;
    }


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
    db::DbGroup* dbh;

    std::map<QString, struct db::Passwords2::entry_t> tempStore;
    QString formExtractor;
    QString formFiller;
    QString qWebChannel;

    QString encrypt(QString text);
};

#endif /* PASSWORDMANAGER_H_ */
