#ifndef TREEMODEL2_H_
#define TREEMODEL2_H_

#include <QStandardItemModel>

#include <map>

class TreeModel2 : public QStandardItemModel
{
    Q_OBJECT
public:
    bool moveRows(const QModelIndex &srcParent, int srcRow, int count,
                  const QModelIndex &dstParent, int dstChild) override;

signals:
    void indicesUpdated(std::map<QModelIndex, QModelIndex>);
};

#endif /* TREEMODEL2_H_ */
