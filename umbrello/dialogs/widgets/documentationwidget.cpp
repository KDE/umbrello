/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "documentationwidget.h"

#include "associationwidget.h"
#include "codetextedit.h"
#include "operation.h"
#include "umlobject.h"
#include "umlwidget.h"

#include <QTextEdit>
#include <KLocalizedString>

#include <QGroupBox>
#include <QHBoxLayout>
#include <QTabWidget>

DocumentationWidget::DocumentationWidget(UMLObject *o, QWidget *parent) :
    QWidget(parent),
    m_object(o),
    m_widget(nullptr),
    m_assocWidget(nullptr)
{
    Q_ASSERT(o);
    init(o->doc());
}

DocumentationWidget::DocumentationWidget(UMLWidget *w, QWidget *parent) :
    QWidget(parent),
    m_object(nullptr),
    m_widget(w),
    m_assocWidget(nullptr)
{
    Q_ASSERT(w);
    init(w->documentation());
}

DocumentationWidget::DocumentationWidget(AssociationWidget *w, QWidget *parent) :
    QWidget(parent),
    m_object(nullptr),
    m_widget(nullptr),
    m_assocWidget(w)
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
    else if (m_widget) {
        m_widget->setDocumentation(m_editField->toPlainText());
        m_widget->updateGeometry();
    }
    else if (m_assocWidget) {
        m_assocWidget->setDocumentation(m_editField->toPlainText());
    }
}

/**
 * initialize widget
 * @param text text to display
 */
void DocumentationWidget::init(const QString &text)
{
    QHBoxLayout *l = new QHBoxLayout;
    l->setContentsMargins({});
    m_box = new QGroupBox;
    m_box->setTitle(i18n("Documentation"));
    m_editField = new QTextEdit(m_box);
    m_editField->setLineWrapMode(QTextEdit::WidgetWidth);
    m_editField->setWordWrapMode(QTextOption::WordWrap);
    m_editField->setText(text);
    setFocusProxy(m_editField);
    QHBoxLayout *layout = new QHBoxLayout(m_box);
    if (m_object && m_object->isUMLOperation()) {
        const UMLOperation *o = m_object->asUMLOperation();
        m_codeEditField = new CodeTextEdit();
        m_codeEditField->setPlainText(o->getSourceCode());
        QTabWidget* tabWidget = new QTabWidget();
        tabWidget->addTab(m_editField, i18n("Comment"));
        tabWidget->addTab(m_codeEditField, i18n("Source Code"));
        layout->addWidget(tabWidget);
    } else {
        layout->addWidget(m_editField);
    }
    int margin = fontMetrics().height();
    layout->setContentsMargins(margin, margin, margin, margin);
    l->addWidget(m_box);
    setLayout(l);
}
