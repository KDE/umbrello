#include "umlclassifierlistitemlist.h"
#include "classifierlistitem.h"
#include <kdebug.h>
#include <klocale.h>

void UMLClassifierListItemList::copyInto (UMLClassifierListItemList *rhs) const
{
	// Prevent copying to yourself. (Can cause serious injuries)
	if (rhs == this) return;
	
	rhs->clear();

	// Suffering from const; we shall not modify our object.
	UMLClassifierListItemList *tmp = new UMLClassifierListItemList(*this);

	UMLClassifierListItem *item;
	for (item = tmp->first(); item; item = tmp->next() )
	{
		rhs->append(item->clone());
	}
	delete tmp;
}


UMLClassifierListItemList* UMLClassifierListItemList::clone () const
{
	UMLClassifierListItemList *clone = new UMLClassifierListItemList();
	copyInto(clone);
	return clone;
}



