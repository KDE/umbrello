/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "codegenselectpage.h"

// app includes
#include "folder.h"
#include "umldoc.h"
#include "uml.h"
#include "umlclassifier.h"
#include "entity.h"

//kde includes
#include <KLocalizedString>

//qt includes
#include <QListWidget>

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
    ui_listSelected->clear();

    if (classList == nullptr) {
        Uml::ProgrammingLanguage::Enum pl = UMLApp::app()->activeLanguage();
        switch (pl) {
            case Uml::ProgrammingLanguage::PostgreSQL:
            case Uml::ProgrammingLanguage::MySQL:
                for(UMLEntity* ent : doc->entities()) {
                    cList.append(ent);
                }
                break;
            default:
                cList = doc->classesAndInterfaces();
                break;
        }
        classList = &cList;
    }

    for(UMLClassifier* c : cList) {
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
    Q_EMIT completeChanged();
}

/**
 * Removes the classes selected in the selected classes list from the
 * list of classes used to generate the code.
 */
void CodeGenSelectPage::deselectClass()
{
    moveSelectedItems(ui_listSelected, ui_listAvailable);
    Q_EMIT completeChanged();
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
    for(QListWidgetItem* item : fromList->selectedItems()) {
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

