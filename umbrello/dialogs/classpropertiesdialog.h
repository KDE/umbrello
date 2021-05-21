/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CLASSPROPERTIESDIALOG_H
#define CLASSPROPERTIESDIALOG_H

#include "multipagedialogbase.h"

class ClassAssociationsPage;
class ClassGeneralPage;
class ClassifierListPage;
class ClassOptionsPage;
class ConstraintListPage;
class EntityWidget;
class PackageContentsPage;
class ObjectWidget;
class UMLDoc;
class UMLObject;
class UMLWidget;
class UMLWidgetStylePage;

/**
 * @author Paul Hensgen <phensgen@techie.com>
 * @version 1.0
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class ClassPropertiesDialog : public MultiPageDialogBase
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
    void setupEntityDisplayPage(EntityWidget *widget);
    void setupEntityAttributesPage();
    void setupEntityConstraintsPage();
    void setupContentsPage();
    void setupAssociationsPage();
    void setupInstancePages();

private:
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
