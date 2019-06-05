#include <TreeModel2.h>

#include <Tab.h>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(treeModel2, "treeModel2")

bool TreeModel2::moveRows(const QModelIndex &srcParent, int srcRow, int count,
              const QModelIndex &dstParent, int dstRow)
{
    QStandardItem* srcItem = itemFromIndex(srcParent);
    QStandardItem* dstItem;

    if (dstParent == invisibleRootItem()->index())
        dstItem = invisibleRootItem();
    else
        dstItem = itemFromIndex(dstParent);


    /// Remove items from old position
    ///
    QList<QList<QStandardItem*>> rows;
    for (int i = 0; i < count; ++i)
    {
        rows.append(srcItem->takeRow(srcRow));
    }

    /// Insert rows into new positions
    ///
    for (int i = 0; i < count; ++i)
    {
        dstItem->insertRow(dstRow+i, rows[i]);
        dynamic_cast<Tab*>(rows[i][0])->updateIndent();
    }

    return true;
}
