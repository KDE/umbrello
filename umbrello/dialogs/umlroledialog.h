/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLROLEDIALOG_H
#define UMLROLEDIALOG_H

//kde includes
#include "singlepagedialogbase.h"

/**
 * @author Paul Hensgen
 */
class UMLRole;
class UMLRoleProperties;

class UMLRoleDialog : public SinglePageDialogBase
{
    Q_OBJECT
public:
    UMLRoleDialog(QWidget *parent, UMLRole *pRole);
    ~UMLRoleDialog();

protected:
    void setupDialog();

    bool apply();

    UMLRole *m_pRole;  //< The role to represent.

private:
    UMLRoleProperties *m_pRoleProps;

};

#endif
