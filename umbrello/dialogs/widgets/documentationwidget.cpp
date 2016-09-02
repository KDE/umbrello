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

#include "codetextedit.h"
#include "operation.h"
#include "umlobject.h"
#include "umlwidget.h"

#include <KTabWidget>
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
    if (m_object) {
        m_object->setDoc(m_editField->toPlainText());
        if (m_object->isUMLOperation()) {
            UMLOperation *o = m_object->asUMLOperation();
            o->setSourceCode(m_codeEditField->toPlainText());
        }
    }
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
    if (m_object && m_object->isUMLOperation()) {
        UMLOperation *o = m_object->asUMLOperation();
        m_codeEditField = new CodeTextEdit();
        m_codeEditField->setPlainText(o->getSourceCode());
#if QT_VERSION >= 0x050000
        QTabWidget* tabWidget = new QTabWidget();
#else
        KTabWidget* tabWidget = new KTabWidget();
#endif
        tabWidget->addTab(m_editField, i18n("Comment"));
        tabWidget->addTab(m_codeEditField, i18n("Source Code"));
        layout->addWidget(tabWidget);
    } else {
        layout->addWidget(m_editField);
    }
    layout->setMargin(fontMetrics().height());
    l->addWidget(m_box);
    setLayout(l);
}
