#ifndef DB_PASSWORDS_H_
#define DB_PASSWORDS_H_

#include <QString>

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

#endif /* DB_PASSWORDS_H_ */
