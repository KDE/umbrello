/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

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
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLAttribute : public UMLClassifierListItem
{
    Q_OBJECT
public:

    UMLAttribute(UMLObject *parent, const QString& name,
                 Uml::IDType id = Uml::id_None,
                 Uml::Visibility s = Uml::Visibility::Private,
                 UMLObject *type = 0, const QString& iv = QString());
    UMLAttribute(UMLObject *parent);

    bool operator==(const UMLAttribute &rhs) const;

    virtual ~UMLAttribute();

    virtual void copyInto(UMLObject *lhs) const;

    void setName(const QString &name);

    void setVisibility(Uml::Visibility s);

    virtual UMLObject* clone() const;

    QString getInitialValue() const;

    void setInitialValue( const QString &iv );

    QString toString(Uml::SignatureType sig = Uml::SignatureType::NoSig);

    QString getFullyQualifiedName( const QString& separator = QString(),
                                  bool includeRoot = false) const;

    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    virtual bool showPropertiesDialog(QWidget* parent);

    void setParmKind (Uml::Parameter_Direction pk);
    Uml::Parameter_Direction getParmKind () const;

    virtual UMLClassifierList getTemplateParams();

signals:

    void attributeChanged();

protected:

    bool load( QDomElement & element );

    QString m_InitialValue; ///< text for the attribute's initial value.
    Uml::Parameter_Direction m_ParmKind;

private:

    void setTemplateParams(const QString& templateParam, UMLClassifierList &templateParamList);

};

#endif
