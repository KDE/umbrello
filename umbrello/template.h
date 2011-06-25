/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef TEMPLATE_H
#define TEMPLATE_H

#include "classifierlistitem.h"

/**
 * This class holds information used by template classes, called
 * paramaterised class in UML and a generic in Java.  It has a
 * type (usually just "class") and name.
 *
 * @short Sets up template information.
 * @author Jonathan Riddell
 * @see UMLObject
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLTemplate : public UMLClassifierListItem
{
public:

    /**
     * Sets up a template.
     *
     * @param parent    The parent of this UMLTemplate (i.e. its concept).
     * @param name              The name of this UMLTemplate.
     * @param id                The unique id given to this UMLTemplate.
     * @param type              The type of this UMLTemplate.
     */
    UMLTemplate(UMLObject *parent, const QString& name,
                Uml::IDType id = Uml::id_None, const QString& type = "class");

    /**
     * Sets up a template.
     *
     * @param parent    The parent of this UMLTemplate (i.e. its concept).
     */
    UMLTemplate(UMLObject *parent);

    /**
     * Overloaded '==' operator
     */
    bool operator==(const UMLTemplate &rhs) const;

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    /**
     * destructor
     */
    virtual ~UMLTemplate();

    QString toString(Uml::SignatureType sig = Uml::SignatureType::NoSig);

    virtual QString getTypeName() const;

    bool showPropertiesDialog(QWidget* parent);

    void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);

protected:

    bool load(QDomElement & element);

};

#endif
