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

#ifndef STEREOTYPE_H
#define STEREOTYPE_H

#include "umlobject.h"

/**
 * This class is used to set up information for a stereotype.
 * Stereotypes are used essentially as properties of
 * attributes and operations etc.
 *
 * @short Sets up stereotype information.
 * @author Jonathan Riddell
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLStereotype : public UMLObject {
public:
    /**
     * Sets up a stereotype.
     *
     * @param name              The name of this UMLStereotype.
     * @param id                The unique id given to this UMLStereotype.
     */
    explicit UMLStereotype(const QString &name, Uml::IDType id = Uml::id_None);

    /**
     * Sets up a stereotype.
     */
    UMLStereotype();

    /**
     * Overloaded '==' operator
     */
    bool operator==(UMLStereotype &rhs);

    /**
     * destructor
     */
    virtual ~UMLStereotype();

    /**
     * Copy the internal presentation of this object into the new
     * object.
     */
    virtual void copyInto(UMLStereotype *rhs) const;

    /**
     * Make a clone of this object.
     */
    virtual UMLObject* clone() const;

    /**
     * Increments the reference count for this stereotype.
     */
    void incrRefCount();

    /**
     * Decrements the reference count for this stereotype.
     */
    void decrRefCount();

    /**
     * Returns the reference count for this stereotype.
     */
    int refCount() const;

    /**
     * Saves to the <UML:StereoType> XMI element.
     */
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

    /**
     * Display the properties configuration dialog for the stereotype
     * (just a line edit).
     */
    bool showPropertiesDialog(QWidget* parent);

protected:
    /**
     * Each stereotype object is reference counted, i.e. client code
     * manages it such that it comes into existence as soon as there is
     * at least one user, and ceases existing when the number of users
     * drops to 0.
     * m_refCount reflects the number of users.  It is externally managed,
     * i.e. client code must take care to call incrRefCount() and
     * decrRefCount() as appropriate.
     */
    int m_refCount;
};

#endif
