/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "finddialog.h"

FindDialog::FindDialog(QWidget *parent) :
    SinglePageDialogBase(parent)
{
    setCaption(i18n("Find"));
    setupUi(mainWidget());
    connect(ui_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotFilterButtonClicked(int)));

    setButtons(User1 | Cancel);
    setDefaultButton(User1);
    setButtonText(User1, i18n("Search"));
    ui_treeView->setChecked(true);
    ui_categoryAll->setChecked(true);
}

FindDialog::~FindDialog()
{
}

/**
 * return entered text.
 * @return text
 */
QString FindDialog::text()
{
    return ui_searchTerm->text();
}

/**
 * Return user selected filter.
 * @return filter enum
 */
UMLFinder::Filter FindDialog::filter()
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
UMLFinder::Category FindDialog::category()
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
 * Handles dialog button click.
 * @param button
 */
void FindDialog::slotButtonClicked(int button)
{
    if (button == KDialog::User1)
       accept();
    else
       KDialog::slotButtonClicked(button);
}

/**
 * Handles filter radio button group click.
 * @param button (-2=Treeview,-3,-4)
 */
void FindDialog::slotFilterButtonClicked(int button)
{
    ui_categoryOperation->setEnabled(button == -2);
    ui_categoryAttribute->setEnabled(button == -2);
    if (button != -2)
        ui_categoryAll->setChecked(true);
}

/**
 * Override default event handler for show event.
 * @param event
 */
void FindDialog::showEvent(QShowEvent *event)
{
    ui_searchTerm->setFocus();
    KDialog::showEvent(event);
}
