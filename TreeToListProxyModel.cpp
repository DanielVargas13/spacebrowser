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

    // FIXME: probably remove this, doesn't seem to be triggered
//    connect(this, &TreeToListProxyModel::rowsInserted,
//            this, &TreeToListProxyModel::myRowInserted);

//    connect(this, &TreeToListProxyModel::rowsInsertedXX,
//            this, &TreeToListProxyModel::rowsInserted);
}

//void TreeToListProxyModel::myRowInserted(const QModelIndex &parent, int first, int last)
//{
//    qCCritical(ttlProxy, "my inserted: %i", first);
//}

void TreeToListProxyModel::sourceRowsInserted(const QModelIndex &parent, int first, int last)
{
    qCCritical(ttlProxy, "source inserted: %i", first);

    /// Invalid parent means we add root level entry: add as last
    if (!parent.isValid())
    {
        qCDebug(ttlProxy, "!! >> Parent invalid, rows in model: %i",
                rowCount());

        beginInsertRows(QModelIndex(), rows, rows);

        QModelIndex newRow = sourceModel()->index(first, 0);
        toSource[rows] = newRow;
        fromSource[newRow] = rows;

        QVariant d = sourceModel()->data(newRow, 3); // 3 = id (viewId)
        viewId2ModelId[d.toInt()] = rows;
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
        for (int i = rowCount(); i > parentRow; --i)
        {
            toSource[i+1] = toSource[i];
            fromSource[toSource[i+1]] = i+1;
            QVariant d = sourceModel()->data(toSource[i], 3);
            viewId2ModelId[d.toInt()] = i+1;
        }

        /// Add new row
        qCDebug(ttlProxy, "adding row as: %i", parentRow+1);
        toSource[parentRow+1] = newRow;
        fromSource[newRow] = parentRow+1;

        QVariant d = sourceModel()->data(newRow, 3); // 3 = id (viewId)
        qCDebug(ttlProxy, "viewId: %i", d.toInt());
        viewId2ModelId[d.toInt()] = parentRow+1;
        rows++;

        endInsertRows();
    }
}

void TreeToListProxyModel::sourceRowsAboutToBeRemoved(
    const QModelIndex &parent, int first, int last)
{
    qCCritical(ttlProxy, "source about to be removed: %i", first);
    qCCritical(ttlProxy, "sizes: %lu, %lu, %lu", toSource.size(), fromSource.size(),
               viewId2ModelId.size());

    QModelIndex tbr = sourceModel()->index(first, 0, parent);
    int row = fromSource.at(tbr);

    /// Remove row
    ///
    int r1 = toSource.erase(row);
    int r2 = fromSource.erase(tbr);

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

    if (!fromSource.count(sourceIndex))
    {
        qCDebug(ttlProxy, "noIndex");
        return QModelIndex();
    }

    qCDebug(ttlProxy, "trying to return");
    return createIndex(fromSource.at(sourceIndex), 0, nullptr);
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

    return toSource.at(proxyIndex.row());
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

    auto start = std::chrono::system_clock::now();

    QAbstractItemModel* model = sourceModel();

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
            toSource[rows] = child;
            fromSource[child] = rows;
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
        toSource[rows] = child;
        fromSource[child] = rows;
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
{
    return;
//    QModelIndex localDest = mapFromSource(destination.child(row, 0));

    for (int i = start; i <= end; ++i)
    {
        int rowMovedFrom = fromSource.at(sourceModel()->index(i, 0, parent));


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
{

    qCCritical(ttlProxy, "MOVED: start: %i, end: %i, row: %i", start, end, row);


    QModelIndex first = mapFromSource(parent.child(start, 0));
    QModelIndex last = mapFromSource(parent.child(end, 0));
    QModelIndex target = mapFromSource(destination.child(row, 0));

    qCCritical(ttlProxy, "MOVING: first: %i, last: %i, target: %i", first.row(),
        last.row(), target.row());

    beginMoveRows(QModelIndex(), first.row(), last.row(), QModelIndex(), target.row());
    endMoveRows();

//FIXME: update mappings

}
