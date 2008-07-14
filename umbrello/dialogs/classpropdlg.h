/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CLASSPROPDLG_H
#define CLASSPROPDLG_H

// qt class includes
#include <QtGui/QFrame>
// kde class includes
#include <kpagedialog.h>
#include <kfontdialog.h>

#include "umlnamespace.h"
#include "icon_utils.h"

class ClassGenPage;
class ClassifierListPage;
class ConstraintListPage;
class PkgContentsPage;
class AssocPage;
class ClassOptionsPage;
class UMLWidgetColorPage;

class ObjectWidget;
class UMLDoc;
class UMLObject;
class UMLWidget;

/**
 * @author Paul Hensgen <phensgen@techie.com>
 * @version 1.0
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ClassPropDlg : public KPageDialog
{
    Q_OBJECT
public:

    /**
     *  Sets up a ClassPropDlg.
     *
     *  @param parent    The parent of the ClassPropDlg
     *  @param c         The UMLObject to display properties of.
     *  @param assoc     Determines whether to display associations
     */
    ClassPropDlg(QWidget *parent, UMLObject *c, bool assoc = false);

    /**
     *  Sets up a ClassPropDlg.
     *
     *  @param  parent  The parent of the ClassPropDlg
     *  @param  o       The ObjectWidget to display properties of.
     */
    ClassPropDlg(QWidget *parent, ObjectWidget * o);

    /**
     *  Sets up a ClassPropDlg.
     *
     *  @param  parent  The parent of the ClassPropDlg
     *  @param  o       The UMLWidget to display properties of.
     */
    ClassPropDlg(QWidget *parent, UMLWidget * o);


    /**
     *  Standard deconstructor.
     */
    ~ClassPropDlg();

    enum Page{page_gen = 0, page_att, page_op, page_entatt, page_constraint , page_template,
              page_assoc, page_options, page_color, page_font};

protected slots:

    /**
     * Calls slotApply() and accepts (closes) the dialog.
     */
    void slotOk();

    /**
     * Applys the settings in the dialog to the widget and object.
     */
    void slotApply();

protected:

    /**
     * Sets up the general, attribute, operations, template and association pages as appropriate.
     */
    void setupPages(bool assoc = false);

    /**
     * Sets up the page "General" for the component.
     */
    void setupGeneralPage();

    /**
     * Sets up the page "Color" for the component.
     */
    void setupColorPage();

    /**
     * Sets up the page "Display" for the component.
     */
    void setupDisplayPage();

    /**
     * Sets up the page "Attributes" for the component.
     */
    void setupAttributesPage();

    /**
     * Sets up the page "Operations" for the component.
     */
    void setupOperationsPage();

    /**
     * Sets up the page "Templates" for the component.
     */
    void setupTemplatesPage();

    /**
     * Sets up the page "Enum Literals" for the component.
     */
    void setupEnumLiteralsPage();

    /**
     * Sets up the page "Entity Attributes" for the component.
     */
    void setupEntityAttributesPage();

    /**
     * Sets up the page "Entity Constraints" for the component.
     */
    void setupEntityConstraintsPage();

    /**
     * Sets up the page "Contents" for the component.
     */
    void setupContentsPage();

    /**
     * Sets up the page "Associations" for the component.
     */
    void setupAssociationsPage();

    /**
     * Sets up the general page for the component.
     */
    void setupInstancePages();

    /**
     * Sets up the font page.
     */
    void setupFontPage();

private:

    KFontChooser*        m_pChooser;
    ClassGenPage*        m_pGenPage;
    ClassifierListPage*  m_pAttPage;
    ClassifierListPage*  m_pOpsPage;
    ClassifierListPage*  m_pTemplatePage;
    ClassifierListPage*  m_pEnumLiteralPage;
    ClassifierListPage*  m_pEntityAttributePage;
    ConstraintListPage*  m_pEntityConstraintPage;
    PkgContentsPage*     m_pPkgContentsPage;
    AssocPage*           m_pAssocPage;
    ClassOptionsPage*    m_pOptionsPage;
    UMLWidgetColorPage*  m_pColorPage;

    UMLDoc*              m_pDoc;
    UMLObject*           m_pObject;
    UMLWidget*           m_pWidget;

    enum Page_Type {
        pt_Object = 1,    // Show General page + Assoc. page if Class i.e. no colours page
        pt_ObjectWidget,  // Shows pages needed for an ObjectWidget
        pt_Widget         // Shows pages needed for any other widget
    };

    Page_Type m_Type;

private:

    void init();

    QFrame* createPage(const QString& name, const QString& header, Icon_Utils::Icon_Type icon);

};

#endif
