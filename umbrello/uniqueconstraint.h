/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UNIQUECONSTRAINT_H
#define UNIQUECONSTRAINT_H

#include "basictypes.h"
#include "classifierlistitem.h"
#include "entityconstraint.h"
#include "umlclassifierlist.h"
#include "umlentityattributelist.h"

/**
 * This class is used to set up information for a unique entity constraint.
 *
 * @short Sets up Unique entity constraint information.
 * @author Sharan Rao
 * @see UMLObject UMLClassifierListItem UMLEntityConstraint
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLUniqueConstraint : public UMLEntityConstraint
{
public:

    UMLUniqueConstraint(UMLObject *parent, const QString& name,
                        Uml::IDType id = Uml::id_None );
    UMLUniqueConstraint(UMLObject *parent);
    virtual ~UMLUniqueConstraint();

    bool operator==(const UMLUniqueConstraint &rhs) const;

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    QString toString(Uml::SignatureType sig = Uml::SignatureType::NoSig);

    QString getFullyQualifiedName(const QString& separator = QString(),
                                  bool includeRoot = false) const;

    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    virtual bool showPropertiesDialog(QWidget* parent);

    bool hasEntityAttribute(UMLEntityAttribute* attr);

    bool addEntityAttribute(UMLEntityAttribute* attr);

    bool removeEntityAttribute(UMLEntityAttribute* attr);

    UMLEntityAttributeList getEntityAttributeList() const;

    void clearAttributeList();

protected:

    bool load( QDomElement & element );

private:

    void init();

    /**
     * The list of entity attributes that together make up the unique constraint.
     */
    UMLEntityAttributeList m_EntityAttributeList;

};

#endif
