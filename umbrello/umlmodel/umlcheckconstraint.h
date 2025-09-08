/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CHECKCONSTRAINT_H
#define CHECKCONSTRAINT_H

#include "basictypes.h"
#include "umlentityconstraint.h"

/**
 * This class is used to set up information for a unique entity constraint.
 *
 * @short Sets up Check Constraint information for UMLEntities.
 * @author Sharan Rao
 * @see UMLObject UMLClassifierListItem UMLEntityConstraint
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLCheckConstraint : public UMLEntityConstraint
{
     Q_OBJECT
public:

    UMLCheckConstraint(UMLObject *parent, const QString& name,
                       Uml::ID::Type id = Uml::ID::None);
    explicit UMLCheckConstraint(UMLObject *parent);

    bool operator==(const UMLCheckConstraint &rhs) const;

    virtual ~UMLCheckConstraint();

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    QString toString(Uml::SignatureType::Enum sig = Uml::SignatureType::NoSig,
                     bool withStereotype=false) const;

    QString getFullyQualifiedName(const QString& separator = QString(),
                                  bool includeRoot = false) const;

    void saveToXMI(QXmlStreamWriter& writer);

    virtual bool showPropertiesDialog(QWidget *parent = nullptr);

    QString getCheckCondition() const {
        return m_CheckCondition;
    }

    void setCheckCondition(const QString& condition) {
        m_CheckCondition = condition.trimmed();
    }

protected:

    bool load1(QDomElement & element);

private:

    void init();

    QString m_CheckCondition;   ///< the check condition

};

#endif
