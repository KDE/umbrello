#include "umlattributelist.h"
#include "attribute.h"
#include <kdebug.h>
#include <klocale.h>

void UMLAttributeList::copyInto (UMLAttributeList *rhs) const
{
	// Don't copy yourself.
	if (rhs == this) return;
	
	rhs->clear();

	// Suffering from const; we shall not modify our object.
	UMLAttributeList *tmp = new UMLAttributeList(*this);

	UMLAttribute *item;
	for (item = tmp->first(); item; item = tmp->next() )
	{
		rhs->append(item->clone());
	}
	delete tmp;
}


UMLAttributeList* UMLAttributeList::clone () const
{
	UMLAttributeList *clone = new UMLAttributeList();
	copyInto(clone);
	return clone;
}



