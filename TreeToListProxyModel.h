#ifndef TREETOLISTPROXYMODEL_H
#define TREETOLISTPROXYMODEL_H

#include <QAbstractProxyModel>

class TreeToListProxyModel : public QAbstractProxyModel
{
public:
    TreeToListProxyModel();

    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    void updateMapping();

};

#endif /* TREETOLISTPROXYMODEL_H */
