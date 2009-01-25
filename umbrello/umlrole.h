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
class UMLRole : public UMLObject {
    Q_OBJECT
public:

    /**
     * Sets up an association.
     *
     * @param parent    The parent (association) of this UMLRole.
     * @param parentUMLObject The Parent UML Object of this UMLRole
     * @param role  The Uml::Role_Type of this UMLRole
     */
    UMLRole (UMLAssociation * parent, UMLObject * parentUMLObject, Uml::Role_Type role);

    /**
     * Overloaded '==' operator
     */
    bool operator==(const UMLRole & rhs);

    /**
     * Standard deconstructor.
     */
    virtual ~UMLRole();

    UMLObject* getObject();

    Uml::Changeability_Type getChangeability() const;

    QString getMultiplicity() const;

    void setObject(UMLObject *obj);

    void setChangeability (Uml::Changeability_Type value);

    void setMultiplicity ( const QString &multi );

    UMLAssociation * getParentAssociation ();

    Uml::Role_Type getRole();

    UMLObject* clone() const { return NULL; }

    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:

    bool load(QDomElement& element);

private:

    void init (UMLAssociation * parent, UMLObject * parentObj, Uml::Role_Type r);

    UMLAssociation * m_pAssoc;
    Uml::Role_Type m_role;
    QString m_Multi;
    Uml::Changeability_Type m_Changeability;
};

#endif
