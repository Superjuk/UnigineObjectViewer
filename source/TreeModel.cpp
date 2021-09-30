#include "TreeModel.h"
#include "TreeItem.h"

#include <QtWidgets>
#include <QDebug>
#include <QVariant>

TreeModel::TreeModel(const QStringList &headers, const QVariantList &data, QObject *parent)
    : QAbstractItemModel(parent)
{
    QVector<QVariant> rootData;
    for (const QString &header : headers)
        rootData << header;

    _rootItem = new TreeItem(rootData);
    _setupModelData(data, _rootItem);
}

TreeModel::~TreeModel()
{
    delete _rootItem;
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    TreeItem *item = _getItem(index);

    if(role == Qt::CheckStateRole && index.column() == 0)
        return item->isChecked();

    if(role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    return item->data(index.column());
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if( index.column() == 0 )
        flags |= Qt::ItemIsUserCheckable;

    return flags | Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

TreeItem *TreeModel::_getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return _rootItem;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return _rootItem->data(section);

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    TreeItem *parentItem = _getItem(parent);
    if (!parentItem)
        return QModelIndex();

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

bool TreeModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    beginInsertColumns(parent, position, position + columns - 1);
    const bool success = _rootItem->insertColumns(position, columns);
    endInsertColumns();

    return success;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = _getItem(parent);
    if (!parentItem)
        return false;

    beginInsertRows(parent, position, position + rows - 1);
    const bool success = parentItem->insertChildren(position,
                                                    rows,
                                                    _rootItem->columnCount());
    endInsertRows();

    return success;
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = _getItem(index);
    TreeItem *parentItem = childItem ? childItem->parent() : nullptr;

    if (parentItem == _rootItem || !parentItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool TreeModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
    beginRemoveColumns(parent, position, position + columns - 1);
    const bool success = _rootItem->removeColumns(position, columns);
    endRemoveColumns();

    if (_rootItem->columnCount() == 0)
        removeRows(0, rowCount());

    return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = _getItem(parent);
    if (!parentItem)
        return false;

    beginRemoveRows(parent, position, position + rows - 1);
    const bool success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    const TreeItem *parentItem = _getItem(parent);

    return parentItem ? parentItem->childCount() : 0;
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role == Qt::CheckStateRole && index.column() == 0)
    {
        TreeItem *item = _getItem(index);
        item->setChecked(value.toInt());
        emit dataChanged(index, index, {Qt::CheckStateRole});
        return true;
    }

    if (role == Qt::EditRole && index.column() == 0)
    {
        if(value.toString().isEmpty())
            return false;

        TreeItem *item = _getItem(index);
        bool result = item->setData(index.column(), value);
        if(result)
            emit dataChanged(index, index, {Qt::EditRole});
        return result;
    }

    if (role == Qt::DisplayRole)
        return true;

    return false;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    const bool result = _rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

void TreeModel::_setupModelData(const QVariantList &lines, TreeItem *parent)
{
    const auto columnCount = lines.count();
    for(int col = 0; col < columnCount; ++col)
    {
        auto list = lines.at(col).toList();
        auto childCount = 0;
        for(auto& item : list)
        {
            switch(item.type())
            {
            case QVariant::String:
                if(col == 0)
                    parent->insertChildren(childCount, 1, _rootItem->columnCount());

                ++childCount;
                parent->child(childCount - 1)->setData(col, item.toString());
                if(col == 0)
                    parent->child(childCount - 1)->setData(col+1, item.toString());
                break;
            case QVariant::Map:
            {
                const auto keys = item.toMap().keys();
                for(auto& key : keys)
                {
                    if(col == 0)
                        parent->insertChildren(childCount, 1, _rootItem->columnCount());

                    ++childCount;
                    parent->child(childCount - 1)->setData(col, key);
                    _setupModelData(item.toMap().value(key).toList(), parent->child(childCount - 1));
                }
                break;
            }
            default:
                continue;
            }
        }
    }
}
