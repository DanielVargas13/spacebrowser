#ifndef DB_PASSWORDS2_H_
#define DB_PASSWORDS2_H_

#include <db/Backend.h>
#include <db/DbClient.h>

namespace db
{

class Passwords2
{
public:
    struct entry_t
    {
        QString host;
        QString path;
        QString login;
        QString password;
        QString fingerprint;
    };

    enum class SaveState
    {
        Current,
        Outdated,
        Absent
    };

    Passwords2(DbClient& _dbClient, Backend& _backend);
    virtual ~Passwords2();

    SaveState isSaved(entry_t pwd);
    bool hasSavedCredentials(QString host, QString path);
    int countSavedCredentials(QString host, QString path);
    std::vector<entry_t> getCredentials(QString host, QString path);

    bool saveOrUpdate(entry_t pwd);

private:
    static QString tableName;
    DbClient& dbClient;
    Backend& backend;
};

}

Q_DECLARE_METATYPE(db::Passwords2::entry_t)

#endif
