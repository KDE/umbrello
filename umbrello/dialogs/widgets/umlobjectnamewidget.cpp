/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umlobjectnamewidget.h"

#include <QLineEdit>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

UMLObjectNameWidget::UMLObjectNameWidget(const QString &label, const QString &text, QWidget *parent)
    : QWidget(parent),
      m_text(text)
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    m_label = new QLabel(label, this);
    layout->addWidget(m_label);

    m_editField = new QLineEdit(this);
    layout->addWidget(m_editField, 2);
    m_editField->setText(text);

    m_label->setBuddy(m_editField);
    setLayout(layout);
    setFocusProxy(m_editField);
}

UMLObjectNameWidget::~UMLObjectNameWidget()
{
    delete m_editField;
    delete m_label;
}

/**
 * Add this widget to a given grid layout. Umbrello dialogs places labels in column 0
 * and the editable field in column 1.
 * @param layout The layout to which the widget should be added
 * @param row The row in the grid layout where the widget should be placed
 */
void UMLObjectNameWidget::addToLayout(QGridLayout *layout, int row)
{
    layout->addWidget(m_label, row, 0);
    layout->addWidget(m_editField, row, 1);
}

QString UMLObjectNameWidget::text()
{
    return m_editField->text();
}

void UMLObjectNameWidget::reset()
{
    m_editField->setText(m_text);
}
