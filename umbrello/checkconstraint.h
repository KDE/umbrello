/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CHECKCONSTRAINT_H
#define CHECKCONSTRAINT_H

#include "basictypes.h"
#include "entityconstraint.h"

/**
 * This class is used to set up information for a unique entity constraint.
 *
 * @short Sets up Check Constraint information for UMLEntities.
 * @author Sharan Rao
 * @see UMLObject UMLClassifierListItem UMLEntityConstraint
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLCheckConstraint : public UMLEntityConstraint
{
     Q_OBJECT
public:

    UMLCheckConstraint(UMLObject *parent, const QString& name,
                       Uml::IDType id = Uml::id_None );
    UMLCheckConstraint(UMLObject *parent);

    bool operator==(const UMLCheckConstraint &rhs) const;

    virtual ~UMLCheckConstraint();

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    QString toString(Uml::SignatureType sig = Uml::SignatureType::NoSig);

    QString getFullyQualifiedName(const QString& separator = QString(),
                                  bool includeRoot = false) const;

    void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);

    virtual bool showPropertiesDialog(QWidget* parent);

    QString getCheckCondition() const {
        return m_CheckCondition;
    }

    void setCheckCondition(const QString& condition) {
        m_CheckCondition = condition.trimmed();
    }

protected:

    bool load(QDomElement & element);

private:

    void init();

    QString m_CheckCondition;   ///< the check condition

};

#endif
