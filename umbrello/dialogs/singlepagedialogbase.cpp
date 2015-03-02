/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012-2015                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "singlepagedialogbase.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>

/**
 * Constructor
 */
SinglePageDialogBase::SinglePageDialogBase(QWidget *parent)
  : QDialog(parent)
{
    setModal(true);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                       QDialogButtonBox::Cancel);
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(slotOk()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(m_buttonBox);
}

SinglePageDialogBase::~SinglePageDialogBase()
{
}

/**
 * Apply dialog changes to the related object.
 */
bool SinglePageDialogBase::apply()
{
    return false;
}

void SinglePageDialogBase::setCaption(const QString &caption)
{
    setWindowTitle(caption);
}

void SinglePageDialogBase::setMainWidget(QWidget *widget)
{
    delete layout();
    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->setMargin(0);
    vlayout->addWidget(widget);
    vlayout->addWidget(m_buttonBox);
    setLayout(vlayout);
}

/**
 * Used when the Apply button is clicked. Calls apply().
 */
void SinglePageDialogBase::slotApply()
{
    apply();
}

/**
 * Used when the OK button is clicked. Calls apply().
 */
void SinglePageDialogBase::slotOk()
{
    if (apply()) {
        accept();
    }
}
