#include "umlobjectlist.h"
#include "umlobject.h"
#include <kdebug.h>
#include <klocale.h>

void UMLObjectList::copyInto (UMLObjectList *rhs) const
{
	// Don't copy yourself.
	if (rhs == this) return;
	
	rhs->clear();

	// Suffering from const; we shall not modify our object.
	UMLObjectList *tmp = new UMLObjectList(*this);

	UMLObject *item;
	for (item = tmp->first(); item; item = tmp->next() )
	{
		rhs->append(item->clone());
	}
	delete tmp;
}


UMLObjectList* UMLObjectList::clone () const
{
	UMLObjectList *clone = new UMLObjectList();
	copyInto(clone);
	return clone;
}



