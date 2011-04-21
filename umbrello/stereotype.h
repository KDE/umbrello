/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
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
 * Each stereotype object is reference counted, i.e. client code
 * manages it such that it comes into existence as soon as there is
 * at least one user, and ceases existing when the number of users
 * drops to 0.
 * m_refCount reflects the number of users. It is externally managed,
 * i.e. client code must take care to call incrRefCount() and
 * decrRefCount() as appropriate.
 *
 * @short Sets up stereotype information.
 * @author Jonathan Riddell
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLStereotype : public UMLObject
{
public:
    explicit UMLStereotype(const QString &name, Uml::IDType id = Uml::id_None);
    UMLStereotype();

    virtual ~UMLStereotype();

    bool operator==(const UMLStereotype &rhs) const;

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    void incrRefCount();
    void decrRefCount();

    int refCount() const;

    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

    bool showPropertiesDialog(QWidget* parent);

protected:
    int m_refCount;

};

#endif
