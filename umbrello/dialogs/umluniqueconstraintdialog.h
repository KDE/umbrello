/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLUNIQUECONSTRAINTDIALOG_H
#define UMLUNIQUECONSTRAINTDIALOG_H

//app includes
#include "umlentityattributelist.h"

//kde includes
#include "singlepagedialogbase.h"

class KComboBox;
class QLineEdit;
class QGroupBox;
class QLabel;
class QListWidget;
class QPushButton;
class UMLUniqueConstraint;

/**
 * A dialog page to display unique constraint properties.
 *
 * @short A dialog page to display unique constraint properties.
 * @author Sharan Rao
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLUniqueConstraintDialog : public SinglePageDialogBase
{
    Q_OBJECT
public:
    UMLUniqueConstraintDialog(QWidget* parent, UMLUniqueConstraint* pUniqueConstraint);
    ~UMLUniqueConstraintDialog();

protected:
    void setupDialog();

    virtual bool apply();

private:
    UMLUniqueConstraint *m_pUniqueConstraint;

    // GUI Widgets
    QGroupBox* m_pAttributeListGB;
    QLabel* m_pNameL;
    QLineEdit* m_pNameLE;
    QListWidget* m_pAttributeListLW;
    KComboBox* m_pAttributeCB;
    QPushButton* m_pAddPB, *m_pRemovePB;

    // local cache
    UMLEntityAttributeList m_pEntityAttributeList;
    UMLEntityAttributeList m_pConstraintAttributeList;

public Q_SLOTS:
    void slotResetWidgetState();
    void slotAddAttribute();
    void slotDeleteAttribute();

};

#endif
