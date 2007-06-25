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

#ifndef COMPONENT_H
#define COMPONENT_H

#include "package.h"


/**
 * This class contains the non-graphical information required for a
 * UML Component.
 * This class inherits from @ref UMLPackage which contains most
 * of the information.
 *
 * @short Non-graphical information for a Component.
 * @author Jonathan Riddell
 * @see UMLCanvasObject
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLComponent : public UMLPackage {
    Q_OBJECT
public:
    /**
     * Sets up a Component.
     *
     * @param name              The name of the Concept.
     * @param id                The unique id of the Concept.
     */
    explicit UMLComponent(const QString & name = "", Uml::IDType id = Uml::id_None);

    /**
     * Empty deconstructor.
     */
    virtual ~UMLComponent();

    /**
     * Initializes key variables of the class.
     */
    virtual void init();

    /**
     * Make a clone of this object.
     */
    virtual UMLObject* clone() const;

    /**
     * Creates the UML:Component element including its operations,
     * attributes and templates
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    /**
     * sets m_executable
     */
    void setExecutable(bool executable);

    /**
     * returns the value of m_executable
     */
    bool getExecutable();

protected:
    /**
     * Loads the UML:Component element including its operations,
     * attributes and templates
     */
    bool load( QDomElement & element );

private:
    /**
     * holds whether this is an executable component or not
     */
    bool m_executable;
};

#endif
