#ifndef TREETOLISTPROXYMODEL_H
#define TREETOLISTPROXYMODEL_H

#include <QAbstractProxyModel>
#include <QIdentityProxyModel>
#include <QStandardItemModel>

#ifdef TEST_BUILD
class ViewHandler_test;
#endif

class TreeToListProxyModel : public QAbstractProxyModel
{
    Q_OBJECT

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
    void setDbName(QString _dbName);
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE int getModelId(int viewId) const;

private:
    void updateMapping();
    void sourceRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void sourceRowsInserted(const QModelIndex &parent, int first, int last);
    void sourceDataChanged(const QModelIndex &topLeft,
                           const QModelIndex &bottomRight,
                           const QVector<int> &roles = QVector<int>());
    QModelIndex findLastItemInBranch(const QModelIndex& idx);

    unsigned int rows = 0;
    std::map<int, QStandardItem*> toSource;
    std::map<QStandardItem*, int> fromSource;
    std::map<int, int> viewId2ModelId;  // viewId to id in proxy model
    QHash<int, QByteArray> rNames;
    QString dbName;

#ifdef TEST_BUILD
    friend ViewHandler_test;
#endif

};

#endif /* TREETOLISTPROXYMODEL_H */
