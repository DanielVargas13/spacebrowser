#include <TreeToListProxyModel.h>

#include <QLoggingCategory>

#include <chrono>
#include <stack>
#include <tuple>

#include <iostream>

Q_LOGGING_CATEGORY(ttlProxy, "ttlProxyModel")

TreeToListProxyModel::TreeToListProxyModel()
{
    connect(this, &TreeToListProxyModel::sourceModelChanged,
            this, &TreeToListProxyModel::updateMapping);
}

void TreeToListProxyModel::sourceRowsInserted(const QModelIndex &parent, int first, int last)
{
    int count = last - first + 1;
    qCCritical(ttlProxy, "source inserted: %i:%i, cnt:%i", first, last, count);

    QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(sourceModel());
    QStandardItem* parentItem = model->itemFromIndex(parent);

    /// If no valid parent item given, add at the end of the list
    ///
    int parentPos = -1;
    if (parentItem)
        parentPos = parentItem->row();
    else
        parentItem = model->invisibleRootItem();

    /// Begin insertion procedure
    ///
    int localFirst = parentPos + 1 + first;
    int localLast = parentPos + 1 + last;
    qCDebug(ttlProxy, "Adding rows: first: %i, last: %i", localFirst, localLast);
    beginInsertRows(QModelIndex(), localFirst, localLast);

    qCDebug(ttlProxy, "------ Before move ------");
    for (auto it: viewId2ModelId)
    {
        qCDebug(ttlProxy, "viewId: %i, modelId: %i", it.first, it.second);
    }
    for (auto it: toSource)
    {
        QVariant d = it.second->data();
        qCDebug(ttlProxy, "toSource: %i, viewId: %i", it.first, d.toInt());
    }
    for (auto it: fromSource)
    {
        QVariant d = it.first->data();
        qCDebug(ttlProxy, "fromSource: %i, viewId: %i", it.second, d.toInt());
    }
    qCDebug(ttlProxy, "-------------------------");


    /// First move all following rows forward by count
    ///

    for (int i = 0; i < rowCount() - localFirst; ++i)
    {
        int oldPos = rowCount() - 1 - i;
        int newPos = rowCount() - 1 - i + count;
        qCDebug(ttlProxy, "Moving row %i to %i, localFirst: %i", oldPos, newPos, localFirst);

        toSource[newPos] = toSource[oldPos];
        fromSource[toSource[newPos]] = newPos;
        QVariant d = toSource[newPos]->data();
        viewId2ModelId[d.toInt()] = newPos;
        qCDebug(ttlProxy, "viewId2ModelId[%i] = %i", d.toInt(), newPos);
    }

    qCDebug(ttlProxy, "------ After move ------");
    for (auto it: viewId2ModelId)
    {
        qCDebug(ttlProxy, "viewId: %i, modelId: %i", it.first, it.second);
    }
    for (auto it: toSource)
    {
        QVariant d = it.second->data();
        qCDebug(ttlProxy, "toSource: %i, viewId: %i", it.first, d.toInt());
    }
    for (auto it: fromSource)
    {
        QVariant d = it.first->data();
        qCDebug(ttlProxy, "fromSource: %i, viewId: %i", it.second, d.toInt());
    }
    qCDebug(ttlProxy, "-------------------------");


    /// Add new row
    ///
    for (int i = 0; i < count; ++i)
    {
        int newPos = localFirst + i;
        qCDebug(ttlProxy, "Adding new row from %i at %i", first + i, newPos);

//        QModelIndex newRow =
//        QStandardItem* newItem = model->itemFromIndex(newRow);
        QStandardItem* newItem = parentItem->child(first + i, 0);

        toSource[newPos] = newItem;
        fromSource[newItem] = newPos;

        QVariant d = newItem->data(); // 3 = id (viewId)
        viewId2ModelId[d.toInt()] = newPos;
        rows++;

        qCDebug(ttlProxy, "Added entry for viewId: %i", d.toInt());
    }

    qCDebug(ttlProxy, "------ After add ------");
    for (auto it: viewId2ModelId)
    {
        qCDebug(ttlProxy, "viewId: %i, modelId: %i", it.first, it.second);
    }
    for (auto it: toSource)
    {
        QVariant d = it.second->data();
        qCDebug(ttlProxy, "toSource: %i, viewId: %i", it.first, d.toInt());
    }
    for (auto it: fromSource)
    {
        QVariant d = it.first->data();
        qCDebug(ttlProxy, "fromSource: %i, viewId: %i", it.second, d.toInt());
    }
    qCDebug(ttlProxy, "-------------------------");


    endInsertRows();

/*
    /// Invalid parent means we add root level entry: add as last
    if (!parent.isValid())
    {
        qCDebug(ttlProxy, "!! >> Parent invalid, rows in model: %i",
                rowCount());

        beginInsertRows(QModelIndex(), rows, rows);

        QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(sourceModel());

        QModelIndex newRow = sourceModel()->index(first, 0);
        toSource[rows] = newRow.data();
        fromSource[newRow] = rows;

        QVariant d = sourceModel()->data(newRow, 3); // 3 = id (viewId)
        viewId2ModelId[d.toInt()] = rows;
        qCDebug(ttlProxy, "inserting viewId2ModelId[%i] = %i", d.toInt(), rows);
        rows++;

        endInsertRows();

    }
    else
    {
        qCDebug(ttlProxy, "!!!! >> Parent valid, first: %i", first);

        QModelIndex newRow = sourceModel()->index(first, 0, parent);
        QModelIndex prevItem = newRow.siblingAtRow(newRow.row()-1);
        qCDebug(ttlProxy, "newRow: %i; prevItem: %i", newRow.row(), prevItem.row());

        if (prevItem.isValid())
            prevItem = findLastItemInBranch(prevItem);
        qCDebug(ttlProxy, "prevItem: %i", prevItem.row());

        int parentRow;
        if (prevItem.isValid())
            parentRow = fromSource.at(prevItem);
        else
            parentRow = fromSource.at(parent);

        qCDebug(ttlProxy, "parentRow: %i", parentRow);

        beginInsertRows(QModelIndex(), parentRow+1, parentRow+1);

        /// First move all following rows by one
        for (int i = rowCount(); i > parentRow + 1; --i)
        {
            qCDebug(ttlProxy, "inserting/moving i=%i", i);
            toSource[i] = toSource[i-1]; // indexing changed by -1
            fromSource[toSource[i]] = i;
            QVariant d = sourceModel()->data(toSource[i], 3);
            viewId2ModelId[d.toInt()] = i;
            qCDebug(ttlProxy, "inserting/moving viewId2ModelId[%i] = %i", d.toInt(), i);
        }

        /// Add new row
        qCDebug(ttlProxy, "adding row as: %i", parentRow+1);
        toSource[parentRow+1] = newRow;
        fromSource[newRow] = parentRow+1;

        QVariant d = sourceModel()->data(newRow, 3); // 3 = id (viewId)
        qCDebug(ttlProxy, "viewId: %i", d.toInt());
        viewId2ModelId[d.toInt()] = parentRow+1;
        qCDebug(ttlProxy, "inserting/adding viewId2ModelId[%i] = %i", d.toInt(), parentRow+1);
        rows++;

        endInsertRows();
    }
    */
}

