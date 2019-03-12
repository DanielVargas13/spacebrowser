#ifndef TREEMODEL2_H_
#define TREEMODEL2_H_

#include <QStandardItemModel>

class TreeModel2 : public QStandardItemModel
{
public:
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
        const QModelIndex &destinationParent, int destinationChild) override
    {

        beginMoveRows(sourceParent, sourceRow, sourceRow+count-1,
            destinationParent, destinationChild);
        endMoveRows();

        return true;
    }

};

#endif /* TREEMODEL2_H_ */
