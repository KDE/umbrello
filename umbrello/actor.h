/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ACTOR_H
#define ACTOR_H

#include "umlobject.h"

/**
 *	This class contains the non-graphical information required for a UML Actor.
 *	This class inherits from @ref UMLObject which contains most of the information.
 *	The @ref UMLDoc class creates instances of this type.  All Actors will need a unique
 *	id.  This will be given by the @ref UMLDoc class.  If you don't leave it up to the @ref UMLDoc
 *	class then call the method @ref UMLDoc::getUniqueID to get a unique id.
 *
 *	@short Information for a non-graphical UML Actor.
 *	@author Paul Hensgen <phensgen@techie.com>
 *	@version	1.0
 *	@see	UMLObject
 *
 *	@see UMLDoc
 */
class UMLActor : public UMLObject {
public:
	/**
	 * Constructs an Actor.
	 * @param parent		The parent of this Actor.
	 * @param name		The name of the Actor.
	 * @param id				The unique id to assign to this Actor.
	 */
	UMLActor(QObject * parent, QString Name, int id);

	/**
	 * Constructs an Actor.
	 * @param parent	The parent of this Actor.
	 */
	UMLActor(QObject * parent);

	/**
	 *	Standard deconstructor.
	 */
	~UMLActor();

	/**
	 *	Allows the class to be loaded and saved.
	 *	@param	s	The data stream to save/load to file to/from.
	 *	@param	archive If true will save the classes information, else will
	 *			load the information.
	 *	@param	fileversion the version of the serialize format
	 *
	 *	@return		The success of the operation.
	 */
	virtual bool serialize(QDataStream * s, bool archive, int fileversion);

	/**
	 * Returns the amount of bytes needed to serialize an instance object to the clipboard
	 */
	virtual long getClipSizeOf();

	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & element );
};

#endif
