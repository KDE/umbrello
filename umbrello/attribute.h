/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2006                                                *
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                  *
 ***************************************************************************/

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "classifierlistitem.h"
#include "umlnamespace.h"

/**
 * This class is used to set up information for an attribute.  This is like
 * a programming attribute.  It has a type, name, visibility and initial value.
 *
 * @short Sets up attribute information.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see UMLObject
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLAttribute : public UMLClassifierListItem {
public:
    /**
     * Sets up an attribute.
     *
     * @param parent    The parent of this UMLAttribute.
     * @param name      The name of this UMLAttribute.
     * @param id        The unique id given to this UMLAttribute.
     * @param s         The visibility of the UMLAttribute.
     * @param type      The type of this UMLAttribute.
     * @param iv        The initial value of the attribute.
     */
    UMLAttribute(const UMLObject *parent, QString name, Uml::IDType id = Uml::id_None,
             Uml::Visibility s = Uml::Visibility::Private, QString type = "int", QString iv = 0);

    /**
     * Sets up an attribute.
     *
     * @param parent    The parent of this UMLAttribute.
     */
    UMLAttribute(const UMLObject *parent);

    /**
     * Overloaded '==' operator
     */
    bool operator==( UMLAttribute &rhs);

    /**
     * destructor.
     */
    virtual ~UMLAttribute();

    /**
     * Copy the internal presentation of this object into the UMLAttribute
     * object.
     */
    virtual void copyInto(UMLAttribute *rhs) const;

    /**
     * Make a clone of the UMLAttribute.
     */
    virtual UMLObject* clone() const;

    /**
     * Returns The initial value of the UMLAttribute.
     *
     * @return  The initial value of the Atrtibute.
     */
    QString getInitialValue();

    /**
     * Sets the initial value of the UMLAttribute.
     *
     * @param iv                The initial value of the UMLAttribute.
     */
    void setInitialValue( const QString &iv );

    /**
     * Returns a string representation of the UMLAttribute.
     *
     * @param sig               If true will show the attribute type and
     *                  initial value.
     * @return  Returns a string representation of the UMLAttribute.
     */
    QString toString(Uml::Signature_Type sig = Uml::st_NoSig);

    /**
     * Reimplement method from UMLObject.
     */
    QString getFullyQualifiedName(QString separator = QString::null,
                                  bool includeRoot = false) const;

    /**
     * Creates the <UML:Attribute> XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * Display the properties configuration dialog for the attribute.
     */
    virtual bool showPropertiesDialog(QWidget* parent);

    void setParmKind (Uml::Parameter_Direction pk);
    Uml::Parameter_Direction getParmKind () const;

protected:
    /**
     * Loads the <UML:Attribute> XMI element.
     */
    bool load( QDomElement & element );

    QString m_InitialValue; ///< text for the attribute's initial value.
    Uml::Parameter_Direction m_ParmKind;
};

#endif
