/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "finddialog.h"

FindDialog::FindDialog(QWidget *parent) :
    SinglePageDialogBase(parent, false, true)
{
    setCaption(i18n("Find"));
    setupUi(mainWidget());
    connect(ui_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotFilterButtonClicked(int)));
    ui_treeView->setChecked(true);
    ui_categoryAll->setChecked(true);
}

FindDialog::~FindDialog()
{
    disconnect(ui_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotFilterButtonClicked(int)));
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
#if QT_VERSION >= 0x050000
    QDialog::showEvent(event);
#else
    KDialog::showEvent(event);
#endif
}
