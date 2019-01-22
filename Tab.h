#ifndef TAB_H_
#define TAB_H_

#include <db/Tabs.h>

#include <QByteArray>
#include <QHash>
#include <QLoggingCategory>
#include <QObject>
#include <QStandardItem>
#include <QString>
#include <QVariant>

#include <vector>

class Tab : public QStandardItem
{
public:
    Tab(int id);
    Tab(const db::Tabs::TabInfo& ti);

    bool operator== (const Tab &c1)
    {
        /// It is enough to compare id, it's uniqueness is guaranteed
        /// by the database
        ///

        return this->id == c1.id;
    }

    QVariant data(int column) const override;
    void updateIndent();

    int getId() const;
    QString getTitle() const;
    QString getIcon() const;
    QString getUrl() const;
    QVariant getView() const;
//    int getRowId() const;

//    void setId(int id_);
    void setTitle(QString title_);
    void setIcon(QString icon_);
    void setView(QVariant view_);
//    void setRowId(int);

signals:
    void titleChanged();

public:
    static QHash<int, QByteArray> roles;

private:
    int columnCnt = 1;
    int id = -1;

    int indent = 0;
//    std::vector<Tab> children;

    QString url;
    QString title = "Empty";
    QString icon;

    QVariant view;
};

#endif /* TAB_H_ */
