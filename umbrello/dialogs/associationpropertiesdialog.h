/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef ASSOCIATIONPROPERTIESDIALOG_H
#define ASSOCIATIONPROPERTIESDIALOG_H

#include "multipagedialogbase.h"

class AssociationWidget;

/**
 * Based off of AssociationPropertiesDialog class
 * @author Brian Thomas <Brian.A.Thomas@gsfc.nasa.gov>
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class AssociationPropertiesDialog : public MultiPageDialogBase
{
    Q_OBJECT
public:
    AssociationPropertiesDialog(QWidget *parent, AssociationWidget *a, int pageNum = 0);
    ~AssociationPropertiesDialog();

protected:
    void setupPages();

protected slots:
    void slotOk();
    void slotApply();

private:
    AssociationWidget  *m_pAssoc;

};

#endif

