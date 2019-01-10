#ifndef TREETOLISTPROXYMODEL_H
#define TREETOLISTPROXYMODEL_H

#include <QAbstractProxyModel>
#include <QIdentityProxyModel>
#include <QHash>

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
    QVariant data(const QModelIndex &index, int role) const override;

    void setRoleNames(const QHash<int, QByteArray> &roleNames);
    QHash<int, QByteArray> roleNames() const override;

    int getModelId(int viewId) const;

private:
    void updateMapping();

    unsigned int rows = 0;
    std::map<int, QModelIndex> toSource;
    std::map<QModelIndex, int> fromSource;
    std::map<int, int> viewId2ModelId;
    QHash<int, QByteArray> rNames;

};

#endif /* TREETOLISTPROXYMODEL_H */
