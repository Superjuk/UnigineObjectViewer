#pragma once

#include <QVariant>
#include <QVector>

class TreeItem
{
public:
    explicit TreeItem(const QVector<QVariant> &data, TreeItem *parent = nullptr);
    ~TreeItem();

    TreeItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    bool insertChildren(int position, int count, int columns);
    bool insertColumns(int position, int columns);
    TreeItem *parent();
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    int childNumber() const;
    bool setData(int column, const QVariant &value);
    int isChecked() {return _checked;}
    void setChecked(int check) {_checked = check;}

private:
    QVector<TreeItem*> _childItems;
    QVector<QVariant> _itemData;
    TreeItem* _parentItem;
    int _checked;
};
