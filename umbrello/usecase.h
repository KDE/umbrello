/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef USECASE_H
#define USECASE_H

#include "umlobject.h"


/**
 *	This class contains the non-graphical information required for a UML UseCase.
 * This class inherits from @ref UMLObject which contains most of the information.
 * The @ref UMLDoc class creates instances of this type.  All UseCases will need a unique
 * id.  This will be given by the @ref UMLDoc class.  If you don't leave it up to the @ref UMLDoc
 * class then call the method @ref UMLDoc::getUniqueID to get a unique id.
 *
 * @short Information for a non-graphical UML UseCase.
 *	@author Paul Hensgen <phensgen@techie.com>
 *	@version 1.0
 */

class UMLUseCase : public UMLObject {
public:
	/**
	 *	Creates a UseCase object
	 *
	 *	@param	parent		The parent of the object.
	 *	@param name		The name of the object.
	 *	@param	id				The id of the object.
	 */
	UMLUseCase(QObject * parent, QString Name, int id);

	/**
	 *	Creates a UseCase object
	 *
	 *	@param	parent		The parent of the object.
	 */
	UMLUseCase(QObject * parent);

	/**
	 *	Standard deconstructor
	 */
	~UMLUseCase();

	/**
	 * Use to save or load this classes information
	 *
	 *	@param	s	Pointer to the datastream (file) to save/load from.
	 *	@param	archive	If true will save the classes information, else will
	 * load the information.
	 *
	 *	@return	Returns the status of the operation.
	 */
	virtual bool serialize(QDataStream * s, bool archive, int serialize);

	/**
	 * Returns the amount of bytes needed to serialize an instance object to the clipboard
	 */
	virtual long getClipSizeOf();

	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & element );
};

#endif
