#ifndef TREETOLISTPROXYMODEL_H
#define TREETOLISTPROXYMODEL_H

#include <QAbstractProxyModel>
#include <QIdentityProxyModel>
#include <QHash>

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
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE int getModelId(int viewId) const;

//signals:
//    void rowsInsertedXX(const QModelIndex &parent, int first, int last, QPrivateSignal());

private:
    void updateMapping();
    void myRowInserted(const QModelIndex &parent, int first, int last);
    void sourceRowsInserted(const QModelIndex &parent, int first, int last);
    void sourceRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void sourceDataChanged(const QModelIndex &topLeft,
                           const QModelIndex &bottomRight,
                           const QVector<int> &roles = QVector<int>());
    QModelIndex findLastItemInBranch(const QModelIndex& idx);

    unsigned int rows = 0;
    std::map<int, QModelIndex> toSource;
    std::map<QModelIndex, int> fromSource;
    std::map<int, int> viewId2ModelId;
    QHash<int, QByteArray> rNames;

};

#endif /* TREETOLISTPROXYMODEL_H */
