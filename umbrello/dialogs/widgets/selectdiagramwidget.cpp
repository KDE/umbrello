/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2019                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "selectdiagramwidget.h"

#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

#include <KComboBox>
#include <KLocalizedString>

#include <QLabel>
#include <QHBoxLayout>

/**
 * constructor
 * @param title title shown as label
 * @param parent parent widget
 */
SelectDiagramWidget::SelectDiagramWidget(const QString &title, QWidget *parent)
  : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    m_label = new QLabel(title, this);
    layout->addWidget(m_label);

    m_editField = new KComboBox(this);
    m_editField->setEditable(true);
    layout->addWidget(m_editField, 2);
    setLayout(layout);
}

/**
 * setup widget woth diagram type, currently used diagram, diagram name to exclude and an option to choose a new diagram
 * @param type
 * @param currentName
 * @param excludeName
 * @param withNewEntry
 */
void SelectDiagramWidget::setupWidget(Uml::DiagramType::Enum type, const QString &currentName, const QString &excludeName, bool withNewEntry)
{
    QStringList diagrams;

    foreach (UMLView *view, UMLApp::app()->document()->viewIterator()) {
        QString name = view->umlScene()->name();
        if (name == excludeName)
            continue;
        if (type == Uml::DiagramType::Undefined || type == view->umlScene()->type()) {
            diagrams << name;
            m_editField->completionObject()->addItem(name);
        }
    }
    diagrams.sort();
    if (withNewEntry)
        diagrams.push_front(i18n("New"));
    m_editField->clear();
    m_editField->insertItems(-1, diagrams);
    int currentIndex = m_editField->findText(currentName);
    if (currentIndex > -1) {
        m_editField->setCurrentIndex(currentIndex);
    }
}
/**
 * Add this widget to a given grid layout. Umbrello dialogs places labels in column 0
 * and the editable field in column 1.
 * @param layout The layout to which the widget should be added
 * @param row The row in the grid layout where the widget should be placed
 */
void SelectDiagramWidget::addToLayout(QGridLayout *layout, int row)
{
    layout->addWidget(m_label, row, 0);
    layout->addWidget(m_editField, row, 1);
}

/**
 * Return current text
 * @return The text entered by the user
 */
QString SelectDiagramWidget::currentText()
{
    return m_editField->currentText();
}

/**
 * Return the id of the currently selected diagram
 * @return id of diagram
 * @return Uml::ID::None in case the diagram was not found
 */
Uml::ID::Type SelectDiagramWidget::currentID()
{
    QString name = m_editField->currentText();
    foreach (UMLView *view, UMLApp::app()->document()->viewIterator()) {
        if (name == view->umlScene()->name())
            return view->umlScene()->ID();
    }
    return Uml::ID::None;
}
