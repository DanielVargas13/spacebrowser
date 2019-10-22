#ifndef TREENODE_H_
#define TREENODE_H_

#include <QVariant>

class TreeNode
{
public:

    virtual QVariant data(int column) const
    {
        return QVariant();
    }

    virtual TreeNode* child(int row)
    {
        return nullptr;
    }

    virtual TreeNode* parentItem()
    {
        return nullptr;
    }

    virtual int row() const
    {
        return 0;
    }

    virtual int indexOf(const TreeNode* t) const
    {
        return 0;
    }

    virtual int childCount() const
    {
        return 0;
    }

    virtual int columnCount() const
    {
        return 0;
    }

};

#endif /* TREENODE_H_ */
