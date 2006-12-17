/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
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

class UMLTemplate : public UMLClassifierListItem {
public:
    /**
     * Sets up a template.
     *
     * @param parent    The parent of this UMLTemplate (i.e. its concept).
     * @param name              The name of this UMLTemplate.
     * @param id                The unique id given to this UMLTemplate.
     * @param type              The type of this UMLTemplate.
     */
    UMLTemplate(const UMLObject *parent, const QString& name,
                Uml::IDType id = Uml::id_None, const QString& type = "class");

    /**
     * Sets up a template.
     *
     * @param parent    The parent of this UMLTemplate (i.e. its concept).
     */
    UMLTemplate(const UMLObject *parent);

    /**
     * Overloaded '==' operator
     */
    bool operator==(UMLTemplate &rhs);

    /**
     * Copy the internal presentation of this object into the new
     * object.
     */
    virtual void copyInto(UMLTemplate *rhs) const;

    /**
     * Make a clone of this object.
     */
    virtual UMLObject* clone() const;

    /**
     * destructor
     */
    virtual ~UMLTemplate();

    /**
     * Returns a string representation of the UMLTemplate.
     *
     * @param sig               Currently unused.
     * @return  Returns a string representation of the UMLTemplate.
     */
    QString toString(Uml::Signature_Type sig = Uml::st_NoSig);

    /**
     * Overrides method from UMLClassifierListItem.
     * Returns the type name of the UMLTemplate.
     * If the template parameter is a class, there is no separate
     * type object. In this case, getTypeName() returns "class".
     *
     * @return  The type name of the UMLClassifierListItem.
     */
    virtual QString getTypeName();

    /**
     * Display the properties configuration dialog for the template.
     *
     * @return  Success status.
     */
    bool showPropertiesDialog(QWidget* parent);

    /**
     * Writes the <UML:TemplateParameter> XMI element.
     */
    void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);

protected:
    /**
     * Loads the <UML:TemplateParameter> XMI element.
     */
    bool load(QDomElement & element);

};

#endif
