#ifndef TREEITEM_H
#define TREEITEM_H

#include <QtCore/QList>
#include <QtCore/QVariant>

class TreeItem
{
public:
    TreeItem(const QList<QVariant> &data, TreeItem *parent = 0);
    ~TreeItem();

    void appendChild(TreeItem *child);

    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TreeItem *parent();

private:
    QList<TreeItem*> m_childItems;
    QList<QVariant>  m_itemData;
    TreeItem*        m_parentItem;
};

#endif
