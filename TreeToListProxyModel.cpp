#include <TreeToListProxyModel.h>

#include <QLoggingCategory>

#include <chrono>
#include <stack>
#include <tuple>

Q_LOGGING_CATEGORY(ttlProxy, "ttlProxyModel")

TreeToListProxyModel::TreeToListProxyModel()
{
    connect(this, &TreeToListProxyModel::sourceModelChanged,
            this, &TreeToListProxyModel::updateMapping);
}

void TreeToListProxyModel::sourceRowsInserted(const QModelIndex &parent, int first, int last)
{
    int count = last - first + 1;

    QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(sourceModel());
    QStandardItem* parentItem = model->itemFromIndex(parent);

    /// If no valid parent item given, add at the end of the list
    ///
    int localFirst;
    int localLast;
    if (parentItem)
    {
        QStandardItem* prevItem;

        if (first >=1)
        {
            prevItem = parentItem->child(first-1);
            prevItem = model->itemFromIndex(findLastItemInBranch(prevItem->index()));
        }
        else
            prevItem = parentItem;

        localFirst = fromSource.at(prevItem) + 1;
    }
    else
    {
        /// Adding top level tab
        ///
        parentItem = model->invisibleRootItem();

        QStandardItem* prevItem;
        /// -1 is the newly added item, not in mapping yet, -2 is previous item
        if (parentItem->rowCount() >= 2)
        {
            prevItem = parentItem->child(first-1); // FIXME: what if first-1 = 0??
            prevItem = model->itemFromIndex(findLastItemInBranch(prevItem->index()));
            localFirst = fromSource.at(prevItem) + 1;
        }
        else
            localFirst = 0;
    }

    localLast = localFirst + count - 1;

    /// Begin insertion procedure
    ///
    beginInsertRows(QModelIndex(), localFirst, localLast);

    /// First move all following rows forward by count
    ///

    for (int i = 0; i < rowCount() - localFirst; ++i)
    {
        int oldPos = rowCount() - 1 - i;
        int newPos = rowCount() - 1 - i + count;

        toSource[newPos] = toSource[oldPos];
        fromSource[toSource[newPos]] = newPos;
        QVariant d = toSource[newPos]->data();
        viewId2ModelId[d.toInt()] = newPos;
    }

    /// Add new row
    ///
    for (int i = 0; i < count; ++i)
    {
        int newPos = localFirst + i;

        QStandardItem* newItem = parentItem->child(first + i, 0);

        toSource[newPos] = newItem;
        fromSource[newItem] = newPos;

        QVariant d = newItem->data(); // 3 = id (viewId)
        viewId2ModelId[d.toInt()] = newPos;
        rows++;
    }

    endInsertRows();
}

void TreeToListProxyModel::sourceRowsAboutToBeRemoved(
    const QModelIndex &parent, int first, int last)
{
    int count = last - first + 1;

    QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(sourceModel());

    /// Delete rows about to be removed
    ///
    int firstRemovedRow = rowCount();
    for (int i = 0; i < count; ++i)
    {
        QModelIndex tbr = sourceModel()->index(first + i, 0, parent);
        QStandardItem* tbrItem = model->itemFromIndex(tbr);
        int row = fromSource.at(tbrItem);
        if (i == 0)
        {
            firstRemovedRow = row;
            beginRemoveRows(QModelIndex(), row, row + count - 1);
        }

        /// Remove row
        ///
        int r1 = toSource.erase(row);
        int r2 = fromSource.erase(tbrItem);

        QVariant d = tbrItem->data();
        int r3 = viewId2ModelId.erase(d.toInt());

        if (r1 != 1 || r2 != 1 || r3 != 1)
        {
            qCCritical(ttlProxy, "Failed to remove %i from ts: %i, fs: %i, v2m: %i",
                       first + i, r1, r2, r3);
        }
    }

    /// Move all following rows back by one
    ///
    for (int i = 0; i < rowCount() - firstRemovedRow - 1; ++i)
    {
        int newPos = firstRemovedRow+i;
        int oldPos = firstRemovedRow+count+i;

        toSource[newPos] = toSource[oldPos];
        fromSource[toSource[newPos]] = newPos;

        QVariant d = toSource[newPos]->data();
        viewId2ModelId[d.toInt()] = newPos;
    }

    /// Need to remove last rows, as they're duplicated now
    ///
    for (int i = rowCount() - count; i < rowCount(); ++i)
    {
        toSource.erase(i);
    }

    rows -= count;
    endRemoveRows();
}

QModelIndex TreeToListProxyModel::findLastItemInBranch(const QModelIndex& idx)
{
    if (!idx.isValid())
        return idx;

    QModelIndex last = idx;
    QModelIndex cur = idx;


    int rowCount = sourceModel()->rowCount(cur);

    while (rowCount > 0)
    {
        cur = cur.child(rowCount-1, 0);
        if (cur.isValid())
        {
            last = cur;
            rowCount = sourceModel()->rowCount(cur);
        }
        else
        {
            break;
        }
    }

    return last;
}

void TreeToListProxyModel::sourceDataChanged(
    const QModelIndex &topLeft,
    const QModelIndex &bottomRight,
    const QVector<int> &roles)
{
    if (topLeft != bottomRight)
    {
        qCCritical(ttlProxy, "Multiple source data fields changed: "
                   "this is unexpected and unsupported");
    }

    emit dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), roles);

}

