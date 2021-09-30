#include "TreeItem.h"

TreeItem::TreeItem(const QVector<QVariant> &data, TreeItem *parent)
    : _itemData(data)
    , _parentItem(parent)
    , _checked(Qt::Checked)
{}

TreeItem::~TreeItem()
{
    qDeleteAll(_childItems);
}

TreeItem *TreeItem::child(int number)
{
    if (number < 0 || number >= _childItems.size())
        return nullptr;
    return _childItems.at(number);
}

int TreeItem::childCount() const
{
    return _childItems.count();
}

int TreeItem::childNumber() const
{
    if (_parentItem)
        return _parentItem->_childItems.indexOf(const_cast<TreeItem*>(this));
    return 0;
}

int TreeItem::columnCount() const
{
    return _itemData.count();
}

QVariant TreeItem::data(int column) const
{
    if (column < 0 || column >= _itemData.size())
        return QVariant();
    return _itemData.at(column);
}

bool TreeItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > _childItems.size())
        return false;

    for (int row = 0; row < count; ++row) {
        QVector<QVariant> data(columns);
        TreeItem *item = new TreeItem(data, this);
        _childItems.insert(position, item);
    }

    return true;
}

bool TreeItem::insertColumns(int position, int columns)
{
    if (position < 0 || position > _itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        _itemData.insert(position, QVariant());

    for (TreeItem *child : qAsConst(_childItems))
        child->insertColumns(position, columns);

    return true;
}

TreeItem *TreeItem::parent()
{
    return _parentItem;
}

bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > _childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete _childItems.takeAt(position);

    return true;
}

bool TreeItem::removeColumns(int position, int columns)
{
    if (position < 0 || position + columns > _itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        _itemData.remove(position);

    for (TreeItem *child : qAsConst(_childItems))
        child->removeColumns(position, columns);

    return true;
}

bool TreeItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= _itemData.size())
        return false;

    _itemData[column] = value;
    return true;
}
