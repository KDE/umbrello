/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLROLE_H
#define UMLROLE_H

#include "umlobject.h"

class UMLAssociation;

/**
 * This class contains the non-graphic representation of an association role.
 *
 * @author Brian Thomas <brian.thomas@gsfc.nasa.gov>
 * @see UMLObject
 */
class UMLRole : public UMLObject
{
    Q_OBJECT
public:

    UMLRole(UMLAssociation * parent, UMLObject * parentUMLObject, Uml::Role_Type role);
    virtual ~UMLRole();

    bool operator==(const UMLRole & rhs) const;

    void setObject(UMLObject *obj);
    UMLObject* object() const;

    void setChangeability(Uml::Changeability value);
    Uml::Changeability changeability() const;

    void setMultiplicity(const QString &multi);
    QString multiplicity() const;

    UMLAssociation * parentAssociation() const;

    Uml::Role_Type role() const;

    UMLObject* clone() const { return NULL; }

    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:

    bool load(QDomElement& element);

private:

    UMLAssociation *        m_pAssoc;
    Uml::Role_Type          m_role;
    QString                 m_Multi;
    Uml::Changeability      m_Changeability;
};

#endif
