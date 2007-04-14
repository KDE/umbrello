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

#ifndef UMLROLEPROPERTIES_H
#define UMLROLEPROPERTIES_H

//my class includes
#include "umlrolepropertiesbase.h"
#include "../umlrole.h"

/**
 * Displays properties of a UMLRole in a widget which may be used as
 * a properties page or a stand-alone dialog.
 *
 * @author Brian Thomas <brian.thomas@gsfc.nasa.gov>
 */
class UMLRoleProperties : public UMLRolePropertiesBase {
    Q_OBJECT
public:

    /**
     *  Sets up the UMLRoleProperties.
     *
     *  @param  parent  The parent widget to the UMLRoleProperties.
     *  @param  role    The UMLRole to display the properties of.
     */
    UMLRoleProperties( QWidget *parent, UMLRole *role);

    /**
     *  Standard deconstructor.
     */
    ~UMLRoleProperties();

    /**
     *  Will move information from the dialog into the object.
     *  Call when the ok or apply button is pressed.
     */
    void updateObject();

protected:

    // the parent role object
    UMLRole * m_pRole;

private:

    void constructWidget();

};

#endif
