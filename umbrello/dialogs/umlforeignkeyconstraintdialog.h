/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLFOREIGNKEYCONSTRAINTDIALOG_H
#define UMLFOREIGNKEYCONSTRAINTDIALOG_H

//app includes
#include "umlclassifierlistitemlist.h"
#include "umlentityattributelist.h"

//kde includes
#include <klineedit.h>
#include <karrowbutton.h>
#include <kcombobox.h>
#include <kdialog.h>
#include <kpagedialog.h>

//qt  includes
#include <QtCore/QList>

class UMLDoc;
class UMLForeignKeyConstraint;
class QGroupBox;
class QLabel;
class QPushButton;
class QTreeWidget;

/**
 * A dialog page to display foreignkey constraint properties.
 *
 * @short A dialog page to display foreignkey constraint properties.
 * @author Sharan Rao
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLForeignKeyConstraintDialog : public KPageDialog
{
    Q_OBJECT
public:

    /**
     *  Sets up the UMLForeignKeyConstraintDialog
     *
     *  @param parent   The parent to the UMLForeignKeyConstraintDialog.
     *  @param pForeignKeyConstraint The Unique Constraint to show the properties of
     */
    UMLForeignKeyConstraintDialog(QWidget* parent, UMLForeignKeyConstraint* pForeignKeyConstraint);

    /**
     *  Standard deconstructor.
     */
    ~UMLForeignKeyConstraintDialog();

private:

    bool apply();

    void setupGeneralPage();

    void setupColumnPage();

    void refillReferencedAttributeCB();

    void refillLocalAttributeCB();

    /**
     * The UMLDocument where all objects live.
     */
    UMLDoc* m_doc;

    UMLForeignKeyConstraint* m_pForeignKeyConstraint;

    struct GeneralWidgets {

        QGroupBox* generalGB;
        QGroupBox* actionGB;

        QLabel* referencedEntityL;
        QLabel* nameL;

        KLineEdit* nameT;

        KComboBox* referencedEntityCB;

        QLabel* onUpdateL;
        QLabel* onDeleteL;
        KComboBox* updateActionCB;
        KComboBox* deleteActionCB;
    }; // end general widgets

    struct ColumnWidgets {

        QTreeWidget* mappingTW;

        KComboBox* localColumnCB;
        KComboBox* referencedColumnCB;

        QLabel* localColumnL;
        QLabel* referencedColumnL;

        QPushButton* addPB,*removePB;
    }; // end column widgets

    // these attributes store the local cache

    UMLEntityAttributeList m_pLocalAttributeList;
    UMLEntityAttributeList m_pReferencedAttributeList;

    QList< QPair<UMLEntityAttribute*,UMLEntityAttribute*> > m_pAttributeMapList;

    /**
     * Temporary Storage for entity index in referencedColumnCB.
     * Used for reverting back a change in referenced entities.
     */
    int m_pReferencedEntityIndex;
    // end of local cache

    GeneralWidgets m_GeneralWidgets;
    ColumnWidgets m_ColumnWidgets;

    KPageWidgetItem *pageGeneral,*pageColumn;

public slots:

    void slotResetWidgetState();

    void slotApply();

    void slotOk();

    void slotAddPair();

    void slotDeletePair();

    void slotReferencedEntityChanged(int index);
};

#endif