void TreeToListProxyModel::sourceRowsAboutToBeRemoved(
    const QModelIndex &parent, int first, int last)
{
    int count = last - first + 1;
    qCCritical(ttlProxy, "*** source about to be removed: %i:%i, cnt:%i",
               first, last, count);
    qCCritical(ttlProxy, "sizes: %lu, %lu, %lu", toSource.size(), fromSource.size(),
               viewId2ModelId.size());

    qCDebug(ttlProxy, "------ Before delete ------");
    for (auto it: viewId2ModelId)
    {
        qCDebug(ttlProxy, "viewId: %i, modelId: %i", it.first, it.second);
    }
    for (auto it: toSource)
    {
        QVariant d = it.second->data();
        qCDebug(ttlProxy, "toSource: %i, viewId: %i", it.first, d.toInt());
    }
    for (auto it: fromSource)
    {
        QVariant d = it.first->data();
        qCDebug(ttlProxy, "fromSource: %i, viewId: %i", it.second, d.toInt());
    }
    qCDebug(ttlProxy, "-------------------------");


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
        qCDebug(ttlProxy, "Removing viewId: %i", d.toInt());

        if (r1 != 1 || r2 != 1 || r3 != 1)
        {
            qCCritical(ttlProxy, "Failed to remove %i from ts: %i, fs: %i, v2m: %i",
                       first + i, r1, r2, r3);
        }
    }

    qCDebug(ttlProxy, "------ Before move ------");
    for (auto it: viewId2ModelId)
    {
        qCDebug(ttlProxy, "viewId: %i, modelId: %i", it.first, it.second);
    }
    for (auto it: toSource)
    {
        QVariant d = it.second->data();
        qCDebug(ttlProxy, "toSource: %i, viewId: %i", it.first, d.toInt());
    }
    for (auto it: fromSource)
    {
        QVariant d = it.first->data();
        qCDebug(ttlProxy, "fromSource: %i, viewId: %i", it.second, d.toInt());
    }
    qCDebug(ttlProxy, "-------------------------");



    /// Move all following rows back by one
    ///
    for (int i = 0; i < rowCount() - firstRemovedRow - 1; ++i)
    {
        int newPos = firstRemovedRow+i;
        int oldPos = firstRemovedRow+count+i;
        qCDebug(ttlProxy, "moving %i to %i", oldPos, newPos);

        toSource[newPos] = toSource[oldPos];
        fromSource[toSource[newPos]] = newPos;

        QVariant d = toSource[newPos]->data();
        viewId2ModelId[d.toInt()] = newPos;

        qCDebug(ttlProxy, "viewId2ModelId[%i] = %i", d.toInt(), newPos);
    }

    /// Need to remove last rows, as they're duplicated now
    ///
    for (int i = rowCount() - count; i < rowCount(); ++i)
    {
        qCDebug(ttlProxy, "removing dangling element: %i", i);
        toSource.erase(i);
    }

    qCCritical(ttlProxy, "sizes: %lu, %lu, %lu", toSource.size(), fromSource.size(),
               viewId2ModelId.size());

    rows -= count;
    endRemoveRows();


    qCDebug(ttlProxy, "------ After move ------");
    for (auto it: viewId2ModelId)
    {
        qCDebug(ttlProxy, "viewId: %i, modelId: %i", it.first, it.second);
    }
    for (auto it: toSource)
    {
        QVariant d = it.second->data();
        qCDebug(ttlProxy, "toSource: %i, viewId: %i", it.first, d.toInt());
    }
    for (auto it: fromSource)
    {
        QVariant d = it.first->data();
        qCDebug(ttlProxy, "fromSource: %i, viewId: %i", it.second, d.toInt());
    }
    qCDebug(ttlProxy, "-------------------------");
}

