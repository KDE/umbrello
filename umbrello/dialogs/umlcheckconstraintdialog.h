/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLCHECKCONSTRAINTDIALOG_H
#define UMLCHECKCONSTRAINTDIALOG_H

//qt  includes
#include <qwidget.h>
#include <qlabel.h>

//kde includes
#include <kdialog.h>

class KLineEdit;
class KTextEdit;

class UMLDoc;
class UMLCheckConstraint;
/**
 * A dialog page to display check constraint properties.
 *
 * @short A dialog page to display check constraint properties.
 * @author Sharan Rao
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLCheckConstraintDialog : public KDialog {
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

    /**
    *   Sets up the dialog
    */
    void setupDialog();

    /**
     * Apply Changes
     */
    bool apply();
private:


    UMLCheckConstraint* m_pCheckConstraint;

    /**
      * The UMLDocument where all objects live
      */
    UMLDoc* m_doc;


    /**
     * The GUI widgets
     */
    QLabel* m_pNameL;
    KLineEdit* m_pNameLE;

    QLabel* m_pCheckConditionL;
    KTextEdit* m_pCheckConditionTE;

public slots:

    /**
     * Used when the OK button is clicked.  Calls apply()
     */
    void slotOk();

};

#endif
