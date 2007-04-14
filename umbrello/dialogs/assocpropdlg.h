/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ASSOCPROPDLG_H
#define ASSOCPROPDLG_H

//kde class includes
#include <kdialogbase.h>
#include <kfontdialog.h>
#include "../associationwidget.h"

class AssocRolePage;
class AssocGenPage;
class UMLDoc;
// class ObjectWidget;
// class UMLObject;
// class UMLWidget;

/**
 * Based off of AssocPropDlg class
 * @author Brian Thomas <Brian.A.Thomas@gsfc.nasa.gov>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AssocPropDlg : public KDialogBase {
    Q_OBJECT
public:

    /**
     *  Sets up a Association Properties Dialog.
     *  @param  parent  The parent of the AssocPropDlg
     *  @param  a       The Association Widget to display properties of.
     *  @param  pageNum The page to show first.
     */

    AssocPropDlg(QWidget *parent, AssociationWidget *a, int pageNum = 0);

    /**
     *  Standard deconstructor
     */
    ~AssocPropDlg();

    enum Page { page_gen = 0, page_role, page_font };

protected:
    void setupPages(AssociationWidget * assocWidget);
    void setupFontPage();
    void init();

protected slots:
    void slotOk();
    void slotApply();

private:
    AssocGenPage *m_pGenPage;
    AssocRolePage *m_pRolePage;
    KFontChooser * m_pChooser;
    AssociationWidget *m_pAssoc;

    UMLDoc *m_pDoc; // is this needed??

};

#endif /* ASSOCPROPDLG_H */

