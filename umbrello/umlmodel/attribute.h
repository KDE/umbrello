/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "basictypes.h"
#include "classifierlistitem.h"
#include "umlclassifierlist.h"

/**
 * This class is used to set up information for an attribute.  This is like
 * a programming attribute.  It has a type, name, visibility and initial value.
 *
 * @short Sets up attribute information.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see UMLObject
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLAttribute : public UMLClassifierListItem
{
    Q_OBJECT
public:

    UMLAttribute(UMLObject *parent, const QString& name,
                 Uml::ID::Type id = Uml::ID::None,
                 Uml::Visibility::Enum s = Uml::Visibility::Private,
                 UMLObject *type = nullptr, const QString& iv = QString());
    explicit UMLAttribute(UMLObject *parent);

    bool operator==(const UMLAttribute &rhs) const;

    virtual ~UMLAttribute();

    virtual void copyInto(UMLObject *lhs) const;

    void setName(const QString &name);

    void setVisibility(Uml::Visibility::Enum s);

    virtual UMLObject* clone() const;

    QString getInitialValue() const;
    void setInitialValue(const QString &iv);

    QString toString(Uml::SignatureType::Enum sig = Uml::SignatureType::NoSig,
                     bool withStereotype=false) const;

    QString getFullyQualifiedName(const QString& separator = QString(),
                                  bool includeRoot = false) const;

    virtual void saveToXMI(QXmlStreamWriter& writer);

    virtual bool showPropertiesDialog(QWidget *parent = nullptr);

    void setParmKind(Uml::ParameterDirection::Enum pk);
    Uml::ParameterDirection::Enum getParmKind() const;

    virtual UMLClassifierList getTemplateParams();

Q_SIGNALS:

    void attributeChanged();

protected:

    bool load1(QDomElement & element);

    QString m_InitialValue; ///< text for the attribute's initial value.
    Uml::ParameterDirection::Enum m_ParmKind;

private:

    void setTemplateParams(const QString& templateParam, UMLClassifierList &templateParamList);

};

#endif
