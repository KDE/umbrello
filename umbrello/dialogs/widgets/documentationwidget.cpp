/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "documentationwidget.h"

#include "umlobject.h"
#include "umlwidget.h"

#include <KTextEdit>
#include <KLocalizedString>

#include <QGroupBox>
#include <QHBoxLayout>

DocumentationWidget::DocumentationWidget(UMLObject *o, QWidget *parent) :
    QWidget(parent),
    m_object(o),
    m_widget(0)
{
    Q_ASSERT(o);
    init(o->doc());
}

DocumentationWidget::DocumentationWidget(UMLWidget *w, QWidget *parent) :
    QWidget(parent),
    m_object(0),
    m_widget(w)
{
    Q_ASSERT(w);
    init(w->documentation());
}

DocumentationWidget::~DocumentationWidget()
{
    delete m_editField;
    delete m_box;
}

/**
 * Apply changes to the related UMLObject.
 */
void DocumentationWidget::apply()
{
    if (m_object)
        m_object->setDoc(m_editField->toPlainText());
    else if (m_widget)
        m_widget->setDocumentation(m_editField->toPlainText());
}

/**
 * initialize widget
 * @param text text to display
 */
void DocumentationWidget::init(const QString &text)
{
    QHBoxLayout *l = new QHBoxLayout;
    m_box = new QGroupBox;
    m_box->setTitle(i18n("Documentation"));
    m_editField = new KTextEdit(m_box);
    m_editField->setLineWrapMode(QTextEdit::WidgetWidth);
    m_editField->setWordWrapMode(QTextOption::WordWrap);
    m_editField->setText(text);
    setFocusProxy(m_editField);

    QHBoxLayout *layout = new QHBoxLayout(m_box);
    layout->addWidget(m_editField);
    layout->setMargin(fontMetrics().height());
    l->addWidget(m_box);
    setLayout(l);
}
