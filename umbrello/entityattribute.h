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

#ifndef ENTITYATTRIBUTE_H
#define ENTITYATTRIBUTE_H

#include "attribute.h"
#include "umlnamespace.h"

/**
 * This class is used to set up information for an entityattribute.  This is a database field
 * It has a type, name, index type and default value.
 *
 * @short Sets up entityattribute information.
 * @author Jonathan Riddell <jr @jriddell.org>
 * @see UMLObject
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLEntityAttribute : public UMLAttribute {
    Q_OBJECT
public:
    /**
     * Sets up an entityattribute.
     *
     * @param parent    The parent of this UMLEntityAttribute.
     * @param name      The name of this UMLEntityAttribute.
     * @param id        The unique id given to this UMLEntityAttribute.
     * @param s         The visibility of the UMLEntityAttribute.
     * @param type      The type of this UMLEntityAttribute.
     * @param iv        The initial value of the entityattribute.
     */
    UMLEntityAttribute(const UMLObject* parent, const QString& name,
                       Uml::IDType id = Uml::id_None,
                       Uml::Visibility s = Uml::Visibility::Private,
                       UMLObject *type = 0, const QString& iv = 0);

    /**
     * Sets up an entityattribute.
     *
     * @param parent    The parent of this UMLEntityAttribute.
     */
    UMLEntityAttribute(const UMLObject* parent);

    /**
     * Overloaded '==' operator
     */
    bool operator==( UMLEntityAttribute& rhs);

    /**
     * destructor.
     */
    virtual ~UMLEntityAttribute();

    /**
     * Copy the internal presentation of this object into the UMLEntityAttribute
     * object.
     */
    virtual void copyInto(UMLEntityAttribute* rhs) const;

    /**
     * Make a clone of the UMLEntityAttribute.
     */
    virtual UMLObject* clone() const;

    /**
     * Returns The value of the UMLEntityAttribute's attributes property.
     *
     * @return  The value of the UMLEntityAttribute's attributes property.
     */
    QString getAttributes() const;

    /**
     * Sets the UMLEntityAttribute's attributes property.
     *
     * @param attributes  The new value for the attributes property.
     */
    void setAttributes(const QString& attributes);

    /**
     * Returns the UMLEntityAttribute's index type property.
     *
     * @return  The value of the UMLEntityAttribute's index type property.
     */
    Uml::DBIndex_Type getIndexType() const;

    /**
     * Sets the UMLEntityAttribute's index type property.
     *
     * @param indexType  The UMLEntityAttribute's index type property.
     */
    void setIndexType(const Uml::DBIndex_Type indexType);

    /**
     * Returns the UMLEntityAttribute's length/values property.
     *
     * @return  The UMLEntityAttribute's length/values property.
     */
    QString getValues() const;

    /**
     * Sets the UMLEntityAttribute's length/values property.
     *
     * @param values    The new value of the length/values property.
     */
    void setValues(const QString& values);

    /**
     * Returns the UMLEntityAttribute's auto_increment boolean
     *
     * @return  The UMLEntityAttribute's auto_increment boolean
     */
    bool getAutoIncrement() const;

    /**
     * Sets the UMLEntityAttribute's auto_increment property
     *
     * @param autoIncrement  The UMLEntityAttribute's auto_increment property
     */
    void setAutoIncrement(const bool autoIncrement);

    /**
     * Returns the UMLEntityAttribute's allow null value.
     *
     * @return  The UMLEntityAttribute's allow null value.
     */
    bool getNull() const;

    /**
     * Sets the UMLEntityAttribute's allow null value.
     *
     * @param null      The UMLEntityAttribute's allow null value.
     */
    void setNull(const bool null);

    /**
     * Returns a string representation of the UMLEntityAttribute.
     *
     * @param sig               If true will show the entityattribute type and
     *                  initial value.
     * @return  Returns a string representation of the UMLEntityAttribute.
     */
    QString toString(Uml::Signature_Type sig = Uml::st_NoSig);

    /**
     * Creates the <UML:EntityAttribute> XMI element.
     */
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

    /**
     * Display the properties configuration dialog for the entityattribute.
     */
    bool showPropertiesDialog(QWidget* parent);

protected:
    /**
     * Initialize members of this class.
     * Auxiliary method used by constructors.
     */
    void init();

    /**
     * Loads the <UML:EntityAttribute> XMI element.
     */
    bool load(QDomElement& element);

private:
    Uml::DBIndex_Type m_indexType;
    QString m_values;
    QString m_attributes;
    bool m_autoIncrement;
    bool m_null;
};

#endif

