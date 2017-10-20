#ifndef MISC_DEBUGHELPERS_H_
#define MISC_DEBUGHELPERS_H_

#include <vector>

class QMetaObject;

namespace misc
{

class DebugHelpers
{
public:

    static void printIntVector(const std::vector<int>& v);
    static void inspectProperties(const QMetaObject* meta);
    static void inspectMethods(const QMetaObject* meta);
};

} /* namespace misc */

#endif /* MISC_DEBUGHELPERS_H_ */
