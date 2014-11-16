/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/
#ifndef UMLROLEPROPERTIES_H
#define UMLROLEPROPERTIES_H

#include "dialogpagebase.h"
#include "ui_umlrolepropertiesbase.h"

class UMLRole;

class UMLRolePropertiesBase : public DialogPageBase, public Ui::UMLRolePropertiesBase
{
public:
  explicit UMLRolePropertiesBase(QWidget *parent) : DialogPageBase(parent) {
    setupUi(this);
  }
};

/**
 * Displays properties of a UMLRole in a widget which may be used as
 * a properties page or a stand-alone dialog.
 *
 * @author Brian Thomas <brian.thomas@gsfc.nasa.gov>
 */
class UMLRoleProperties : public UMLRolePropertiesBase
{
    Q_OBJECT
public:
    UMLRoleProperties(QWidget *parent, UMLRole *role);
    ~UMLRoleProperties();

    void updateObject();

protected:

    // the parent role object
    UMLRole * m_pRole;

private:

    void constructWidget();

};

#endif
