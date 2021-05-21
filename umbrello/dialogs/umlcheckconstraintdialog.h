/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLCHECKCONSTRAINTDIALOG_H
#define UMLCHECKCONSTRAINTDIALOG_H

//kde includes
#include "singlepagedialogbase.h"

class UMLDoc;
class UMLCheckConstraint;
class KLineEdit;
class KTextEdit;
class QLabel;

/**
 * A dialog page to display check constraint properties.
 *
 * @short A dialog page to display check constraint properties.
 * @author Sharan Rao
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLCheckConstraintDialog : public SinglePageDialogBase
{
    Q_OBJECT
public:

    /**
     *  Sets up the UMLCheckConstraintDialog
     *
     *  @param parent   The parent to the UMLUniqueConstraintDialog.
     *  @param pUniqueConstraint The Unique Constraint to show the properties of.
     */
    UMLCheckConstraintDialog(QWidget* parent, UMLCheckConstraint* pUniqueConstraint);

    /**
     *  Standard destructor
     */
    ~UMLCheckConstraintDialog();

protected:

    void setupDialog();

    virtual bool apply();

private:

    UMLCheckConstraint* m_pCheckConstraint;

    /**
     * The GUI widgets
     */
    QLabel* m_pNameL;
    KLineEdit* m_pNameLE;
    QLabel* m_pCheckConditionL;
    KTextEdit* m_pCheckConditionTE;
};

#endif
