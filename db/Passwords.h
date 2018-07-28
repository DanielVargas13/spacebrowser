#ifndef DB_PASSWORDS_H_
#define DB_PASSWORDS_H_

#include <pqxx/pqxx>

#include <QString>

#include <string>
#include <vector>

namespace db
{

class Passwords
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

    Passwords();
    virtual ~Passwords();

    SaveState isSaved(entry_t pwd);
    bool hasSavedCredentials(QString host, QString path);
    int countSavedCredentials(QString host, QString path);
    std::vector<entry_t> getCredentials(QString host, QString path);

    void saveOrUpdate(entry_t pwd);

private:
    static std::string tableName;
    pqxx::connection conn;
};

} /* namespace db */

#endif /* DB_PASSWORDS_H_ */
