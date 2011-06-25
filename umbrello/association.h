/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ASSOCIATION_H
#define ASSOCIATION_H

#include "basictypes.h"
#include "umlobject.h"

#include <QtXml/QDomElement>
#include <QtXml/QDomDocument>

class UMLRole;

/**
 * This class contains the non-graphic representation of an association.
 * An association can be a generalization, realization, simple association,
 * directed association, aggregation, or composition.
 *
 * @short Sets up association information.
 * @author Oliver Kellogg <okellogg@users.sourceforge.net>
 * @see UMLObject
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLAssociation : public UMLObject
{
    Q_OBJECT
    friend class AssociationWidget;

public:
    UMLAssociation(Uml::AssociationType type, UMLObject *roleA, UMLObject *roleB);
    UMLAssociation(Uml::AssociationType type = Uml::AssociationType::Unknown);

    virtual ~UMLAssociation();

    bool operator==(const UMLAssociation &rhs) const;

    QString toString() const;

    UMLRole * getUMLRole(Uml::Role_Type role) const;
    Uml::IDType getObjectId(Uml::Role_Type role) const;
    Uml::IDType getRoleId(Uml::Role_Type role) const;

    void setAssociationType(Uml::AssociationType assocType);
    Uml::AssociationType getAssocType() const;

    void setObject(UMLObject *obj, Uml::Role_Type role);
    UMLObject* getObject(Uml::Role_Type role) const;

    void setVisibility(Uml::Visibility value, Uml::Role_Type role);
    Uml::Visibility getVisibility(Uml::Role_Type role) const;

    void setChangeability(Uml::Changeability value, Uml::Role_Type role);
    Uml::Changeability changeability(Uml::Role_Type role) const;

    void setMulti(const QString &multi, Uml::Role_Type role);
    QString getMulti(Uml::Role_Type role) const;

    void setRoleName(const QString &roleName, Uml::Role_Type role);
    QString getRoleName(Uml::Role_Type role) const;

    void setRoleDoc(const QString &doc, Uml::Role_Type role);
    QString getRoleDoc(Uml::Role_Type role) const;

    void setOldLoadMode(bool value = true);
    bool getOldLoadMode() const;

    virtual UMLObject* clone() const { return NULL; }

    virtual bool resolveRef();

    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:

    bool load(QDomElement& element);

    // keep track of number of parent widgets
    int nrof_parent_widgets;

    void init(Uml::AssociationType type, UMLObject *roleAObj, UMLObject *roleBObj);

    UMLRole *              m_pRole[2];
    Uml::AssociationType   m_AssocType;
    QString                m_Name;
    bool                   m_bOldLoadMode;

private:

    bool isRealization(UMLObject* objA, UMLObject* objB) const;
};

#endif
