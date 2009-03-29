/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ASSOCIATION_H
#define ASSOCIATION_H

#include <qdom.h>

#include "umlnamespace.h"
#include "umlobject.h"

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

    UMLAssociation(Uml::Association_Type type, UMLObject *roleA, UMLObject *roleB);
    UMLAssociation(Uml::Association_Type type = Uml::at_Unknown);

    virtual ~UMLAssociation();

    bool operator==(const UMLAssociation &rhs);

    QString toString() const;

    static QString toString(Uml::Association_Type atype);
    static QString toString(Uml::Changeability_Type type);

    static bool assocTypeHasUMLRepresentation(Uml::Association_Type atype);

    UMLRole * getUMLRole(Uml::Role_Type role) const;
    Uml::IDType getObjectId(Uml::Role_Type role) const;
    Uml::IDType getRoleId(Uml::Role_Type role) const;

    void setAssocType(Uml::Association_Type assocType);
    Uml::Association_Type getAssocType() const;

    void setObject(UMLObject *obj, Uml::Role_Type role);
    UMLObject* getObject(Uml::Role_Type role) const;

    void setVisibility(Uml::Visibility value, Uml::Role_Type role);
    Uml::Visibility getVisibility(Uml::Role_Type role) const;

    void setChangeability(Uml::Changeability_Type value, Uml::Role_Type role);
    Uml::Changeability_Type getChangeability(Uml::Role_Type role) const;

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

    void init(Uml::Association_Type type, UMLObject *roleAObj, UMLObject *roleBObj);

    /* If the type Uml::Association_Type is changed then also the following
       must be changed accordingly:
       atypeFirst, atypeLast, assocTypeStr[], toAssocType(), toString().
       The ordering within assocTypeStr must match the enumeration
       order of Uml::Association_Type.
     */
    static const Uml::Association_Type atypeFirst;
    static const Uml::Association_Type atypeLast;
    static const unsigned int nAssocTypes;
    static const QString assocTypeStr[];

    UMLRole *              m_pRole[2];
    Uml::Association_Type  m_AssocType;
    QString                m_Name;
    bool                   m_bOldLoadMode;

private:

    bool isRealization(UMLObject* objA, UMLObject* objB) const;
};

#endif
