/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/


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

    /**
     *   Constructor
     */
    UMLRoleDialog(QWidget * parent, UMLRole * pRole);

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
};

#endif