QModelIndex TreeToListProxyModel::mapFromSource(
    const QModelIndex &sourceIndex) const
{

    if (!sourceModel())
    {
        qCCritical(ttlProxy, "mapFromSource: no model");
        return QModelIndex();
    }

    QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(sourceModel());
    auto item = model->itemFromIndex(sourceIndex);

    if (!fromSource.count(item))
    {
        qCCritical(ttlProxy, "mapFromSource: no item in map");
        return QModelIndex();
    }

    return index(fromSource.at(item), 0);
}

QModelIndex TreeToListProxyModel::mapToSource(
    const QModelIndex &proxyIndex) const
{
    if (!sourceModel())
    {
        qCCritical(ttlProxy, "mapToSource: no model");
        return QModelIndex();
    }

    if (!proxyIndex.isValid())
    {
        /// This case is triggered by Qt during normal operation
        return QModelIndex();
    }

    if (!toSource.count(proxyIndex.row()))
    {
        qCCritical(ttlProxy, "mapToSource: no index");
        return QModelIndex();
    }

    return toSource.at(proxyIndex.row())->index();
}

QModelIndex TreeToListProxyModel::index(int row, int column,
                  const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        qCDebug(ttlProxy, "! has no index: row: %i, col:%i", row, column);
        return QModelIndex();
    }

    return createIndex(row, column);
}

QModelIndex TreeToListProxyModel::parent(const QModelIndex &index) const
{
    // Not sure which parent should be returned here.
    // Proxy model is flat, so there is no parent
    Q_UNUSED(index);
    return QModelIndex();

    // Code below implements returning of parent according to source model
    /*
    QStandardItem* p = toSource.at(index.row())->parent();
    if (!fromSource.count(p))
    {
        qCDebug(ttlProxy, "parent(): parent not found");
        return QModelIndex();
    }

    return TreeToListProxyModel::index(fromSource.at(p), 0);
    */
}

int TreeToListProxyModel::rowCount(const QModelIndex &parent) const
{
    return rows;
}

int TreeToListProxyModel::columnCount(const QModelIndex &parent) const
{
    // FIXME: make sure returns proper value
    return sourceModel()->columnCount(parent);
}

QVariant TreeToListProxyModel::data(const QModelIndex &index, int role) const
{

    return sourceModel()->data(mapToSource(index), role);
}

void TreeToListProxyModel::updateMapping()
{
    if (!sourceModel())
        return;

    connect(sourceModel(), &QAbstractItemModel::rowsInserted,
            this, &TreeToListProxyModel::sourceRowsInserted);

    connect(sourceModel(), &QAbstractItemModel::rowsAboutToBeRemoved,
            this, &TreeToListProxyModel::sourceRowsAboutToBeRemoved);

    connect(sourceModel(), &QAbstractItemModel::dataChanged,
            this, &TreeToListProxyModel::sourceDataChanged);

    auto start = std::chrono::system_clock::now();

    QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(sourceModel());

    using tup_t = std::tuple<int, int, QModelIndex>;
    std::stack<tup_t> entries;

    rows = 0;
    QModelIndex parent;
    int cnt = model->rowCount();
    int i = 0;

    toSource.clear();
    fromSource.clear();
    viewId2ModelId.clear();

    while(i < cnt || !entries.empty())
    {
        if (i == cnt)
        {
            std::tie(i, cnt, parent) = entries.top();
            entries.pop();
            continue;
        }

        const QModelIndex child = model->index(i, 0, parent);
        if (model->hasChildren(child))
        {
            QStandardItem* childItem = model->itemFromIndex(child);
            toSource[rows] = childItem;
            fromSource[childItem] = rows;
            insertRow(rows);

            QVariant d = sourceModel()->data(child, 3); // 3 = id (viewId)
            viewId2ModelId[d.toInt()] = rows;

            rows++;
            i++;

            entries.push(tup_t(i, cnt, parent));
            parent = child;
            cnt = model->rowCount(child);
            i = 0;
            continue;
        }

        QStandardItem* childItem = model->itemFromIndex(child);
        toSource[rows] = childItem;
        fromSource[childItem] = rows;
        insertRow(rows);

        QVariant d = sourceModel()->data(child, 3); // 3 = id (viewId)
        viewId2ModelId[d.toInt()] = rows;

        rows++;
        i++;
    }

    auto end = std::chrono::system_clock::now();

    qCDebug(ttlProxy, "Proxy Model mapping updated");
    qCDebug(ttlProxy, "Proxy Model rows: %i, %li, %li",
            rows, toSource.size(), fromSource.size());
    std::chrono::duration<double> total = end-start;
    qCDebug(ttlProxy, "Proxy Model load time: %lli ns",
            std::chrono::duration_cast<std::chrono::nanoseconds>(total));

}

void TreeToListProxyModel::setRoleNames(const QHash<int, QByteArray> &roleNames)
{
    rNames = roleNames;
}

QHash<int, QByteArray> TreeToListProxyModel::roleNames() const
{
    return rNames;
}

int TreeToListProxyModel::getModelId(int viewId) const
{
    return viewId2ModelId.at(viewId);
}
