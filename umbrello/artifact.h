/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ARTIFACT_H
#define ARTIFACT_H

#include <qptrlist.h>
#include "umlcanvasobject.h"

class IDChangeLog;
class UMLAssociation;

//FIXMEnow doc
enum Artifact_draw_type {
	defaultDraw,
	file,
	library,
	table
};


/**
 *	This class contains the non-graphical information required for a UML Artifact.
 *	This class inherits from @ref UMLCanvasObject which contains most of the information.
 *	The @ref UMLDoc class creates instances of this type.  All Artifacts will need a unique
 *	id.  This will be given by the @ref UMLDoc class.  If you don't leave it up to the @ref UMLDoc
 *	class then call the method @ref UMLDoc::getUniqueID to get a unique id.
 *
 *	@short	Non-graphical information for a Artifact.
 *	@author Jonathan Riddell
 *	@see	UMLCanvasObject
 */
class UMLArtifact : public UMLCanvasObject {
public:
	/**
	 *	Sets up a Artifact.
	 *
	 *	@param	parent	The parent to this Concept.
	 *	@param	name	The name of the Concept.
	 *	@param	id	The unique id of the Concept.
	 */
	UMLArtifact(QObject* parent, QString Name, int id);

	/**
	 *	Sets up a Artifact.
	 *
	 *	@param	parent		The parent to this Concept.
	 */
	UMLArtifact(QObject* parent);

	/**
	 *	Empty deconstructor.
	 */
	virtual ~UMLArtifact();

	/**
	 *	Initializes key variables of the class.
	 */
	virtual void init();

	/**
	 * Creates the UML:Artifact element including it's operations,
	 * attributes and templates
	 */
	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads the UML:Artifact element including it's operations,
	 * attributes and templates
	 */
	bool loadFromXMI( QDomElement & element );

	//FIXMEnow doc
	void setDrawAsType(Artifact_draw_type type);

	Artifact_draw_type getDrawAsType();

private:

	Artifact_draw_type m_drawAsType;
};

#endif
