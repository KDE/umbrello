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

#ifndef UMLUNIQUECONSTRAINTDIALOG_H
#define UMLUNIQUECONSTRAINTDIALOG_H

//qt  includes
#include <qwidget.h>
#include <qlabel.h>
#include <q3groupbox.h>
#include <q3listbox.h>
#include <q3textedit.h>
#include <qlineedit.h>

//kde includes
#include <karrowbutton.h>
#include <kcombobox.h>
#include <kdialog.h>

//app includes
#include "../umlclassifierlistitemlist.h"
#include "../umlentityattributelist.h" 

class UMLObject;
class UMLClassifier;
class UMLDoc;
class UMLUniqueConstraint;

/**
 * A dialog page to display unique constraint properties.
 * 
 * @short A dialog page to display unique constraint properties.
 * @author Sharan Rao
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLUniqueConstraintDialog : public KDialog {
    Q_OBJECT
public:
    /**
     *  Sets up the UMLUniqueConstraintDialog
     *
     *  @param parent   The parent to the UMLUniqueConstraintDialog.
     *  @param pUniqueConstraint The Unique Constraint to show the properties of.
     */
    UMLUniqueConstraintDialog(QWidget* parent, UMLUniqueConstraint* pUniqueConstraint);

    /**
     *  Standard destructor
     */
    ~UMLUniqueConstraintDialog();


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


    UMLUniqueConstraint* m_pUniqueConstraint;
    
    /**
      * The UMLDocument where all objects live
      */
    UMLDoc* m_doc;

    /* GUI Widgets */
    QGroupBox* m_pAttributeListGB;
    QLabel* m_pNameL;
    QLineEdit* m_pNameLE;
    Q3ListBox* m_pAttributeListLB;
    KComboBox* m_pAttributeCB; 

    /* local cache */
    UMLEntityAttributeList m_pEntityAttributeList;
 
    UMLEntityAttributeList m_pConstraintAttributeList;
 
public slots:

    /**
     * Used when the Apply Button is clicked
     */
    void slotApply();

    /**
     * Used when the OK button is clicked.  Calls apply()
     */
    void slotOk();

    /**
     * adds attribute to the list
     */
    void slotAddAttribute();

    /**
     * deletes an attribute from the list
     */
    void slotDeleteAttribute();
 
};

#endif
