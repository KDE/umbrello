/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ENTITYCONSTRAINT_H
#define ENTITYCONSTRAINT_H

#include "basictypes.h"
#include "classifierlistitem.h"
#include "umlclassifierlist.h"

/**
 * This class is used to set up information for a entity constraint.
 *
 * @short Sets up entity constraint information.
 * @author Sharan Rao
 * @see UMLObject UMLClassifierListItem
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLEntityConstraint : public UMLClassifierListItem
{
     Q_OBJECT

public:
    UMLEntityConstraint(UMLObject *parent, const QString& name,
                        Uml::ID::Type id = Uml::ID::None);

    explicit UMLEntityConstraint(UMLObject *parent);

    bool operator==(const UMLEntityConstraint &rhs) const;

    virtual ~UMLEntityConstraint();

    virtual void copyInto(UMLObject *lhs) const;

    /**
     * Make a clone of the UMLEntityConstraint.
     */
    virtual UMLObject* clone() const = 0;

};

#endif
