/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLUNIQUECONSTRAINTDIALOG_H
#define UMLUNIQUECONSTRAINTDIALOG_H

//app includes
#include "umlentityattributelist.h"

//kde includes
#include "singlepagedialogbase.h"

class KComboBox;
class KLineEdit;
class QAbstractButton;
class QDialogButtonBox;
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
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
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
    KLineEdit* m_pNameLE;
    QListWidget* m_pAttributeListLW;
    KComboBox* m_pAttributeCB;
    QPushButton* m_pAddPB, *m_pRemovePB;
    QDialogButtonBox *m_dialogButtonBox;

    // local cache
    UMLEntityAttributeList m_pEntityAttributeList;
    UMLEntityAttributeList m_pConstraintAttributeList;

public slots:
    void slotResetWidgetState();
    void slotAddAttribute();
    void slotDeleteAttribute();

protected slots:
    virtual void slotButtonClicked(QAbstractButton *button);

};

#endif
