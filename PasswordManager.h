#ifndef PASSWORDMANAGER_H_
#define PASSWORDMANAGER_H_

#include <QObject>
#include <QString>
#include <QVariant>

class PasswordManager : public QObject
{
    Q_OBJECT
public:
    PasswordManager();
    virtual ~PasswordManager();

public slots:
    void loadSucceeded(QVariant view);
    bool savePassword(QVariant fields);

public:
    QString formExtractor;
    QString qWebChannel;
};

#endif /* PASSWORDMANAGER_H_ */
