/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MODEL_UTILS_H
#define MODEL_UTILS_H

#include "umlnamespace.h"
#include "umlobjectlist.h"

/**
 * General purpose model utilities.
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
namespace Umbrello {

	/**
	 * Determines whether the given widget type is cloneable.
	 *
	 * @param type		The input Widget_Type.
	 * @return	True if the given type is cloneable.
	 */
	bool isCloneable(Uml::Widget_Type type);

	/**
	 * Seek the given id in the given list of objects.
	 * Each list element may itself contain other objects
	 * and the search is done recursively.
	 *
	 * @param id		The unique ID to seek.
	 * @param inList	The UMLObjectList in which to search.
	 * @return	Pointer to the UMLObject that matches the ID
	 *		(NULL if none matches.)
	 */
	UMLObject * findObjectInList(Uml::IDType id, UMLObjectList inList);

	/**
	 * Find the UML object of the given type and name in the passed-in list.
	 *
	 * @param inList	List in which to seek the object.
	 * @param name		Name of the object to find.
	 * @param type		Object_Type of the object to find (optional.)
	 *			When the given type is ot_UMLObject the type is
	 *			disregarded, i.e. the given name is the only
	 *			search criterion.
	 * @param currentObj	Object relative to which to search (optional.)
	 *			If given then the enclosing scope(s) of this
	 *			object are searched before the global scope.
	 * @return	Pointer to the UMLObject found, or NULL if not found.
	 */
	UMLObject* findUMLObject( UMLObjectList inList, QString name,
				  Uml::Object_Type type = Uml::ot_UMLObject,
				  UMLObject *currentObj = NULL);

	/**
	 * Return true if the given tag is a one of the common XMI
	 * attributes, such as:
	 * "name" | "visibility" | "isRoot" | "isLeaf" | "isAbstract" |
	 * "isActive" | "ownerScope"
	 */
	bool isCommonXMIAttribute(QString tag);

}

#endif
