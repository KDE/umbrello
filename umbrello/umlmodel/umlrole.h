/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

    UMLRole(UMLAssociation * parent, UMLObject * parentUMLObject, Uml::RoleType::Enum role);
    virtual ~UMLRole();

    bool operator==(const UMLRole & rhs) const;

    QString toString() const;

    void setObject(UMLObject *obj);
    UMLObject* object() const;

    void setChangeability(Uml::Changeability::Enum value);
    Uml::Changeability::Enum changeability() const;

    void setMultiplicity(const QString &multi);
    QString multiplicity() const;

    UMLAssociation * parentAssociation() const;

    Uml::RoleType::Enum role() const;

    UMLObject* clone() const { return 0; }

    void saveToXMI(QXmlStreamWriter& writer);

    virtual bool showPropertiesDialog(QWidget *parent = nullptr);

protected:

    bool load1(QDomElement& element);

private:

    UMLAssociation *           m_pAssoc;
    Uml::RoleType::Enum        m_role;
    QString                    m_Multi;
    Uml::Changeability::Enum   m_Changeability;
};

#endif
