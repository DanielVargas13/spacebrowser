#include <PasswordManager.h>

#include <QFile>
#include <QMetaObject>
#include <QString>

#include <iostream>

PasswordManager::PasswordManager()
{
    QFile file(":/js/formExtractor.js");
    file.open(QIODevice::ReadOnly);
    formExtractor = file.readAll();

    QFile file2(":/qtwebchannel/qwebchannel.js");
    file2.open(QIODevice::ReadOnly);
    qWebChannel= file2.readAll();
}

PasswordManager::~PasswordManager()
{

}

void PasswordManager::loadSucceeded(QVariant view)
{
    bool s = QMetaObject::invokeMethod(qvariant_cast<QObject *>(view),
            "runJavaScript", Q_ARG(QString, formExtractor));

    s = QMetaObject::invokeMethod(qvariant_cast<QObject *>(view),
            "runJavaScript", Q_ARG(QString, qWebChannel));
}

bool PasswordManager::savePassword(QVariant fields)
{
    QJsonArray f = fields.toJsonArray();



    std::cout << "PasswordManager::savePassword called\n";
    return true;
}

