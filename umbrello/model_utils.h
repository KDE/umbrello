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
	 * Seek the given id in the object hierarchy rooted at rootObj.
	 *
	 * @param id		The unique ID to seek.
	 * @param inList	The UMLObjectList in wihch to search.
	 * @return	Pointer to the UMLObject that matches the ID
	 *		(NULL if none matches.)
	 */
	UMLObject * findObjectInList(int id, UMLObjectList inList);
}

#endif
