/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLFOREIGNKEYCONSTRAINTDIALOG_H
#define UMLFOREIGNKEYCONSTRAINTDIALOG_H

//app includes
#include "multipagedialogbase.h"
#include "umlentityattributelist.h"

//kde includes

//qt  includes
#include <QList>

class KComboBox;
class KLineEdit;
class UMLDoc;
class UMLForeignKeyConstraint;
class QGroupBox;
class QLabel;
class QPushButton;
class QTreeWidget;

typedef QPair<UMLEntityAttribute*, UMLEntityAttribute*> EntityAttributePair;
typedef QList<EntityAttributePair> EntityAttributePairList;

/**
 * A dialog page to display foreignkey constraint properties.
 *
 * @short A dialog page to display foreignkey constraint properties.
 * @author Sharan Rao
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class UMLForeignKeyConstraintDialog : public MultiPageDialogBase
{
    Q_OBJECT
public:
    UMLForeignKeyConstraintDialog(QWidget* parent, UMLForeignKeyConstraint* pForeignKeyConstraint);
    ~UMLForeignKeyConstraintDialog();

private:
    bool apply();

    void setupGeneralPage();
    void setupColumnPage();

    void refillReferencedAttributeCB();
    void refillLocalAttributeCB();

    UMLDoc* m_doc;  ///< the UMLDocument where all objects live
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

        QPushButton* addPB, *removePB;
    }; // end column widgets

    // these attributes store the local cache

    UMLEntityAttributeList m_pLocalAttributeList;
    UMLEntityAttributeList m_pReferencedAttributeList;

    EntityAttributePairList m_pAttributeMapList;

    /**
     * Temporary Storage for entity index in referencedColumnCB.
     * Used for reverting back a change in referenced entities.
     */
    int m_pReferencedEntityIndex;
    // end of local cache

    GeneralWidgets m_GeneralWidgets;
    ColumnWidgets m_ColumnWidgets;

    KPageWidgetItem *pageGeneral, *pageColumn;

public slots:
    void slotResetWidgetState();
    void slotApply();
    void slotOk();
    void slotAddPair();
    void slotDeletePair();
    void slotReferencedEntityChanged(int index);
};

#endif
