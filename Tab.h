#ifndef TAB_H_
#define TAB_H_

//#include <TreeNode.h>
#include <db/Tabs.h>

#include <QStandardItem>
#include <QString>
#include <QLoggingCategory>
#include <QVariant>

#include <vector>

Q_LOGGING_CATEGORY(tabModel, "tabModel")

class Tab : public QStandardItem //public TreeNode
{
public:
//    Q_PROPERTY(QString title MEMBER title NOTIFY titleChanged);

    /// It is enough to compare id, it's uniqueness is guaranteed
    /// by the database
    ///

    Tab(const db::Tabs::TabInfo& ti)
    {
        // FIXME: move to Tabs2::TabInfo
        id = ti.id;
        url = QString(ti.url.c_str());
        title = QString(ti.title.c_str());
        icon = QString(ti.icon.c_str());

    }

    bool operator== (const Tab &c1)
    {
        return this->id == c1.id;
    }

    QVariant data(int column) const override
    {
//        qCDebug(tabModel, ">>>>>> data(column=%i):", column);
//        qCDebug(tabModel, ">>>>>> icon: %s", icon.toStdString().c_str());

        switch (column)
        {
            case 0: return url;
            case 1: return title;
            case 2: return icon;
            case 3: return id;
            case 4: return indent;
        }
        return id;
    }

/*    TreeNode* child(int row) override
    {
        if (row >= children.size())
            return nullptr;

        return children[row];
    }

    TreeNode* parentItem() override
    {
        return parent;
    }
*/
    /*
    int row() const override
    {
        parent->indexOf(this);

        return 0;
        }*/
/*
    int indexOf(const TreeNode* t) const override
    {
        for (unsigned int i = 0; i < children.size(); ++i)
        {
            if (children[i] == *t)
                return i;
        }

        return 0; // FIXME: possibly override operator= and compare only id
    }

    int childCount() const override
    {
        return children.size();
    }
*/
/*
    int columnCount() const override
    {
        return columnCnt;
    }
*/

signals:
    void titleChanged();

//private:
    int columnCnt = 1;

    int id = 1;
    int indent = 0;
//    Tab* parent; // FIXME: shared and weak pointers
    std::vector<Tab> children;

    QString url;
    QString title = "Empty";
    QString icon;

    QVariant view;
};

#endif /* TAB_H_ */
