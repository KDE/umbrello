/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "completioncodemodel.h"

#include <QHash>
#include <QVector>

#include <language/duchain/appendedlist.h>
#include <language/duchain/repositories/itemrepository.h>
#include <language/duchain/identifier.h>
#include <language/duchain/indexedstring.h>
#include <util/embeddedfreetree.h>
#include <language/duchain/referencecounting.h>

#define ifDebug(x)

namespace Php {

class CompletionCodeModelItemHandler {
    public:
    static int leftChild(const CompletionCodeModelItem& m_data) {
        return (int)m_data.referenceCount;
    }
    static void setLeftChild(CompletionCodeModelItem& m_data, int child) {
        m_data.referenceCount = (uint)child;
    }
    static int rightChild(const CompletionCodeModelItem& m_data) {
        return (int)m_data.uKind;
    }
    static void setRightChild(CompletionCodeModelItem& m_data, int child) {
        m_data.uKind = (uint)child;
    }
    //Copies this item into the given one
    static void copyTo(const CompletionCodeModelItem& m_data, CompletionCodeModelItem& data) {
      data = m_data;
    }

    static void createFreeItem(CompletionCodeModelItem& data) {
        data = CompletionCodeModelItem();
        data.referenceCount = (uint)-1;
        data.uKind = (uint)-1;
    }

    static bool isFree(const CompletionCodeModelItem& m_data) {
        return !m_data.id.isValid();
    }

    static const CompletionCodeModelItem& data(const CompletionCodeModelItem& m_data) {
      return m_data;
    }

    static bool equals(const CompletionCodeModelItem& m_data, const CompletionCodeModelItem& rhs) {
      return m_data.id == rhs.id;
    }
};


DEFINE_LIST_MEMBER_HASH(CompletionCodeModelRepositoryItem, items, CompletionCodeModelItem)

class CompletionCodeModelRepositoryItem {
  public:
  CompletionCodeModelRepositoryItem() : centralFreeItem(-1) {
    initializeAppendedLists();
  }
  CompletionCodeModelRepositoryItem(const CompletionCodeModelRepositoryItem& rhs, bool dynamic = true) : file(rhs.file), centralFreeItem(rhs.centralFreeItem) {
    initializeAppendedLists(dynamic);
    copyListsFrom(rhs);
  }

  ~CompletionCodeModelRepositoryItem() {
    freeAppendedLists();
  }

  unsigned int hash() const {
    //We only compare the declaration. This allows us implementing a map, although the item-repository
    //originally represents a set.
    return file.index();
  }

  uint itemSize() const {
    return dynamicSize();
  }

  uint classSize() const {
    return sizeof(CompletionCodeModelRepositoryItem);
  }

  KDevelop::IndexedString file;
  int centralFreeItem;

  START_APPENDED_LISTS(CompletionCodeModelRepositoryItem);
  APPENDED_LIST_FIRST(CompletionCodeModelRepositoryItem, CompletionCodeModelItem, items);
  END_APPENDED_LISTS(CompletionCodeModelRepositoryItem, items);
};

class CodeModelRequestItem {
  public:

  CodeModelRequestItem(const CompletionCodeModelRepositoryItem& item) : m_item(item) {
  }
  enum {
    AverageSize = 30+8 //This should be the approximate average size of an Item
  };

  unsigned int hash() const {
    return m_item.hash();
  }

  uint itemSize() const {
      return m_item.itemSize();
  }

  void createItem(CompletionCodeModelRepositoryItem* item) const {
    Q_ASSERT(KDevelop::shouldDoDUChainReferenceCounting(item));
    Q_ASSERT(KDevelop::shouldDoDUChainReferenceCounting(((char*)item) + (itemSize()-1)));
    new (item) CompletionCodeModelRepositoryItem(m_item, false);
  }

  static void destroy(CompletionCodeModelRepositoryItem* item, KDevelop::AbstractItemRepository&) {
    Q_ASSERT(KDevelop::shouldDoDUChainReferenceCounting(item));
//     Q_ASSERT(shouldDoDUChainReferenceCounting(((char*)item) + (itemSize()-1)));
    item->~CompletionCodeModelRepositoryItem();
  }

