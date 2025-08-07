/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "finddialog.h"

enum ButtonID {
    TreeView = -2,
    CurrentDiagram = -3,
    Diagrams = -4
};

FindDialog::FindDialog(QWidget *parent) :
    SinglePageDialogBase(parent, false, true)
{
    setCaption(i18n("Find"));
    QFrame * frame = new QFrame(this);
    setMainWidget(frame);
    setupUi(mainWidget());
    ui_buttonGroup->setId(ui_treeView, TreeView);
    ui_buttonGroup->setId(ui_CurrentDiagram, CurrentDiagram);
    ui_buttonGroup->setId(ui_Diagrams, Diagrams);
    connect(ui_buttonGroup, SIGNAL(idClicked(int)), this, SLOT(slotFilterButtonClicked(int)));
    ui_treeView->setChecked(true);
    ui_categoryAll->setChecked(true);
}

FindDialog::~FindDialog()
{
    disconnect(ui_buttonGroup, SIGNAL(idClicked(int)), this, SLOT(slotFilterButtonClicked(int)));
}

/**
 * return entered text.
 * @return text
 */
QString FindDialog::text() const
{
    return ui_searchTerm->text();
}

/**
 * Return user selected filter.
 * @return filter enum
 */
UMLFinder::Filter FindDialog::filter() const
{
    if (ui_treeView->isChecked())
        return UMLFinder::TreeView;
    else if (ui_CurrentDiagram->isChecked())
        return UMLFinder::CurrentDiagram;
    else
        return UMLFinder::AllDiagrams;
}

/**
 * Return user selected category.
 * @return category enum
 */
UMLFinder::Category FindDialog::category() const
{
    if (ui_categoryAll->isChecked())
        return UMLFinder::All;
    else if (ui_categoryClass->isChecked())
        return UMLFinder::Classes;
    else if (ui_categoryPackage->isChecked())
        return UMLFinder::Packages;
    else if (ui_categoryInterface->isChecked())
        return UMLFinder::Interfaces;
    else if (ui_categoryOperation->isChecked())
        return UMLFinder::Operations;
    else if (ui_categoryAttribute->isChecked())
        return UMLFinder::Attributes;
    else
        return UMLFinder::All;
}

/**
 * Handles filter radio button group click.
 * @param button (see enum ButtonID)
 */
void FindDialog::slotFilterButtonClicked(int button)
{
    ui_categoryOperation->setEnabled(button == TreeView);
    ui_categoryAttribute->setEnabled(button == TreeView);
    if (button != TreeView)
        ui_categoryAll->setChecked(true);
}

/**
 * Override default event handler for show event.
 * @param event
 */
void FindDialog::showEvent(QShowEvent *event)
{
    ui_searchTerm->setFocus();
    QDialog::showEvent(event);
}
