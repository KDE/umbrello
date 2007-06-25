/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ACTOR_H
#define ACTOR_H

#include "umlcanvasobject.h"

/**
 * This class contains the non-graphical information required for a UML Actor.
 * This class inherits from @ref UMLCanvasObject which contains most of the
 * information.
 * The @ref UMLDoc class creates instances of this type.
 *
 * @short Information for a non-graphical UML Actor.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see UMLCanvasObject
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLActor : public UMLCanvasObject {
    Q_OBJECT
public:
    /**
     * Constructs an Actor.
     *
     * @param name              The name of the Actor.
     * @param id                The unique id to assign to this Actor.
     */
    explicit UMLActor(const QString & name = "", Uml::IDType id = Uml::id_None);

    /**
     * Standard deconstructor.
     */
    ~UMLActor();

    /**
     * Initializes key variables of the class.
     */
    virtual void init();

    /**
     * Make a clone of this object.
     */
    virtual UMLObject* clone() const;

    /**
     * Creates the <UML:Actor> XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

protected:
    /**
     * Loads the <UML:Actor> XMI element (empty.)
     */
    bool load( QDomElement & element );
};

#endif
