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

#ifndef NODE_H
#define NODE_H

#include "umlcanvasobject.h"


/**
 * This class contains the non-graphical information required for a UML Node.
 * This class inherits from @ref UMLCanvasObject which contains most of the
 * information.
 *
 * @short Non-graphical information for a Node.
 * @author Jonathan Riddell
 * @see UMLCanvasObject
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLNode : public UMLCanvasObject {
    Q_OBJECT
public:
    /**
     * Sets up a Node.
     *
     * @param name              The name of the Concept.
     * @param id                The unique id of the Concept.
     */
    explicit UMLNode(const QString & name = "", Uml::IDType id = Uml::id_None);

    /**
     * Empty deconstructor.
     */
    virtual ~UMLNode();

    /**
     * Initializes key variables of the class.
     */
    virtual void init();

    /**
     * Make a clone of this object.
     */
    virtual UMLObject* clone() const;

    /**
     * Creates the <UML:Node> XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

protected:
    /**
     * Loads the <UML:Node> XMI element (empty.)
     */
    bool load( QDomElement & element );

};

#endif
