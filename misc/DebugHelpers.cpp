#include <misc/DebugHelpers.h>

#include <QMetaObject>
#include <QMetaProperty>
#include <QString>

#include <iostream>

namespace misc
{

void DebugHelpers::printIntVector(const std::vector<int>& v)
{
    for (int i: v)
    {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}

void DebugHelpers::inspectProperties(const QMetaObject* meta)
{
    for (int i = meta->propertyOffset(); i < meta->propertyCount(); ++i)
    {
        std::cout << meta->property(i).name() << std::endl;
    }
}

void DebugHelpers::inspectMethods(const QMetaObject* meta)
{
    for (int i = meta->methodOffset(); i < meta->methodCount(); ++i)
    {
        std::cout << QString::fromLatin1(meta->method(i).methodSignature()).toStdString() <<
                std::endl;
    }
}

} /* namespace misc */
