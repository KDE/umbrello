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

#ifndef ENTITYCONSTRAINT_H
#define ENTITYCONSTRAINT_H

#include "classifierlistitem.h"
#include "umlnamespace.h"
#include "umlclassifierlist.h"

/**
 * This class is used to set up information for a entity constraint.
 *
 * @short Sets up entity constraint information.
 * @author Sharan Rao
 * @see UMLObject UMLClassifierListItem
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLEntityConstraint : public UMLClassifierListItem {

     Q_OBJECT

public:
    /**
     * Sets up a constraint.
     *
     * @param parent    The parent of this UMLEntityConstraint.
     * @param name      The name of this UMLEntityConstraint.
     * @param id        The unique id given to this UMLEntityConstraint.
     * @param type      The type of this UMLEntityConstraint.
     */
    UMLEntityConstraint(const UMLObject *parent, const QString& name,
			Uml::IDType id = Uml::id_None );

    /**
     * Sets up a constraint.
     *
     * @param parent    The parent of this UMLEntityConstraint.
     */
    UMLEntityConstraint(const UMLObject *parent);

    /**
     * Overloaded '==' operator
     */
    bool operator==( UMLEntityConstraint &rhs);

    /**
     * destructor.
     */
    virtual ~UMLEntityConstraint();

    /**
     * Copy the internal presentation of this object into the UMLEntityConstraint
     * object.
     */
    virtual void copyInto(UMLEntityConstraint *rhs) const;

    /**
     * Make a clone of the UMLEntityConstraint.
     */
    virtual UMLObject* clone() const;

    /**
     * Returns a string representation of the UMLEntityConstraint.
     *
     * @param sig               If true will show the attribute type and
     *                  initial value.
     * @return  Returns a string representation of the UMLAttribute.
     */
    QString toString(Uml::Signature_Type sig = Uml::st_NoSig);

    /**
     * Reimplement method from UMLObject.
     */
    QString getFullyQualifiedName(QString separator = QString(),
                                  bool includeRoot = false) const;

    /**
     * Creates the <UML:EntityConstraint> XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Display the properties configuration dialog for the attribute.
     */
    virtual bool showPropertiesDialog(QWidget* parent);


protected:
    /**
     * Loads the <UML:EntityConstraint> XMI element.
     */
    bool load( QDomElement & element );

};

#endif
