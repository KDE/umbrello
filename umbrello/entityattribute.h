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
 * @author Jonathan Riddell <jr@jriddell.org>
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
    UMLEntityAttribute(UMLObject* parent, const QString& name,
                       Uml::IDType id = Uml::id_None,
                       Uml::Visibility s = Uml::Visibility::Private,
                       UMLObject *type = 0, const QString& iv = 0);

    /**
     * Sets up an entityattribute.
     *
     * @param parent    The parent of this UMLEntityAttribute.
     */
    UMLEntityAttribute(UMLObject* parent);

    /**
     * Overloaded '==' operator
     */
    bool operator==(const UMLEntityAttribute& rhs);

    /**
     * destructor.
     */
    virtual ~UMLEntityAttribute();

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    QString getAttributes() const;

    void setAttributes(const QString& attributes);

    Uml::DBIndex_Type getIndexType() const;

    void setIndexType(const Uml::DBIndex_Type indexType);

    QString getValues() const;

    void setValues(const QString& values);

    bool getAutoIncrement() const;

    void setAutoIncrement(const bool autoIncrement);

    bool getNull() const;

    void setNull(const bool null);

    QString toString(Uml::Signature_Type sig = Uml::st_NoSig);

    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

    bool showPropertiesDialog(QWidget* parent);

protected:

    void init();

    bool load(QDomElement& element);

private:
    Uml::DBIndex_Type m_indexType;
    QString m_values;
    QString m_attributes;
    bool m_autoIncrement;
    bool m_null;
};

#endif

