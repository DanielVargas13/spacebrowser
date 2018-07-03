#ifndef DB_PASSWORDS_H_
#define DB_PASSWORDS_H_

#include <QString>

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

    void saveOrUpdate(entry_t pwd);
};

} /* namespace db */

#endif /* DB_PASSWORDS_H_ */
