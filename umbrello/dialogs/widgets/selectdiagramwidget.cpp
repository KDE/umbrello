/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2019-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
  : ComboBoxWidgetBase(title, QString(), parent)
{
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

    for(UMLView  *view : UMLApp::app()->document()->viewIterator()) {
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
    for(UMLView  *view : UMLApp::app()->document()->viewIterator()) {
        if (name == view->umlScene()->name())
            return view->umlScene()->ID();
    }
    return Uml::ID::None;
}
