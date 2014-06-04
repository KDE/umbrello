/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef ASSOCPROPDLG_H
#define ASSOCPROPDLG_H

#include "dialogbase.h"

class AssociationWidget;
class AssocRolePage;
class AssocGenPage;
class KFontChooser;
class UMLWidgetStylePage;

/**
 * Based off of AssocPropDlg class
 * @author Brian Thomas <Brian.A.Thomas@gsfc.nasa.gov>
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class AssocPropDlg : public DialogBase
{
    Q_OBJECT
public:
    AssocPropDlg(QWidget *parent, AssociationWidget *a, int pageNum = 0);
    ~AssocPropDlg();

    enum Page { page_gen = 0, page_role, page_font };

protected:
    void setupPages();

protected slots:
    void slotOk();
    void slotApply();

private:
    AssocGenPage       *m_pGenPage;
    AssocRolePage      *m_pRolePage;
    UMLWidgetStylePage *m_pStylePage;
    KFontChooser       *m_pChooser;
    AssociationWidget  *m_pAssoc;

};

#endif

