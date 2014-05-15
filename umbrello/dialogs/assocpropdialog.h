/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef ASSOCPROPDIALOG_H
#define ASSOCPROPDIALOG_H

#include "dialogbase.h"

class AssociationWidget;
class AssocRolePage;
class AssocGenPage;
class KFontChooser;
class UMLWidgetStylePage;

/**
 * Based off of AssocPropDialog class
 * @author Brian Thomas <Brian.A.Thomas@gsfc.nasa.gov>
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class AssocPropDialog : public DialogBase
{
    Q_OBJECT
public:
    AssocPropDialog(QWidget *parent, AssociationWidget *a, int pageNum = 0);
    ~AssocPropDialog();

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