QModelIndex TreeToListProxyModel::findLastItemInBranch(const QModelIndex& idx)
{
    if (!idx.isValid())
        return idx;

    QModelIndex last = idx;
    QModelIndex cur = idx;


    int rowCount = sourceModel()->rowCount(cur);
    qCDebug(ttlProxy, "find: rowCount: %i for %i", rowCount, cur.row());
    // if rowCount > 0
    while (rowCount > 0)
    {
        cur = cur.child(rowCount-1, 0);
        if (cur.isValid())
        {
            last = cur;
            rowCount = sourceModel()->rowCount(cur);
            qCDebug(ttlProxy, "cur.isValid(), rowCount: %i", rowCount);
        }
        else
        {
            qCDebug(ttlProxy, "!cur.isValid()");
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
    qCDebug(ttlProxy, "Source data changed");

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
        qCDebug(ttlProxy, "noModel");
        return QModelIndex();
    }

    QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(sourceModel());
    auto item = model->itemFromIndex(sourceIndex);

    if (!fromSource.count(item))
    {
        qCDebug(ttlProxy, "Could not map from source: no item in map");
        return QModelIndex();
    }

    return item->index();
}

QModelIndex TreeToListProxyModel::mapToSource(
    const QModelIndex &proxyIndex) const
{
    if (!sourceModel() || !proxyIndex.isValid())
    {
        qCDebug(ttlProxy, "noModel or index invalid");
        return QModelIndex();
    }

    if (!toSource.count(proxyIndex.row()))
    {
        qCDebug(ttlProxy, "noIndex");
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
    qCCritical(ttlProxy, "parent(): NOT IMPLEMENTED");
    // FIXME: implement
    Q_UNUSED(index);
    return QModelIndex();
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

    connect(sourceModel(), &QAbstractItemModel::rowsAboutToBeMoved,
            this, &TreeToListProxyModel::sourceRowsAboutToBeMoved);

    connect(sourceModel(), &QAbstractItemModel::rowsMoved,
            this, &TreeToListProxyModel::sourceRowsMoved);

    connect(sourceModel(), &QAbstractItemModel::dataChanged,
            this, &TreeToListProxyModel::sourceDataChanged);

    connect(sourceModel(), &QAbstractItemModel::layoutAboutToBeChanged,
            this, &TreeToListProxyModel::layoutAboutToBeChanged);
    connect(sourceModel(), &QAbstractItemModel::layoutChanged,
            this, &TreeToListProxyModel::layoutChanged);

    auto start = std::chrono::system_clock::now();

//    QAbstractItemModel* model = sourceModel();
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
//        qCDebug(ttlProxy, "> i=%i, cnt=%i, iter start", i, cnt);

        if (i == cnt)
        {
//            qCDebug(ttlProxy, "i=%i, cnt=%i, popping", i, cnt);
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

//            qCDebug(ttlProxy, "i=%i, cnt=%i, has children", i, cnt);
            entries.push(tup_t(i, cnt, parent));
            parent = child;
            cnt = model->rowCount(child);
            i = 0;
            continue;
        }

//        qCDebug(ttlProxy, "i=%i, cnt=%i, has no children", i, cnt);
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

void TreeToListProxyModel::sourceRowsAboutToBeMoved(const QModelIndex &parent,
    int start, int end, const QModelIndex &destination, int row)
{/*
    qCCritical(ttlProxy, "ToBeMoved: start: %i, end: %i, row: %i",
               start, end, row);

    qCDebug(ttlProxy, "------ ToBeMoved ------");
    for (auto it: viewId2ModelId)
    {
        qCDebug(ttlProxy, "viewId: %i, modelId: %i", it.first, it.second);
    }
    for (auto it: toSource)
    {
        QVariant d = sourceModel()->data(it.second, 3);
        qCDebug(ttlProxy, "toSource: %i, viewId: %i", it.first, d.toInt());
    }
    for (auto it: fromSource)
    {
        qCDebug(ttlProxy, "fromSource: %i, %llu", it.second, it.first.internalId());
    }
    qCDebug(ttlProxy, "-------------------------");

 */
    return;
//    QModelIndex localDest = mapFromSource(destination.child(row, 0));

    for (int i = start; i <= end; ++i)
    {
//        int rowMovedFrom = fromSource.at(sourceModel()->index(i, 0, parent));


    }

    /*
    QModelIndex tbr = sourceModel()->index(first, 0, parent);
    int row = fromSource.at(tbr);

    /// Remove row
    ///
    int r1 = toSource.erase(row);
    int r2 = fromSource.erase(parent);

    QModelIndex deletedRow = sourceModel()->index(first, 0, parent);
    QVariant d = sourceModel()->data(deletedRow, 3); // 3 = id (viewId)
    qCDebug(ttlProxy, "viewId: %i", d.toInt());
    int r3 = viewId2ModelId.erase(d.toInt());

    qCCritical(ttlProxy, "souce removal results: %i, %i, %i", r1, r2, r3);

    /// Move all following rows back by one
    ///
    for (int i = row; i < rowCount() - 1; ++i)
    {
        toSource[row] = toSource[row+1];
        fromSource[toSource[row]] = row;
        QVariant d = sourceModel()->data(toSource[row], 3);
        viewId2ModelId[d.toInt()] = row;
    }

    /// Need to remove last row, as it is duplicated now
    ///
    toSource.erase(row+1);
    qCCritical(ttlProxy, "sizes: %lu, %lu, %lu", toSource.size(), fromSource.size(),
               viewId2ModelId.size());

    beginRemoveRows(QModelIndex(), row, row);
    rows--;
    endRemoveRows();
    */
}

void TreeToListProxyModel::sourceRowsMoved(const QModelIndex &parent,
    int start, int end, const QModelIndex &destination, int row)
{/*
    qCCritical(ttlProxy, "MOVED: start: %i, end: %i, row: %i", start, end, row);

    qCDebug(ttlProxy, "------ Moved ------");
    for (auto it: viewId2ModelId)
    {
        qCDebug(ttlProxy, "viewId: %i, modelId: %i", it.first, it.second);
    }
    for (auto it: toSource)
    {
        QVariant d = sourceModel()->data(it.second, 3);
        qCDebug(ttlProxy, "toSource: %i, viewId: %i", it.first, d.toInt());
    }
    for (int i = start; i < end; ++i)
    {
        qCDebug(ttlProxy, "orig: %i", parent.child(i, 3).data().toInt());
    }
    for (int i = row; i < row + end - start; ++i)
    {
        qCDebug(ttlProxy, "fixed: %i", destination.child(i, 3).data().toInt());
    }
    for (auto it: fromSource)
    {
        qCDebug(ttlProxy, "fromSource: %i, %llu", it.second, it.first.internalId());
    }
    qCDebug(ttlProxy, "-------------------------");


    /// No need to move anything, children are flattened below parent
    if (parent.parent() == destination &&
        parent.row() + 1 == row)
        return;
*/
    /* ------- FIXME ------ */
/*
    for (int i = 1; i < 7; ++i)
        std::cout << i << ": " << getModelId(i) << std::endl;

    qCCritical(ttlProxy, "MOVING NOT IMPLEMENTED YET");

    return;



    QModelIndex first = mapFromSource(parent.child(start, 0));
    QModelIndex last = mapFromSource(parent.child(end, 0));
    QModelIndex target = mapFromSource(destination.child(row, 0));

    qCCritical(ttlProxy, "MOVING: first: %i, last: %i, target: %i", first.row(),
        last.row(), target.row());

    beginMoveRows(QModelIndex(), first.row(), last.row(), QModelIndex(), target.row());
    endMoveRows();
*/
//FIXME: update mappings

}

void TreeToListProxyModel::layoutAboutToBeChanged()
{
    /*
    qCDebug(ttlProxy, "------ ToBeChanged ------");
    for (auto it: viewId2ModelId)
    {
        qCDebug(ttlProxy, "viewId: %i, modelId: %i", it.first, it.second);
    }
    for (auto it: toSource)
    {
        QVariant d = sourceModel()->data(it.second, 3);
        qCDebug(ttlProxy, "toSource: %i, viewId: %i", it.first, d.toInt());
    }
    for (auto it: fromSource)
    {
        qCDebug(ttlProxy, "fromSource: %i, %llu", it.second, it.first.internalId());
    }
    qCDebug(ttlProxy, "-------------------------");
    */
}

void TreeToListProxyModel::layoutChanged()
{
    /*
    qCDebug(ttlProxy, "------ Changed ------");
    for (auto it: viewId2ModelId)
    {
        qCDebug(ttlProxy, "viewId: %i, modelId: %i", it.first, it.second);
    }
    for (auto it: toSource)
    {
        QVariant d = sourceModel()->data(it.second, 3);
        qCDebug(ttlProxy, "toSource: %i, viewId: %i", it.first, d.toInt());
    }
    for (auto it: fromSource)
    {
        QVariant d = sourceModel()->data(it.first, 3);
        qCDebug(ttlProxy, "fromSource: %i, viewId: %i", it.second, d.toInt());
    }
    qCDebug(ttlProxy, "-------------------------");
    */
}

void TreeToListProxyModel::indicesUpdated(std::map<QModelIndex, QModelIndex> indices)
{
    /*
    for (auto index: indices)
    {
        if (!fromSource.count(index.first))
        {
            qCDebug(ttlProxy, "Index not present");
            continue;
        }
        qCDebug(ttlProxy, "Updating index");
        int pos = fromSource[index.first];
        toSource[pos] = index.second;
        fromSource.erase(index.first);
        fromSource[index.second] = pos;
        }
    */
}
