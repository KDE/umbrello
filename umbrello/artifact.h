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

#include "umlcanvasobject.h"

class IDChangeLog;

/**
 * This class contains the non-graphical information required for a UML
 * Artifact.
 * This class inherits from @ref UMLCanvasObject which contains most of the
 * information.
 * The @ref UMLDoc class creates instances of this type.  All Artifacts will
 * need a unique id.  This will be given by the @ref UMLDoc class.  If you
 * don't leave it up to the @ref UMLDoc class then call the method @ref
 * UMLDoc::getUniqueID to get a unique id.
 *
 * @short Non-graphical information for a Artifact.
 * @author Jonathan Riddell
 * @see	UMLCanvasObject
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLArtifact : public UMLCanvasObject {
	Q_OBJECT
public:

	/**
	 * Artifacts can be drawn using one of several icons.
	 */
	enum Draw_Type {
		defaultDraw,
		file,
		library,
		table
	};

	/**
	 * Sets up a Artifact.
	 *
	 * @param name		The name of the Concept.
	 * @param id		The unique id of the Concept.
	 */
	UMLArtifact(const QString & Name = "", int id = -1);

	/**
	 * Standard deconstructor.
	 */
	virtual ~UMLArtifact();

	/**
	 * Initializes key variables of the class.
	 */
	virtual void init();

	/**
	 * Make a clone of this object.
	 */
	virtual UMLObject* clone() const;

	/**
	 * Creates the UML:Artifact element including its operations,
	 * attributes and templates
	 */
	void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * sets m_drawAsType for which method to draw the artifact as
	 */
	void setDrawAsType(Draw_Type type);

	/**
	 * returns the value of m_drawAsType
	 */
	Draw_Type getDrawAsType();

protected:
	/**
	 * Loads the UML:Artifact element including its operations,
	 * attributes and templates
	 */
	bool load( QDomElement & element );

private:
	/**
	 * Artifacts can be drawn as one of several different icons,
	 * this value choosing how to draw them.
	 */
	Draw_Type m_drawAsType;
};

#endif
