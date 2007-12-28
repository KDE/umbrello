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

#ifndef CHECKCONSTRAINT_H
#define CHECKCONSTRAINT_H

#include "entityconstraint.h"
#include "umlnamespace.h"

/**
 * This class is used to set up information for a unique entity constraint.
 *
 * @short Sets up Check Constraint information for UMLEntities.
 * @author Sharan Rao
 * @see UMLObject UMLClassifierListItem UMLEntityConstraint
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLCheckConstraint : public UMLEntityConstraint {
     Q_OBJECT
public:
    /**
     * Sets up a constraint.
     *
     * @param parent    The parent of this UMLCheckConstraint.
     * @param name      The name of this UMLCheckConstraint.
     * @param id        The unique id given to this UMLCheckConstraint.
     */
    UMLCheckConstraint(UMLObject *parent, const QString& name,
			Uml::IDType id = Uml::id_None );

    /**
     * Sets up a constraint.
     *
     * @param parent    The parent of this UMLCheckConstraint.
     */
    UMLCheckConstraint(UMLObject *parent);

    /**
     * Overloaded '==' operator
     */
    bool operator==(const UMLCheckConstraint &rhs);

    /**
     * destructor.
     */
    virtual ~UMLCheckConstraint();

    /**
     * Copy the internal presentation of this object into the UMLCheckConstraint
     * object.
     */
    virtual void copyInto(UMLObject *lhs) const;

    /**
     * Make a clone of the UMLCheckConstraint.
     */
    virtual UMLObject* clone() const;

    /**
     * Returns a string representation of the UMLCheckConstraint.
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
     * Returns a string represention of the condition that this constraint is checking
     */
    QString getCheckCondition() const {
        return m_CheckCondition;
    }

    /**
     * Sets the Condition that's to be checked
     */
    void setCheckCondition(const QString& condition) {
        m_CheckCondition = condition.trimmed();
    }

protected:
    /**
     * Loads the <UML:CheckConstraint> XMI element.
     */
    bool load( QDomElement & element );

private:

    /**
     * Initialises Check Constraint
     */
    void init();
     
    /**
     * The Check Condition
     */
    QString m_CheckCondition;

};

#endif
