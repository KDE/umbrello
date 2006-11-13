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


#ifndef UMLROLEDIALOG_H
#define UMLROLEDIALOG_H

//kde includes
#include <kdialogbase.h>

/**
 * @author Paul Hensgen
 */

class UMLRole;
class UMLRoleProperties;

class UMLRoleDialog : public KDialogBase {
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

    /**
    *   Sets up the dialog
    */
    void setupDialog();

    /**
     * Checks if changes are valid and applies them if they are,
     * else returns false
     */
    bool apply();

    /**
    * The role to represent.
    */
    UMLRole * m_pRole;

private:

    UMLRoleProperties * m_pRoleProps;

public slots:

    /**
     * I don't think this is used, but if we had an apply button
     * it would slot into here
     */
    void slotApply();

    /**
     * Used when the OK button is clicked.  Calls apply()
     */
    void slotOk();
};

#endif
