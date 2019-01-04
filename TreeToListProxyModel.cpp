#include <TreeToListProxyModel.h>

TreeToListProxyModel::TreeToListProxyModel()
{
    connect(this, &TreeToListProxyModel::sourceModelChanged,
            this, &TreeToListProxyModel::updateMapping);
}

QModelIndex TreeToListProxyModel::mapFromSource(
    const QModelIndex &sourceIndex) const
{

}

QModelIndex TreeToListProxyModel::mapToSource(
    const QModelIndex &proxyIndex) const
{
}

QModelIndex TreeToListProxyModel::index(int row, int column,
                  const QModelIndex &parent) const
{
    if (!parent.isValid())
        return QModelIndex();

    if (!hasIndex(row, column, parent))
        return QModelIndex();

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
    // FIXME: return proper count (+ when tree branch collapsed)
    return sourceModel()->rowCount(parent);
}

int TreeToListProxyModel::columnCount(const QModelIndex &parent) const
{
    // FIXME: make sure returns proper value
    return sourceModel()->columnCount(parent);
}

void TreeToListProxyModel::updateMapping()
{

}
