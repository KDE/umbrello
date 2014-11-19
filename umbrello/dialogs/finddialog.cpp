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

#include <KLocalizedString>

#include <QDialogButtonBox>
#include <QPushButton>

FindDialog::FindDialog(QWidget *parent)
  : QDialog(parent)
{
    setupUi(window());

    setWindowTitle(i18n("Find"));
    setModal(true);

    QDialogButtonBox* dlgButtonBox = new QDialogButtonBox();
    QPushButton* searchBtn = dlgButtonBox->addButton(i18n("Search"), QDialogButtonBox::AcceptRole);
    searchBtn->setDefault(true);
    QPushButton* cancelBtn = dlgButtonBox->addButton(i18n("Cancel"), QDialogButtonBox::RejectRole);
    cancelBtn->setAutoDefault(false);
    layout()->addWidget(dlgButtonBox);

    connect(searchBtn, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));

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
