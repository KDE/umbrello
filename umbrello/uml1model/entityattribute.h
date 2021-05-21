/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ENTITYATTRIBUTE_H
#define ENTITYATTRIBUTE_H

#include "attribute.h"
#include "basictypes.h"

/**
 * This class is used to set up information for an entityattribute.  This is a database field
 * It has a type, name, index type and default value.
 *
 * @short Sets up entityattribute information.
 * @author Jonathan Riddell <jr@jriddell.org>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLEntityAttribute : public UMLAttribute
{
    Q_OBJECT
    Q_ENUMS(DBIndex_Type)
public:
    enum DBIndex_Type
    {
        None  =  1100,
        Primary,
        Index,
        Unique
    };

    UMLEntityAttribute(UMLObject* parent, const QString& name,
                       Uml::ID::Type id = Uml::ID::None,
                       Uml::Visibility::Enum s = Uml::Visibility::Private,
                       UMLObject *type = 0, const QString& iv = QString());
    explicit UMLEntityAttribute(UMLObject* parent);
    virtual ~UMLEntityAttribute();

    bool operator==(const UMLEntityAttribute& rhs) const;

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    void setAttributes(const QString& attributes);
    QString getAttributes() const;

    void setIndexType(const DBIndex_Type indexType);
    DBIndex_Type indexType() const;

    void setValues(const QString& values);
    QString getValues() const;

    void setAutoIncrement(const bool autoIncrement);
    bool getAutoIncrement() const;

    void setNull(const bool null);
    bool getNull() const;

    QString toString(Uml::SignatureType::Enum sig = Uml::SignatureType::NoSig, bool withStereotype=false);

    virtual void saveToXMI1(QXmlStreamWriter& writer);

    virtual bool showPropertiesDialog(QWidget* parent = 0);

protected:
    void init();

    bool load1(QDomElement& element);

private:
    DBIndex_Type   m_indexType;
    QString        m_values;
    QString        m_attributes;
    bool           m_autoIncrement;
    bool           m_null;
};

#endif
