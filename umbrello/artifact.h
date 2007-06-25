/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ARTIFACT_H
#define ARTIFACT_H

#include "umlcanvasobject.h"


/**
 * This class contains the non-graphical information required for a UML
 * Artifact.
 * This class inherits from @ref UMLCanvasObject which contains most of the
 * information.
 *
 * @short Non-graphical information for a Artifact.
 * @author Jonathan Riddell
 * @see UMLCanvasObject
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
     * @param Name              The name of the Concept.
     * @param id                The unique id of the Concept.
     */
    explicit UMLArtifact(const QString & Name = "", Uml::IDType id = Uml::id_None);

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
