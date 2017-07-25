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

DocumentationWidget::DocumentationWidget(QWidget *parent)
 : QWidget(parent),
   ui(new Ui::DocumentationWidget),
   m_widget(0)
{
    ui->setupUi(this);
}

DocumentationWidget::DocumentationWidget(WidgetBase *w, QWidget *parent)
 : QWidget(parent),
   ui(new Ui::DocumentationWidget),
   m_object(0),
   m_widget(w)
{
    ui->setupUi(this);
    Q_ASSERT(w);
    init(w->documentation());
}

DocumentationWidget::~DocumentationWidget()
{
}

/**
 * Apply changes to the related UMLObject.
 */
void DocumentationWidget::apply()
{
    if (m_object)
        m_object->setDoc(ui->docTE->toPlainText());
    else if (m_widget)
        m_widget->setDocumentation(ui->docTE->toPlainText());
}

void DocumentationWidget::setUMLObject(UMLObject *o)
{
    Q_ASSERT(o);
    m_object = o;
    init(o->doc());
}

/**
 * initialize widget
 * @param text text to display
 */
void DocumentationWidget::init(const QString &text)
{
    ui->docTE->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    ui->docTE->setWordWrapMode(QTextOption::WordWrap);
    ui->docTE->setPlainText(text);
    setFocusProxy(ui->docTE);
}
