/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2019                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "comboboxwidgetbase.h"

#include <KComboBox>

#include <QHBoxLayout>
#include <QLabel>

ComboBoxWidgetBase::ComboBoxWidgetBase(const QString &title, const QString &postLabel, QWidget *parent)
  : QWidget(parent)
  , m_postLabel(nullptr)
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    m_label = new QLabel(title, this);
    layout->addWidget(m_label);

    m_editField = new KComboBox(this);
    m_editField->setEditable(true);
    m_editField->setDuplicatesEnabled(false);  // only allow one of each type in box
#if QT_VERSION < 0x050000
    m_editField->setCompletionMode(KGlobalSettings::CompletionPopup);
#endif
    layout->addWidget(m_editField, 2);
    m_label->setBuddy(m_editField);

    if (!postLabel.isEmpty()) {
        m_postLabel = new QLabel(postLabel, this);
        layout->addWidget(m_postLabel);
    }
    setLayout(layout);
    setFocusProxy(m_editField);
}

/**
 * Add this widget to a given grid layout. Umbrello dialogs places labels in column 0
 * and the editable field in column 1.
 * @param layout The layout to which the widget should be added
 * @param row The row in the grid layout where the widget should be placed
 * @param startColumn The first column in the grid layout where the widget should be placed
 */
void ComboBoxWidgetBase::addToLayout(QGridLayout *layout, int row, int startColumn)
{
    layout->addWidget(m_label, row, startColumn);
    layout->addWidget(m_editField, row, startColumn + 1);
    if (m_postLabel)
        layout->addWidget(m_postLabel, row, startColumn + 2);
}
