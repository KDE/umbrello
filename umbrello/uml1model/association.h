/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef ASSOCIATION_H
#define ASSOCIATION_H

#include "basictypes.h"
#include "umlobject.h"

#include <QDomDocument>
#include <QDomElement>

class UMLRole;

/**
 * This class contains the non-graphic representation of an association.
 * An association can be a generalization, realization, simple association,
 * directed association, aggregation, or composition.
 *
 * @short Sets up association information.
 * @author Oliver Kellogg <okellogg@users.sourceforge.net>
 * @see UMLObject
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class UMLAssociation : public UMLObject
{
    Q_OBJECT
    friend class AssociationWidget;

public:
    UMLAssociation(Uml::AssociationType::Enum type, UMLObject *roleA, UMLObject *roleB);
    explicit UMLAssociation(Uml::AssociationType::Enum type = Uml::AssociationType::Unknown);

    virtual ~UMLAssociation();

    bool operator==(const UMLAssociation &rhs) const;

    QString toString() const;

    UMLRole * getUMLRole(Uml::RoleType::Enum role) const;
    Uml::ID::Type getObjectId(Uml::RoleType::Enum role) const;
    Uml::ID::Type getRoleId(Uml::RoleType::Enum role) const;

    void setAssociationType(Uml::AssociationType::Enum assocType);
    Uml::AssociationType::Enum getAssocType() const;

    void setObject(UMLObject *obj, Uml::RoleType::Enum role);
    UMLObject* getObject(Uml::RoleType::Enum role) const;

    void setVisibility(Uml::Visibility::Enum value, Uml::RoleType::Enum role);
    Uml::Visibility::Enum visibility(Uml::RoleType::Enum role) const;

    void setChangeability(Uml::Changeability::Enum value, Uml::RoleType::Enum role);
    Uml::Changeability::Enum changeability(Uml::RoleType::Enum role) const;

    void setMultiplicity(const QString &multi, Uml::RoleType::Enum role);
    QString getMultiplicity(Uml::RoleType::Enum role) const;

    void setRoleName(const QString &roleName, Uml::RoleType::Enum role);
    QString getRoleName(Uml::RoleType::Enum role) const;

    void setRoleDoc(const QString &doc, Uml::RoleType::Enum role);
    QString getRoleDoc(Uml::RoleType::Enum role) const;

    void setOldLoadMode(bool value = true);
    bool getOldLoadMode() const;

    virtual UMLObject* clone() const { return 0; }

    virtual bool resolveRef();

    void saveToXMI1(QDomDocument& qDoc, QDomElement& qElement);

    virtual bool showPropertiesDialog(QWidget *parent = 0);

protected:

    bool load1(QDomElement& element);

    // keep track of number of parent widgets
    int nrof_parent_widgets;

    void init(Uml::AssociationType::Enum type, UMLObject *roleAObj, UMLObject *roleBObj);

    UMLRole *                    m_pRole[2];
    Uml::AssociationType::Enum   m_AssocType;
    QString                      m_Name;
    bool                         m_bOldLoadMode;

private:

    bool isRealization(UMLObject* objA, UMLObject* objB) const;
};

#endif
