#ifndef TAB_H_
#define TAB_H_

#include <TreeNode.h>

#include <QStandardItem>
#include <QVariant>

#include <iostream>
#include <vector>

class Tab : public QStandardItem //public TreeNode
{
public:
//    Q_PROPERTY(QString title MEMBER title NOTIFY titleChanged);

    /// It is enough to compare id, it's uniqueness is guaranteed
    /// by the database
    ///
    bool operator== (const Tab &c1)
    {
        return this->id == c1.id;
    }

    QVariant data(int column) const override
    {
        std::cout << ">>>>>> data(): column: " << column << std::endl;
        switch (column)
        {
            case 0: return url;
            case 1: return title;
            case 2: return icon;
            case 3: return id;
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

private:
    int columnCnt = 1;

    int id = 1;
    Tab* parent; // FIXME: shared and weak pointers
    std::vector<Tab> children;

    QString url;
    QString title = "test";
    QString icon;

    QVariant view;
};

#endif /* TAB_H_ */
