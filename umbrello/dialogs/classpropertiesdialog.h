/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CLASSPROPERTIESDIALOG_H
#define CLASSPROPERTIESDIALOG_H

#include "dialogbase.h"

class ClassAssociationsPage;
class ClassGeneralPage;
class ClassifierListPage;
class ClassOptionsPage;
class ConstraintListPage;
class KFontChooser;
class PackageContentsPage;
class ObjectWidget;
class UMLDoc;
class UMLObject;
class UMLWidget;
class UMLWidgetStylePage;

/**
 * @author Paul Hensgen <phensgen@techie.com>
 * @version 1.0
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class ClassPropertiesDialog : public DialogBase
{
    Q_OBJECT
public:
    ClassPropertiesDialog(QWidget *parent, UMLObject *c, bool assoc = false);
    ClassPropertiesDialog(QWidget *parent, ObjectWidget * o);
    ClassPropertiesDialog(QWidget *parent, UMLWidget * o);

    ~ClassPropertiesDialog();

    enum Page{page_gen = 0, page_att, page_op, page_entatt, page_constraint, page_template,
              page_assoc, page_options, page_color, page_font};

    virtual void apply();

protected slots:
    void slotOk();
    void slotApply();

protected:
    void setupPages(bool assoc = false);
    void setupGeneralPage();
    void setupDisplayPage();
    void setupAttributesPage();
    void setupOperationsPage();
    void setupTemplatesPage();
    void setupEnumLiteralsPage();
    void setupEntityAttributesPage();
    void setupEntityConstraintsPage();
    void setupContentsPage();
    void setupAssociationsPage();
    void setupInstancePages();
    void setupFontPage();

private:
    KFontChooser*        m_pChooser;
    ClassGeneralPage*    m_pGenPage;
    ClassifierListPage*  m_pAttPage;
    ClassifierListPage*  m_pOpsPage;
    ClassifierListPage*  m_pTemplatePage;
    ClassifierListPage*  m_pEnumLiteralPage;
    ClassifierListPage*  m_pEntityAttributePage;
    ConstraintListPage*  m_pEntityConstraintPage;
    PackageContentsPage* m_pPkgContentsPage;
    ClassAssociationsPage* m_pAssocPage;
    ClassOptionsPage*    m_pOptionsPage;

    UMLDoc*              m_doc;
    UMLObject*           m_pObject;
    UMLWidget*           m_pWidget;

private:
    void init();

};

#endif
