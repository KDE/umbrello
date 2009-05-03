/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UNIQUECONSTRAINT_H
#define UNIQUECONSTRAINT_H

#include "entityconstraint.h"
#include "classifierlistitem.h"
#include "umlnamespace.h"
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
class UMLUniqueConstraint : public UMLEntityConstraint {
public:

    /**
     * Sets up a constraint.
     *
     * @param parent    The parent of this UMLUniqueConstraint.
     * @param name      The name of this UMLUniqueConstraint.
     * @param id        The unique id given to this UMLUniqueConstraint.
     */
    UMLUniqueConstraint(UMLObject *parent, const QString& name,
                        Uml::IDType id = Uml::id_None );

    /**
     * Sets up a constraint.
     *
     * @param parent    The parent of this UMLUniqueConstraint.
     */
    UMLUniqueConstraint(UMLObject *parent);

    /**
     * Overloaded '==' operator
     */
    bool operator==(const UMLUniqueConstraint &rhs);

    /**
     * destructor.
     */
    virtual ~UMLUniqueConstraint();

    /**
     * Copy the internal presentation of this object into the UMLUniqueConstraint
     * object.
     */
    virtual void copyInto(UMLObject *lhs) const;

    /**
     * Make a clone of the UMLUniqueConstraint.
     */
    virtual UMLObject* clone() const;

    /**
     * Returns a string representation of the UMLUniqueConstraint.
     *
     * @param sig               If true will show the attribute type and
     *                  initial value.
     * @return  Returns a string representation of the UMLAttribute.
     */
    QString toString(Uml::Signature_Type sig = Uml::st_NoSig);

    QString getFullyQualifiedName(const QString& separator = QString(),
                                  bool includeRoot = false) const;

    /**
     * Creates the <UML:UniqueConstraint> XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Display the properties configuration dialog for the attribute.
     */
    virtual bool showPropertiesDialog(QWidget* parent);

    /**
     * Check if a entity attribute is present in m_entityAttributeList
     *
     * @param attr The Entity Attribute to check for existence in list
     * @return true if it exists in the list, else false
     */
    bool hasEntityAttribute(UMLEntityAttribute* attr);

    /**
     * Adds a UMLEntityAttribute to the list.
     * The UMLEntityAttribute should already exist and should
     * belong to the parent UMLEntity.
     *
     * @param attr The UMLEntityAttribute to add
     * @return false if it failed to add , else true
     */
    bool addEntityAttribute(UMLEntityAttribute* attr);

    /**
     * Removes a UMLEntityAttribute from the list
     *
     * @param attr The UMLEntityAttribute to remove from list
     * @return false if it failed to remove the attribute from the list
     */
    bool removeEntityAttribute(UMLEntityAttribute* attr);


    /**
     * Get the Entity Attributes List
     */
    UMLEntityAttributeList getEntityAttributeList() const {
        return m_EntityAttributeList;
    }

    /**
     * Clear the list of attributes contained in this UniqueConstraint
     */
    void clearAttributeList();

protected:
    /**
     * Loads the <UML:UniqueConstraint> XMI element.
     */
    bool load( QDomElement & element );

private:

    void init();

    /**
     * The list of entity attributes that together make up the unique constraint
     */
    UMLEntityAttributeList m_EntityAttributeList;


};

#endif
