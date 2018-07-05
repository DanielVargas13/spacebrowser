#include <db/Passwords.h>

namespace db
{

Passwords::Passwords()
{

}

Passwords::~Passwords()
{

}

Passwords::SaveState Passwords::isSaved(entry_t pwd)
{
    return SaveState::Current; // do nothing until implemented
}

void Passwords::saveOrUpdate(entry_t pwd)
{

}

} /* namespace db */