  static bool persistent(const CompletionCodeModelRepositoryItem* item) {
    Q_UNUSED(item);
    return true;
  }

  bool equals(const CompletionCodeModelRepositoryItem* item) const {
    return m_item.file == item->file;
  }

  const CompletionCodeModelRepositoryItem& m_item;
};


class CompletionCodeModelPrivate {
public:

  CompletionCodeModelPrivate() : m_repository("Php Completion Code Model") {
  }
  //Maps declaration-ids to items
  KDevelop::ItemRepository<CompletionCodeModelRepositoryItem, CodeModelRequestItem> m_repository;
};

CompletionCodeModel::CompletionCodeModel() : d(new CompletionCodeModelPrivate())
{
}

CompletionCodeModel::~CompletionCodeModel()
{
  delete d;
}

void CompletionCodeModel::addItem(const KDevelop::IndexedString& file, const KDevelop::IndexedQualifiedIdentifier& id, const KDevelop::IndexedString & prettyName, CompletionCodeModelItem::Kind kind)
{
  ifDebug( kDebug() << "addItem" << file.str() << id.identifier().toString() << id.index; )

  if(!id.isValid())
    return;
  CompletionCodeModelRepositoryItem item;
  item.file = file;
  CodeModelRequestItem request(item);

  uint index = d->m_repository.findIndex(item);

  CompletionCodeModelItem newItem;
  newItem.id = id;
  newItem.kind = kind;
  newItem.prettyName = prettyName;
  newItem.referenceCount = 1;

  if(index) {
    const CompletionCodeModelRepositoryItem* oldItem = d->m_repository.itemFromIndex(index);
    KDevelop::EmbeddedTreeAlgorithms<CompletionCodeModelItem, CompletionCodeModelItemHandler> alg(oldItem->items(), oldItem->itemsSize(), oldItem->centralFreeItem);

    int listIndex = alg.indexOf(newItem);

    QMutexLocker lock(d->m_repository.mutex());

    KDevelop::DynamicItem<CompletionCodeModelRepositoryItem, true> editableItem = d->m_repository.dynamicItemFromIndex(index);
    CompletionCodeModelItem* items = const_cast<CompletionCodeModelItem*>(editableItem->items());

    if(listIndex != -1) {
      //Only update the reference-count
        ++items[listIndex].referenceCount;
        items[listIndex].kind = kind;
        items[listIndex].prettyName = prettyName;
        return;
    }else{
      //Add the item to the list
      KDevelop::EmbeddedTreeAddItem<CompletionCodeModelItem, CompletionCodeModelItemHandler> add(items, editableItem->itemsSize(), editableItem->centralFreeItem, newItem);

      if(add.newItemCount() != editableItem->itemsSize()) {
        //The data needs to be transferred into a bigger list. That list is within "item".

        item.itemsList().resize(add.newItemCount());
        add.transferData(item.itemsList().data(), item.itemsList().size(), &item.centralFreeItem);

        d->m_repository.deleteItem(index);
      }else{
        //We're fine: The item fits into the existing list.
        return;
      }
    }
  }else{
    //We're creating a new index
    item.itemsList().append(newItem);
  }

  Q_ASSERT(!d->m_repository.findIndex(request));

  //This inserts the changed item
  volatile uint newIndex = d->m_repository.index(request);
  Q_UNUSED(newIndex);
  ifDebug( kDebug() << "new index" << newIndex; )

  Q_ASSERT(d->m_repository.findIndex(request));
}

void CompletionCodeModel::updateItem(const KDevelop::IndexedString& file, const KDevelop::IndexedQualifiedIdentifier& id, const KDevelop::IndexedString & prettyName, CompletionCodeModelItem::Kind kind)
{
  ifDebug( kDebug() << file.str() << id.identifier().toString() << kind; )

  if(!id.isValid())
    return;

  CompletionCodeModelRepositoryItem item;
  item.file = file;
  CodeModelRequestItem request(item);

  CompletionCodeModelItem newItem;
  newItem.id = id;
  newItem.kind = kind;
  newItem.prettyName = prettyName;
  newItem.referenceCount = 1;

  uint index = d->m_repository.findIndex(item);

  if(index) {
    //Check whether the item is already in the mapped list, else copy the list into the new created item
    QMutexLocker lock(d->m_repository.mutex());
    KDevelop::DynamicItem<CompletionCodeModelRepositoryItem, true> oldItem = d->m_repository.dynamicItemFromIndex(index);

    KDevelop::EmbeddedTreeAlgorithms<CompletionCodeModelItem, CompletionCodeModelItemHandler> alg(oldItem->items(), oldItem->itemsSize(), oldItem->centralFreeItem);
    int listIndex = alg.indexOf(newItem);
    Q_ASSERT(listIndex != -1);

    CompletionCodeModelItem* items = const_cast<CompletionCodeModelItem*>(oldItem->items());

    Q_ASSERT(items[listIndex].id == id);
    items[listIndex].kind = kind;
    items[listIndex].prettyName = prettyName;

    return;
  }

  Q_ASSERT(0); //The updated item as not in the symbol table!
}

void CompletionCodeModel::removeItem(const KDevelop::IndexedString& file, const KDevelop::IndexedQualifiedIdentifier& id)
{
  if(!id.isValid())
    return;
  ifDebug( kDebug() << "removeItem" << file.str() << id.identifier().toString(); )
  CompletionCodeModelRepositoryItem item;
  item.file = file;
  CodeModelRequestItem request(item);

  uint index = d->m_repository.findIndex(item);

  if(index) {

    CompletionCodeModelItem searchItem;
    searchItem.id = id;

    QMutexLocker lock(d->m_repository.mutex());
    KDevelop::DynamicItem<CompletionCodeModelRepositoryItem, true> oldItem = d->m_repository.dynamicItemFromIndex(index);

    KDevelop::EmbeddedTreeAlgorithms<CompletionCodeModelItem, CompletionCodeModelItemHandler> alg(oldItem->items(), oldItem->itemsSize(), oldItem->centralFreeItem);

    int listIndex = alg.indexOf(searchItem);
    if(listIndex == -1)
      return;

    CompletionCodeModelItem* items = const_cast<CompletionCodeModelItem*>(oldItem->items());

    --items[listIndex].referenceCount;

    if(oldItem->items()[listIndex].referenceCount)
      return; //Nothing to remove, there's still a reference-count left

    //We have reduced the reference-count to zero, so remove the item from the list

    KDevelop::EmbeddedTreeRemoveItem<CompletionCodeModelItem, CompletionCodeModelItemHandler> remove(items, oldItem->itemsSize(), oldItem->centralFreeItem, searchItem);

    uint newItemCount = remove.newItemCount();
    if(newItemCount != oldItem->itemsSize()) {
      if(newItemCount == 0) {
        //Has become empty, delete the item
        d->m_repository.deleteItem(index);
        return;
      }else{
        //Make smaller
        item.itemsList().resize(newItemCount);
        remove.transferData(item.itemsList().data(), item.itemsSize(), &item.centralFreeItem);

        //Delete the old list
        d->m_repository.deleteItem(index);
        //Add the new list
        d->m_repository.index(request);
        return;
      }
    }
  }
}

void CompletionCodeModel::items(const KDevelop::IndexedString& file, uint& count, const CompletionCodeModelItem*& items) const
{
  ifDebug( kDebug() << "items" << file.str(); )

  CompletionCodeModelRepositoryItem item;
  item.file = file;
  CodeModelRequestItem request(item);

  uint index = d->m_repository.findIndex(item);

  if(index) {
    const CompletionCodeModelRepositoryItem* repositoryItem = d->m_repository.itemFromIndex(index);
    ifDebug( kDebug() << "found index" << index << repositoryItem->itemsSize(); )
    count = repositoryItem->itemsSize();
    items = repositoryItem->items();
  }else{
    ifDebug( kDebug() << "found no index"; )
    count = 0;
    items = 0;
  }
}

CompletionCodeModel& CompletionCodeModel::self() {
  static CompletionCodeModel ret;
  return ret;
}

}

