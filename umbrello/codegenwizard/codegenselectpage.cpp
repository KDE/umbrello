/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "codegenselectpage.h"

//qt includes
#include <QtGui/QListWidget>

//kde includes
#include <klocale.h>

// app includes
#include "folder.h"
#include "umldoc.h"
#include "uml.h"
#include "classifier.h"
#include "entity.h"

/**
 * Constructor. 
 * @param parent   the parent (wizard) of this wizard page
 */
CodeGenSelectPage::CodeGenSelectPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(i18n("Select Classes"));
    setSubTitle(i18n("Place all the classes you want to generate code\nfor in the right hand side list."));

    setupUi(this);

    connect(ui_addButton, SIGNAL(clicked()), this, SLOT(selectClass()));
    connect(ui_removeButton, SIGNAL(clicked()), this, SLOT(deselectClass()));
}

/**
 * Destructor.
 */
CodeGenSelectPage::~CodeGenSelectPage()
{
}

/**
 * Loads the available classes for selection / deselection
 * into the list widget.
 * @param classList   the available classes for generation
 */
void CodeGenSelectPage::setClassifierList(UMLClassifierList *classList)
{
    UMLDoc* doc = UMLApp::app()->document();

    UMLClassifierList cList;

    if (classList == NULL) {
        UMLFolder* currRoot = doc->currentRoot();
        Uml::ModelType type = doc->rootFolderType(currRoot);

        switch (type) {
            case Uml::ModelType::Logical:
                cList = doc->classesAndInterfaces();
                break;
            case Uml::ModelType::EntityRelationship:
                foreach (UMLEntity* ent, doc->entities()) {
                    cList.append(ent);
                }
                break;
           default:
                break;
        }
        classList = &cList;
    }

    foreach (UMLClassifier* c, cList) {
        new QListWidgetItem(c->fullyQualifiedName(), ui_listSelected);
    }
}

/**
 * Reimplemented QWizardPage method the enable / disable the next button.
 * @return   complete state 
 */
bool CodeGenSelectPage::isComplete() const
{
    bool completed = false;
    if (ui_listSelected->count() > 0) {
        completed = true;
    }
    return completed;
}

/**
 * Returns the list widget, which holds the classes for generation.
 * @return   the list widget of the selected classes
 */
QListWidget* CodeGenSelectPage::getSelectionListWidget()
{
    return ui_listSelected;
}

/**
 * Adds the classes selected in the available classes list to the
 * list of classes used to generate the code.
 */
void CodeGenSelectPage::selectClass()
{
    moveSelectedItems(ui_listAvailable, ui_listSelected);
    emit completeChanged();
}

/**
 * Removes the classes selected in the selected classes list from the
 * list of classes used to generate the code.
 */
void CodeGenSelectPage::deselectClass()
{
    moveSelectedItems(ui_listSelected, ui_listAvailable);
    emit completeChanged();
}

/**
 * Moves the selected items from first list to second list.
 * The selected items are removed from the first list and added to the
 * second. An item is added to the second list only if it isn't already
 * there (no duplicated items are created).
 * @param fromList   the list widget of selected items
 * @param toList     the target list widget 
 */
void CodeGenSelectPage::moveSelectedItems(QListWidget* fromList, QListWidget* toList)
{
    foreach (QListWidgetItem* item, fromList->selectedItems()) {
        QString name = item->text();
        QList<QListWidgetItem*> foundItems = toList->findItems(name, Qt::MatchExactly);
        if (foundItems.isEmpty()) {
            new QListWidgetItem(name, toList);
        }
        // Removed here because it can't (really, shouldn't) be removed while
        // iterator is pointing to it
        fromList->takeItem(fromList->row(item));
    }
}

#include "codegenselectpage.moc"
