/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLUNIQUECONSTRAINTDIALOG_H
#define UMLUNIQUECONSTRAINTDIALOG_H

//app includes
#include "umlclassifierlistitemlist.h"
#include "umlentityattributelist.h"

//kde includes
#include <kdialog.h>

//qt  includes
#include <QtGui/QWidget>

class UMLDoc;
class UMLUniqueConstraint;
class KComboBox;
class KLineEdit;
class QGroupBox;
class QLabel;
class QListWidget;
class QPushButton;

/**
 * A dialog page to display unique constraint properties.
 *
 * @short A dialog page to display unique constraint properties.
 * @author Sharan Rao
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLUniqueConstraintDialog : public KDialog
{
    Q_OBJECT
public:

    /**
     *  Sets up the UMLUniqueConstraintDialog.
     *  @param parent   The parent to the UMLUniqueConstraintDialog.
     *  @param pUniqueConstraint The Unique Constraint to show the properties of.
     */
    UMLUniqueConstraintDialog(QWidget* parent, UMLUniqueConstraint* pUniqueConstraint);

    /**
     *  Standard destructor.
     */
    ~UMLUniqueConstraintDialog();

protected:

    void setupDialog();

    bool apply();

private:

    UMLUniqueConstraint* m_pUniqueConstraint;

    /**
     * The UMLDocument where all objects live.
     */
    UMLDoc* m_doc;

    /* GUI Widgets */
    QGroupBox* m_pAttributeListGB;
    QLabel* m_pNameL;
    KLineEdit* m_pNameLE;
    QListWidget* m_pAttributeListLW;
    KComboBox* m_pAttributeCB;
    QPushButton* m_pAddPB,*m_pRemovePB;

    /* local cache */
    UMLEntityAttributeList m_pEntityAttributeList;

    UMLEntityAttributeList m_pConstraintAttributeList;

public slots:

    void slotResetWidgetState();

    void slotApply();

    void slotOk();

    void slotAddAttribute();

    void slotDeleteAttribute();

};

#endif
