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
    connect(this, &TreeToListProxyModel::rowsInserted,
            this, &TreeToListProxyModel::myRowInserted);

//    connect(this, &TreeToListProxyModel::rowsInsertedXX,
//            this, &TreeToListProxyModel::rowsInserted);
}

void TreeToListProxyModel::myRowInserted(const QModelIndex &parent, int first, int last)
{
    qCCritical(ttlProxy, "my inserted: %i", first);
}

void TreeToListProxyModel::sourceRowsInserted(const QModelIndex &parent, int first, int last)
{
    qCCritical(ttlProxy, "source inserted");

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
        qCDebug(ttlProxy, "!!!! >> Parent valid");

        int parentRow = fromSource.at(parent);
        qCDebug(ttlProxy, "parentRow: %i", parentRow);

        beginInsertRows(QModelIndex(), parentRow+1, parentRow+1);

        for (int i = rowCount(); i > parentRow; --i)
        {
            toSource[i+1] = toSource[i];
            fromSource[toSource[i]] = i+1;
            QVariant d = sourceModel()->data(toSource[i], 3);
            viewId2ModelId[d.toInt()] = i+1;
        }

        QModelIndex newRow = sourceModel()->index(first, 0, parent);
        toSource[parentRow+1] = newRow;
        fromSource[newRow] = parentRow+1;

        QVariant d = sourceModel()->data(newRow, 3); // 3 = id (viewId)
        qCDebug(ttlProxy, "viewId: %i", d.toInt());
        viewId2ModelId[d.toInt()] = parentRow+1;
        rows++;

        endInsertRows();
    }


//    QModelIndex p = mapFromSource(parent);





    //sourceModel()->index(first, 0, parent)
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
//    return sourceModel()->data(toSource.at(index.row()), role);
    return sourceModel()->data(mapToSource(index), role);
}

void TreeToListProxyModel::updateMapping()
{
    if (!sourceModel())
        return;

    connect(sourceModel(), &TreeToListProxyModel::rowsInserted,
            this, &TreeToListProxyModel::sourceRowsInserted);

    auto start = std::chrono::system_clock::now();

    QAbstractItemModel* model = sourceModel();

    using tup_t = std::tuple<int, int, QModelIndex>;
    std::stack<tup_t> entries;

    rows = 0;
    QModelIndex parent;
    int cnt = model->rowCount();
    int i = 0;

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
