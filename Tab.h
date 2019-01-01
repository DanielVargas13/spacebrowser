#ifndef TAB_H_
#define TAB_H_

#include <TreeNode.h>

#include <QVariant>

#include <vector>

class Tab : public TreeNode
{
public:

    /// It is enough to compare id, it's uniqueness is guaranteed
    /// by the database
    ///
    bool operator== (const Tab &c1, const Tab &c2)
    {
        return id == id;
    }

    QVariant data(int column) const override
    {
        return id;
    }

    TreeNode* child(int row) override
    {
        if (row >= children.size())
            return nullptr;

        return children[row];
    }

    TreeNode* parentItem() override
    {
        return parent;
    }

    int row() const override
    {
        parent.indexOf(this);

        return 0;
    }

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

    int columnCount() const override
    {
        return columnCount;
    }

private:
    int columnCount = 1;

    int id;
    Tab parent; // FIXME: shared and weak pointers
    std::vector<Tab> children;

    QString url;
    QString title;
    QString icon;

    QVariant view;
};

#endif /* TAB_H_ */
