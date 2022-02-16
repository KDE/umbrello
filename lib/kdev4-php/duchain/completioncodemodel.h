/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef COMPLETIONCODEMODEL_H
#define COMPLETIONCODEMODEL_H

#include "phpduchainexport.h"
#include <language/duchain/identifier.h>
#include <language/duchain/indexedstring.h>

namespace KDevelop {

  class Declaration;
  class IndexedDeclaration;
  class DeclarationId;
  class TopDUContext;
  class QualifiedIdentifier;
  class IndexedString;
  class IndexedQualifiedIdentifier;
}

namespace Php {
  struct CompletionCodeModelItem {
    CompletionCodeModelItem() : referenceCount(0), kind(Unknown) {
    }
    enum Kind {
      Unknown = 0,
      Exception = 1
    };
    KDevelop::IndexedQualifiedIdentifier id;
    KDevelop::IndexedString prettyName;
    uint referenceCount;
    union {
      Kind kind;
      uint uKind;
    };
    bool operator<(const CompletionCodeModelItem& rhs) const {
      return id < rhs.id;
    }
  };

/**
 * Persistent store that efficiently holds a list of identifiers and their kind for each declaration-string.
 * */
  class KDEVPHPDUCHAIN_EXPORT CompletionCodeModel {
    public:
    /// Constructor.
    CompletionCodeModel();
    /// Destructor.
    ~CompletionCodeModel();

    ///There can only be one item for each identifier. If an item with this identifier already exists,
    ///the kind is updated.
    void addItem(const KDevelop::IndexedString& file, const KDevelop::IndexedQualifiedIdentifier& id, const KDevelop::IndexedString & prettyName, CompletionCodeModelItem::Kind kind);

    void removeItem(const KDevelop::IndexedString& file, const KDevelop::IndexedQualifiedIdentifier& id);

    ///Updates the kind for the given item. The item must already be in the symbol table.
    void updateItem(const KDevelop::IndexedString& file, const KDevelop::IndexedQualifiedIdentifier& id, const KDevelop::IndexedString & prettyName, CompletionCodeModelItem::Kind kind);

    ///Retrieves all the global identifiers for a file-name in an efficient way.
    ///@param count A reference that will be filled with the count of retrieved items
    ///@param items A reference to a pointer, that will represent the array of items.
    ///             The array may contain items with an invalid identifier, those should be ignored.
    ///             The list is sorted by identifier-index(except for the invalid identifiers in between).
    void items(const KDevelop::IndexedString& file, uint& count, const CompletionCodeModelItem*& items) const;

    static CompletionCodeModel& self();

    private:
      class CompletionCodeModelPrivate* d;
  };
}

#endif
