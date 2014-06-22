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

#include "dialogbase.h"

class AssociationWidget;
class AssociationRolePage;
class AssociationGeneralPage;
class KFontChooser;
class UMLWidgetStylePage;

/**
 * Based off of AssociationPropertiesDialog class
 * @author Brian Thomas <Brian.A.Thomas@gsfc.nasa.gov>
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class AssociationPropertiesDialog : public DialogBase
{
    Q_OBJECT
public:
    AssociationPropertiesDialog(QWidget *parent, AssociationWidget *a, int pageNum = 0);
    ~AssociationPropertiesDialog();

    enum Page { page_gen = 0, page_role, page_font };

protected:
    void setupPages();

protected slots:
    void slotOk();
    void slotApply();

private:
    AssociationGeneralPage *m_pGenPage;
    AssociationRolePage *m_pRolePage;
    UMLWidgetStylePage *m_pStylePage;
    KFontChooser       *m_pChooser;
    AssociationWidget  *m_pAssoc;

};

#endif

