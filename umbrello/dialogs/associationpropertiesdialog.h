/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ASSOCIATIONPROPERTIESDIALOG_H
#define ASSOCIATIONPROPERTIESDIALOG_H

#include "multipagedialogbase.h"

class AssociationWidget;

/**
 * Based off of AssociationPropertiesDialog class
 * @author Brian Thomas <Brian.A.Thomas@gsfc.nasa.gov>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class AssociationPropertiesDialog : public MultiPageDialogBase
{
    Q_OBJECT
public:
    AssociationPropertiesDialog(QWidget  *parent, AssociationWidget *a, int pageNum = nullptr);
    ~AssociationPropertiesDialog();

protected:
    void setupPages();

protected Q_SLOTS:
    void slotOk();
    void slotApply();

private:
    AssociationWidget  *m_pAssoc;

};

#endif

