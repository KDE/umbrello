/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// own header
#include "model_utils.h"

// app includes
#include "umlobject.h"
#include "package.h"
#include "classifier.h"

namespace Umbrello {

bool isCloneable(Uml::Widget_Type type) {
	switch (type) {
		case Uml::wt_Actor:
		case Uml::wt_UseCase:
		case Uml::wt_Class:
		case Uml::wt_Interface:
		case Uml::wt_Enum:
		case Uml::wt_Datatype:
		case Uml::wt_Package:
		case Uml::wt_Component:
		case Uml::wt_Node:
		case Uml::wt_Artifact:
			return true;
		default:
			return false;
	}
}

UMLObject * findObjectInList(int id, UMLObjectList inList) {
	for (UMLObjectListIt oit(inList); oit.current(); ++oit) {
		UMLObject *obj = oit.current();
		if (obj->getID() == id)
			return obj;
		UMLObject *o;
		Uml::Object_Type t = obj->getBaseType();
		switch (t) {
			case Uml::ot_Package:
				o = ((UMLPackage*)obj)->findObject(id);
				if (o)
					return o;
				break;
			case Uml::ot_Interface:
			case Uml::ot_Class:
			case Uml::ot_Enum:
				o = ((UMLClassifier*)obj)->findChildObject(id);
				if (o == NULL &&
				    (t == Uml::ot_Interface || t == Uml::ot_Class))
					o = ((UMLPackage*)obj)->findObject(id);
				if (o)
					return o;
				break;
			default:
				break;
		}
	}
	return NULL;
}


}  // namespace Umbrello

