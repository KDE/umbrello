/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/


#ifndef UMLROLEDIALOG_H
#define UMLROLEDIALOG_H

//kde includes
#include <kdialog.h>

/**
 * @author Paul Hensgen
 */
class UMLRole;
class UMLRoleProperties;

class UMLRoleDialog : public KDialog
{
    Q_OBJECT
public:

    /**
     *   Constructor
     */
    UMLRoleDialog( QWidget * parent, UMLRole * pRole );

    /**
     *   Deconstructor
     */
    ~UMLRoleDialog();

protected:

    void setupDialog();

    bool apply();

    /**
     * The role to represent.
     */
    UMLRole * m_pRole;

private:

    UMLRoleProperties * m_pRoleProps;

public slots:

    void slotApply();

    void slotOk();
};

#endif
