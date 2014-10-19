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
    QDialog(parent)
{
    setupUi(window());

#if 0 //FIXME KF5
    setCaption(i18n("Find"));
    setButtons(User1 | Cancel);
    setDefaultButton(User1);
    setButtonText(User1, i18n("Search"));
    showButtonSeparator(true);
#endif
    setModal(true);
    ui_treeView->setChecked(true);
}

FindDialog::~FindDialog()
{
}

QString FindDialog::text()
{
    return ui_searchTerm->text();
}

FindDialog::Filter FindDialog::filter()
{
    if (ui_treeView->isChecked())
        return TreeView;
    else if (ui_CurrentDiagram->isChecked())
        return CurrentDiagram;
    else
        return AllDiagrams;
}

void FindDialog::slotButtonClicked(int button)
{
#if 0 //FIXME KF5
    if (button == QDialog::User1)
       accept();
    else
       QDialog::slotButtonClicked(button);
#endif
}

void FindDialog::showEvent(QShowEvent *event)
{
    ui_searchTerm->setFocus();
    QDialog::showEvent(event);
}
