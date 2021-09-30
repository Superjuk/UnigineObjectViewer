#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class TreeItem;

/*
// Example
const QStringList headers({tr("Title"), tr("Description")});

auto item6List = QVariantMap{{"Item6", QVariantList{"SubItem61", "SubItem62", "SubItem63"}}};
auto item8List = QVariantMap{{"Item8", QVariantList{"SubItem81", "SubItem82", "SubItem83"}}};
auto item7List = QVariantMap{{"Item7", QVariantList{"SubItem71", "SubItem72", item8List}}};
auto itemList = QVariantMap{{"Item5", QVariantList{"SubItem1", item7List, "SubItem3"}}};
auto topLevelItems = QVariantList{"Item1", "Item2", itemList, "Item3", item6List};
*/
class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TreeModel(const QStringList &headers, const QVariantList &data,
              QObject *parent = nullptr);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role = Qt::EditRole) override;

    bool insertColumns(int position, int columns,
                       const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int position, int columns,
                       const QModelIndex &parent = QModelIndex()) override;
    bool insertRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex()) override;

private:
    void _setupModelData(const QVariantList &lines, TreeItem *parent);
    TreeItem *_getItem(const QModelIndex &index) const;

    TreeItem *_rootItem;
};
