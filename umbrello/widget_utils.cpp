 /*
  *  copyright (C) 2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// own header
#include "widget_utils.h"

// app includes
#include "umlwidget.h"
#include "objectwidget.h"

namespace Umbrello {

UMLWidget* findWidget(Uml::IDType id,
		      const UMLWidgetList& widgets,
		      const MessageWidgetList* pMessages /* = NULL */)
{
	UMLWidgetListIt it( widgets );
	UMLWidget * obj = NULL;
	while ( (obj = it.current()) != NULL ) {
		++it;
		if (obj->getBaseType() == Uml::wt_Object) {
			if (static_cast<ObjectWidget *>(obj)->getLocalID() == id)
				return obj;
		} else if (obj->getID() == id) {
			return obj;
		}
	}

	if (pMessages == NULL)
		return NULL;

	MessageWidgetListIt mit( *pMessages );
	while ( (obj = (UMLWidget*)mit.current()) != NULL ) {
		++mit;
		if( obj -> getID() == id )
			return obj;
	}
	return NULL;
}


}  // namespace Umbrello

