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

#include "umlcanvasobject.h"

/**
 * This class contains the non-graphical information required for a UML UseCase.
 * This class inherits from @ref UMLCanvasObject which contains most of the information.
 * The @ref UMLDoc class creates instances of this type.  All UseCases will need a unique
 * id.  This will be given by the @ref UMLDoc class.  If you don't leave it up to the @ref UMLDoc
 * class then call the method @ref UMLDoc::getUniqueID to get a unique id.
 *
 * @short Information for a non-graphical UML UseCase.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLUseCase : public UMLCanvasObject {
public:
	/**
	 *	Creates a UseCase object
	 *
	 *	@param	parent		The parent of the object.
	 *	@param name		The name of the object.
	 *	@param	id				The id of the object.
	 */
	UMLUseCase(QObject* parent, QString Name, int id);

	/**
	 *	Creates a UseCase object
	 *
	 *	@param	parent		The parent of the object.
	 */
	UMLUseCase(QObject* parent);

	/**
	 *	Standard deconstructor
	 */
	~UMLUseCase();

	/**
	 *	Initializes key variables of the class.
	 */
	virtual void init();

	/**
	 * Creates the <UML:UseCase> element
	 */
	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads the <UML:UseCase> element
	 */
	bool loadFromXMI( QDomElement & element );
};

#endif
