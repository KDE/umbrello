/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "diagrampropertiespage.h"

// local includes
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

// qt includes

/**
 * Constructor
 * @param parent   the parent (wizard) of this wizard page
 * @param scene    the UMLScene to which the properties apply
 */
DiagramPropertiesPage::DiagramPropertiesPage(QWidget *parent, UMLScene *scene)
  : DialogPageBase(parent), m_scene(scene)
{
    setupUi(this);

    ui_diagramName->setText(scene->name());
    ui_zoom->setValue(scene->activeView()->currentZoom());

    ui_checkBoxShowGrid->setChecked(scene->isSnapGridVisible());
    ui_snapToGrid->setChecked(scene->snapToGrid());
    ui_snapComponentSizeToGrid->setChecked(scene->snapComponentSizeToGrid());

    ui_gridSpaceX->setValue(scene->snapX());
    ui_gridSpaceY->setValue(scene->snapY());
    ui_documentation->setText(scene->documentation());
    if (scene->type() == Uml::DiagramType::Sequence ||
        scene->type() == Uml::DiagramType::Collaboration) {
        ui_autoIncrementSequence->setVisible(true);
        ui_autoIncrementSequence->setChecked(scene->autoIncrementSequence());
    } else {
        ui_autoIncrementSequence->setVisible(false);
    }
}

/**
 * destructor
 */
DiagramPropertiesPage::~DiagramPropertiesPage()
{
}

/**
 * sets default values
 */
void DiagramPropertiesPage::setDefaults()
{
}

/**
 Checks whether the diagram name is unique and sets it if it is.
*/
bool DiagramPropertiesPage::checkUniqueDiagramName()
{
    // check name
    QString newName = ui_diagramName->text();
    if (newName.length() == 0) {
        KMessageBox::sorry(this, i18n("The name you have entered is invalid."),
                           i18n("Invalid Name"), 0);
        ui_diagramName->setText(m_scene->name());
        return false;
    }

    if (newName != m_scene->name()) {
        UMLDoc* doc = UMLApp::app()->document();
        UMLView* view = doc->findView(m_scene->type(), newName);
        if (view) {
            KMessageBox::sorry(this, i18n("The name you have entered is not unique."),
                               i18n("Name Not Unique"), 0);
            ui_diagramName->setText(m_scene->name());
        }
        else {
            // uDebug() << "Cannot find view with name " << newName;
            m_scene->setName(newName);
            doc->signalDiagramRenamed(m_scene->activeView());
            return true;
        }
    }
    return false;
}

/**
 * Reads the set values from their corresponding widgets, writes them back to
 * the data structure, and notifies clients.
 */
void DiagramPropertiesPage::apply()
{
    checkUniqueDiagramName();
    //:TODO: m_pScene->setZoom(m_diagramProperties->ui_zoom->value());
    m_scene->setDocumentation(ui_documentation->toPlainText());
    m_scene->setSnapSpacing(ui_gridSpaceX->value(), ui_gridSpaceY->value());
    m_scene->setSnapToGrid(ui_snapToGrid->isChecked());
    m_scene->setSnapComponentSizeToGrid(ui_snapComponentSizeToGrid->isChecked());
    m_scene->setSnapGridVisible(ui_checkBoxShowGrid->isChecked());
    if (m_scene->type() == Uml::DiagramType::Sequence ||
        m_scene->type() == Uml::DiagramType::Collaboration) {
        m_scene->setAutoIncrementSequence(ui_autoIncrementSequence->isChecked());
    }
    emit applyClicked();
}
