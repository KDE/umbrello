/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003 Luis De la Parra <lparrab@gmx.net>                 *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "refactoringassistant.h"

#include "attribute.h"
#include "basictypes.h"
#include "classifier.h"
#include "classpropdlg.h"
#include "debug_utils.h"
#include "object_factory.h"
#include "operation.h"
#include "umlattributedialog.h"
#include "umldoc.h"
#include "umloperationdialog.h"

#include <klocale.h>
#include <kmessagebox.h>

#include <QtGui/QApplication>
#include <QtGui/QMenu>
#include <QtCore/QPoint>

/**
 * Constructor.
 * @param doc      the UML document
 * @param obj      the UML classifier to refactor
 * @param parent   the parent widget
 * @param name     the object name
 */
RefactoringAssistant::RefactoringAssistant(UMLDoc *doc, UMLClassifier *obj, QWidget *parent, const QString &name)
  : QTreeWidget(parent),
    m_doc(doc)
{
    setObjectName(name);
    setRootIsDecorated(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    setHeaderLabel(i18n("Name"));
    setContextMenuPolicy(Qt::CustomContextMenu);

    m_menu = new QMenu(this);

    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
            this, SLOT(itemExecuted(QTreeWidgetItem*, int)));
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));

    resize(300, 400);
    refactor(obj);
}

/**
 * Destructor.
 */
RefactoringAssistant::~RefactoringAssistant()
{
    m_umlObjectMap.clear();
    clear();
}

/**
 * Builds up the tree for the classifier.
 * @param obj   the classifier which has to be refactored
 */
void RefactoringAssistant::refactor(UMLClassifier *obj)
{
    clear();
    m_umlObjectMap.clear();
    m_umlObject = obj;
    if (! m_umlObject) {
        return;
    }
    uDebug() << "called for " << m_umlObject->name();

    addClassifier(obj, 0, true, true, true);
    QTreeWidgetItem *item = topLevelItem(0);
    item->setExpanded(true);
    for (int i = 0; i < item->childCount(); ++i) {
        item->setExpanded(true);
    }
}

/**
 * Find UML object from tree item.
 * @param item   the item from the tree widget
 * @return       the UML object behind the item
 */
UMLObject* RefactoringAssistant::findUMLObject(const QTreeWidgetItem *item)
{
    if (!item) {
        return 0;
    }
    QTreeWidgetItem *i = const_cast<QTreeWidgetItem*>(item);
    if (m_umlObjectMap.find(i) == m_umlObjectMap.end()) {
        uWarning() << "Item with text " << item->text(0) << "not found in uml map!";
        return 0;
    }
    return m_umlObjectMap[i];
}

/**
 * Find tree item from UML object.
 * @param obj   the UML object to search in tree
 * @return      the found tree widget item or 0
 */
QTreeWidgetItem* RefactoringAssistant::findListViewItem(const UMLObject *obj)
{
    QMapIterator<QTreeWidgetItem*, UMLObject*> it(m_umlObjectMap);
    while (it.hasNext()) {
        it.next();
        if (it.value() == obj) {
            return it.key();
        }
    }
    uWarning() << "Object id " << ID2STR(obj->id()) << "does not have an item in the tree";
    return 0;
}

/**
 * Slot for double clicking on a tree widget item.
 * @param item     tree widget item on which the user clicked
 * @param column   the column of the tree on which the user clicked.
 */
void RefactoringAssistant::itemExecuted(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    UMLObject *obj = findUMLObject(item);
    if (obj) {
        editProperties();
    }
}

/**
 * Set the icon representing the visibility of the given item.
 * @param item   the tree item
 * @param obj    the UML object behind the tree item
 */
void RefactoringAssistant::setVisibilityIcon(QTreeWidgetItem *item , const UMLObject *obj)
{
    UMLObject::ObjectType t = obj->baseType();
    switch (obj->visibility()) {
    case Uml::Visibility::Public:
        if (t == UMLObject::ot_Operation) {
            item->setIcon(0, Icon_Utils::SmallIcon(Icon_Utils::it_Public_Method));
        }
        else {
            item->setIcon(0, Icon_Utils::SmallIcon(Icon_Utils::it_Public_Attribute));
        }
        break;
    case Uml::Visibility::Protected:
        if (t == UMLObject::ot_Operation) {
            item->setIcon(0, Icon_Utils::SmallIcon(Icon_Utils::it_Protected_Method));
        }
        else {
            item->setIcon(0, Icon_Utils::SmallIcon(Icon_Utils::it_Protected_Attribute));
        }
        break;
    case Uml::Visibility::Private:
        if (t == UMLObject::ot_Operation) {
            item->setIcon(0, Icon_Utils::SmallIcon(Icon_Utils::it_Private_Method));
        }
        else {
            item->setIcon(0, Icon_Utils::SmallIcon(Icon_Utils::it_Private_Attribute));
        }
        break;
    case Uml::Visibility::Implementation:
        if (t == UMLObject::ot_Operation) {
            item->setIcon(0, Icon_Utils::SmallIcon(Icon_Utils::it_Implementation_Method));
        }
        else {
            item->setIcon(0, Icon_Utils::SmallIcon(Icon_Utils::it_Implementation_Attribute));
        }
        break;
    default:
        break;
    }
}

/**
 * Slot for updating the tree item properties according to the given UML object.
 * If no parameter is given the sender is used.
 * @param obj   the UML object
 */
void RefactoringAssistant::objectModified(const UMLObject *obj)
{
    if (!obj) {
        obj = dynamic_cast<const UMLObject*>(sender());
    }
    QTreeWidgetItem *item = findListViewItem(obj);
    if (!item) {
        return;
    }
    item->setText(0, obj->name());
    if (typeid(*obj) == typeid(UMLOperation) ||
            typeid(*obj) == typeid(UMLAttribute)) {
        setVisibilityIcon(item, obj);
    }
}

/**
 * Slot for adding an operation to the tree.
 * @param listItem   the new operation to add
 */
void RefactoringAssistant::operationAdded(UMLClassifierListItem *listItem)
{
    UMLOperation *op = static_cast<UMLOperation*>(listItem);
    uDebug() << "operation = " << op->name();  //:TODO:
    UMLClassifier *parent = dynamic_cast<UMLClassifier*>(op->parent());
    if (!parent) {
        uWarning() << op->name() << " - Parent of operation is not a classifier!";
        return;
    }
    QTreeWidgetItem *item = findListViewItem(parent);
    if (!item) {
        return;
    }
    for (int i = 0; i < item->childCount(); ++i) {
        QTreeWidgetItem *folder = item->child(i);
        if (folder->text(1) == "operations") {
            item = new QTreeWidgetItem(folder, QStringList(op->name()));
            m_umlObjectMap[item] = op;
            connect(op, SIGNAL(modified()), this, SLOT(objectModified()));
            setVisibilityIcon(item, op);
            uDebug() << "operation = " << op->name() << " added!";  //:TODO:
            break;
        }
    }
}

/**
 * Slot for removing an operation from the tree.
 * @param listItem   the operation to be removed
 */
void RefactoringAssistant::operationRemoved(UMLClassifierListItem *listItem)
{
    UMLOperation *op = static_cast<UMLOperation*>(listItem);
    QTreeWidgetItem *item = findListViewItem(op);
    if (!item) {
        return;
    }
    disconnect(op, SIGNAL(modified()), this, SLOT(objectModified()));
    m_umlObjectMap.remove(item);
    delete item;
}

/**
 * Slot for adding an attribute to the tree.
 * @param listItem   the new attribute to add
 */
void RefactoringAssistant::attributeAdded(UMLClassifierListItem *listItem)
{
    UMLAttribute *att = static_cast<UMLAttribute*>(listItem);
    uDebug() << "attribute = " << att->name();  //:TODO:
    UMLClassifier *parent = dynamic_cast<UMLClassifier*>(att->parent());
    if (!parent) {
        uWarning() << att->name() << " - Parent of attribute is not a classifier!";
        return;
    }
    QTreeWidgetItem *item = findListViewItem(parent);
    if (!item) {
        uWarning() << "Parent is not in tree!";
        return;
    }
    for (int i = 0; i < item->childCount(); ++i) {
        QTreeWidgetItem *folder = item->child(i);
        if (folder->text(1) == "attributes") {
            item = new QTreeWidgetItem(folder, QStringList(att->name()));
            m_umlObjectMap[item] = att;
            connect(att, SIGNAL(modified()), this, SLOT(objectModified()));
            setVisibilityIcon(item, att);
            uDebug() << "attribute = " << att->name() << " added!";  //:TODO:
            break;
        }
    }
}

/**
 * Slot for removing an attribute from the tree.
 * @param listItem   the attribute to be removed
 */
void RefactoringAssistant::attributeRemoved(UMLClassifierListItem *listItem)
{
    UMLAttribute *att = static_cast<UMLAttribute*>(listItem);
    uDebug() << "attribute = " << att->name();  //:TODO:
    QTreeWidgetItem *item = findListViewItem(att);
    if (!item) {
        uWarning() << "Attribute is not in tree!";
        return;
    }
    disconnect(att, SIGNAL(modified()), this, SLOT(objectModified()));
    m_umlObjectMap.remove(item);
    delete item;
    uDebug() << "attribute = " << att->name() << " deleted!";  //:TODO:
}

/**
 * Slot for calling editProperties with the current item.
 */
void RefactoringAssistant::editProperties()
{
    QTreeWidgetItem* item = currentItem();
    if (item) {
        UMLObject* obj = findUMLObject(item);
        if (obj) {
            editProperties(obj);
        }
    }
}

/**
 * Show the dialog with data from the given UML object.
 * @param obj   the UML object to edit
 */
void RefactoringAssistant::editProperties(UMLObject *obj)
{
    KDialog *dia(0);
    UMLObject::ObjectType t = obj->baseType();
    if (t == UMLObject::ot_Class || t == UMLObject::ot_Interface) {
        dia = new ClassPropDlg(this, obj, true);
    }
    else if (t == UMLObject::ot_Operation) {
        dia = new UMLOperationDialog(this, static_cast<UMLOperation*>(obj));
    }
    else if (t == UMLObject::ot_Attribute) {
        dia = new UMLAttributeDialog(this, static_cast<UMLAttribute*>(obj));
    }
    else {
        uWarning() << "Called for unknown type " << typeid(*obj).name();
        return;
    }
    if (dia && dia->exec()) {
        // need to update something?
    }
    delete dia;
}

/**
 * Slot for deleting an item called from the popup menu.
 */
void RefactoringAssistant::deleteItem()
{
    QTreeWidgetItem *item = currentItem();
    if (item) {
        UMLObject *o = findUMLObject(item);
        if (o) {
            deleteItem(item, o);
        }
    }
}

/**
 * Delete an item from the tree.
 * @param item   the tree widget item
 * @param obj    the uml object
 */
void RefactoringAssistant::deleteItem(QTreeWidgetItem *item, UMLObject *obj)
{
    UMLObject::ObjectType t = obj->baseType();
    if (t == UMLObject::ot_Class || t == UMLObject::ot_Interface) {
        uDebug() << "Delete class or interface - not yet implemented!";  //:TODO:
    }
    else if (t == UMLObject::ot_Operation) {
        QTreeWidgetItem *opNode = item->parent();
        if (opNode) {
            QTreeWidgetItem *parent = opNode->parent();
            UMLClassifier* c = static_cast<UMLClassifier*>(findUMLObject(parent));
            if (!c) {
                uWarning() << "No classifier - cannot delete!";
                return;
            }
            UMLOperation* op = static_cast<UMLOperation*>(obj);
            c->removeOperation(op);
        }
    }
    else if (t == UMLObject::ot_Attribute) {
        QTreeWidgetItem *attrNode = item->parent();
        if (attrNode) {
            QTreeWidgetItem *parent = attrNode->parent();
            UMLClassifier* c = static_cast<UMLClassifier*>(findUMLObject(parent));
            if (!c) {
                uWarning() << "No classifier - cannot delete!";
                return;
            }
            UMLAttribute* attr = static_cast<UMLAttribute*>(obj);
            c->removeAttribute(attr);
        }
    }
    else {
        uWarning() << "Called for unknown type " << typeid(*obj).name();
    }
}

/**
 * Create an action for an entry in the context menu.
 * @param text     the text of the action
 * @param method   the method to call when triggered
 * @param icon     the shown icon
 * @return         the created action
 */
QAction* RefactoringAssistant::createAction(const QString& text, const char * method, const Icon_Utils::IconType icon)
{
    QAction* action = new QAction(this);
    action->setText(text);
    if (icon != Icon_Utils::N_ICONTYPES) {
        action->setIcon(Icon_Utils::SmallIcon(icon));
    }
    connect(action, SIGNAL(triggered()), this, method);
    return action;
}

/**
 * Slot for the context menu by right clicking in the tree widget.
 * @param p   point of the right click inside the tree widget
 */
void RefactoringAssistant::showContextMenu(const QPoint& p)
{
    QTreeWidgetItem* item = itemAt(p);
    if (!item) {
        return;
    }
    m_menu->clear();
    UMLObject *obj = findUMLObject(item);
    if (obj) { // Menu for UMLObjects
        UMLObject::ObjectType t = obj->baseType();
        if (t == UMLObject::ot_Class) {
            m_menu->addAction(createAction(i18n("Add Base Class"), SLOT(addBaseClassifier()), Icon_Utils::it_Generalisation));
            m_menu->addAction(createAction(i18n("Add Derived Class"), SLOT(addDerivedClassifier()), Icon_Utils::it_Uniassociation));
            // m_menu->addAction(createAction(i18n("Add Interface Implementation"), SLOT(addInterfaceImplementation()), Icon_Utils::it_Implementation));
            m_menu->addAction(createAction(i18n("Add Operation"), SLOT(createOperation()), Icon_Utils::it_Public_Method));
            m_menu->addAction(createAction(i18n("Add Attribute"), SLOT(createAttribute()), Icon_Utils::it_Public_Attribute));
        }
        else if (t == UMLObject::ot_Interface) {
            m_menu->addAction(createAction(i18n("Add Base Interface"), SLOT(addSuperClassifier()), Icon_Utils::it_Generalisation));
            m_menu->addAction(createAction(i18n("Add Derived Interface"), SLOT(addDerivedClassifier()), Icon_Utils::it_Uniassociation));
            m_menu->addAction(createAction(i18n("Add Operation"), SLOT(createOperation()), Icon_Utils::it_Public_Method));
        }
        // else {
        //     uDebug() << "No context menu for objects of type " << typeid(*obj).name();
        //     return;
        // }
        m_menu->addSeparator();
        m_menu->addAction(createAction(i18n("Properties"), SLOT(editProperties()), Icon_Utils::it_Properties));
        m_menu->addAction(createAction(i18n("Delete"), SLOT(deleteItem()), Icon_Utils::it_Delete));
    }
    else { //menu for other ViewItems
        if (item->text(1) == "operations") {
            m_menu->addAction(createAction(i18n("Add Operation"), SLOT(createOperation()), Icon_Utils::it_Public_Method));
        }
        else if (item->text(1) == "attributes") {
            m_menu->addAction(createAction(i18n("Add Attribute"), SLOT(createAttribute()), Icon_Utils::it_Public_Attribute));
        }
        else {
            uWarning() << "Called for unsupported item.";
            return;
        }
    }
    m_menu->exec(mapToGlobal(p) + QPoint(0, 20));
}

/**
 * Slot for adding a base classifier.
 */
void RefactoringAssistant::addBaseClassifier()
{
    QTreeWidgetItem *item = currentItem();
    if (!item) {
        uWarning() << "Called with no item selected";
        return;
    }
    UMLObject *obj = findUMLObject(item);
    if (!dynamic_cast<UMLClassifier*>(obj)) {
        uWarning() << "Called for a non-classifier object.";
        return;
    }

    //classes have classes and interfaces interfaces as super/derived classifiers
    UMLObject::ObjectType t = obj->baseType();
    UMLClassifier *super = static_cast<UMLClassifier*>(Object_Factory::createUMLObject(t));
    if (!super) {
        return;
    }
    m_doc->createUMLAssociation(obj, super, Uml::AssociationType::Generalization);

    //////////////////////   Manually add the classifier to the assitant - would be nicer to do it with
    /////////////////////    a signal, like operations and attributes
    QTreeWidgetItem *baseFolder = 0;
    for (int i = 0; i < item->childCount(); ++i) {
        baseFolder = item->child(i);
        if (!baseFolder) {
            uWarning() << "Cannot find base folder!";
            return;
        }
        if (baseFolder->text(0) == i18n("Base Classifiers")) {
            item = new QTreeWidgetItem(baseFolder, QStringList(super->name()));
            item->setIcon(0, Icon_Utils::SmallIcon(Icon_Utils::it_Generalisation));
            item->setExpanded(true);
            m_umlObjectMap[item] = super;
            addClassifier(super, item, true, false, true);
            break;
        }
    }
    /////////////////////////
}

/**
 * Slot for adding a derived classifier.
 */
void RefactoringAssistant::addDerivedClassifier()
{
    QTreeWidgetItem *item = currentItem();
    if (!item) {
        uWarning() << "Called with no item selected.";
        return;
    }
    UMLObject *obj = findUMLObject(item);
    if (!dynamic_cast<UMLClassifier*>(obj)) {
        uWarning() << "Called for a non-classifier object.";
        return;
    }

    //classes have classes and interfaces have interfaces as super/derived classifiers
    UMLObject::ObjectType t = obj->baseType();
    UMLClassifier *derived = static_cast<UMLClassifier*>(Object_Factory::createUMLObject(t));
    if (!derived) {
        return;
    }
    m_doc->createUMLAssociation(derived, obj, Uml::AssociationType::Generalization);

    //////////////////////   Manually add the classifier to the assitant - would be nicer to do it with
    /////////////////////    a signal, like operations and attributes
    QTreeWidgetItem *derivedFolder = 0;
    for (int i = 0; i < item->childCount(); ++i) {
        derivedFolder = item->child(i);
        if (!derivedFolder) {
            uWarning() << "Cannot find derived folder!";
            return;
        }
        if (derivedFolder->text(0) == i18n("Derived Classifiers")) {
            item = new QTreeWidgetItem(derivedFolder, QStringList(derived->name()));
            item->setIcon(0, Icon_Utils::SmallIcon(Icon_Utils::it_Uniassociation));
            item->setExpanded(true);
            m_umlObjectMap[item] = derived;
            addClassifier(derived, item, false, true, true);
        }
    }
    /////////////////////////
}

/**
 * Slot for adding an interface implementation.
 * @todo   not yet implemented, needs addSuperClassifier() first
 */
void RefactoringAssistant::addInterfaceImplementation()
{
    uWarning() << "Not implemented... finish addSuperClassifier() first!!";
    return;
    //  QTreeWidgetItem *item = selectedListViewItem();
    //  UMLObject *obj = findUMLObject( item );
    //  if( !dynamic_cast<UMLClassifier*>(obj) )
    //          return;
    //  UMLObject *n = Object_Factory::createUMLObject( UMLObject::ot_Interface) );
    //  if (!n) {
    //      return;
    //  }
    //  m_doc->createUMLAssociation( n, obj, UMLObject::at_Realization );
    //  //refresh, add classifier to assistant
}

/**
 * Create new operation.
 */
void RefactoringAssistant::createOperation()
{
    QTreeWidgetItem *item = currentItem();
    if (!item) {
        uWarning() << "Called with no item selected.";
        return;
    }
    UMLClassifier *c = dynamic_cast<UMLClassifier*>(findUMLObject(item));
    if (!c) {  // find parent
        QTreeWidgetItem *parent = item->parent();
        c = dynamic_cast<UMLClassifier*>(findUMLObject(parent));
        if (!c) {
            uWarning() << "No classifier - cannot create!";
            return;
        }
    }
    c->createOperation();
}

/**
 * Create new attribute.
 */
void RefactoringAssistant::createAttribute()
{
    QTreeWidgetItem *item = currentItem();
    if (!item) {
        uWarning() << "Called with no item selected.";
        return;
    }
    UMLClassifier *c = dynamic_cast<UMLClassifier*>(findUMLObject(item));
    if (!c) {  // find parent
        QTreeWidgetItem *parent = item->parent();
        c = dynamic_cast<UMLClassifier*>(findUMLObject(parent));
        if (!c) {
            uWarning() << "No classifier - can not create!";
            return;
        }
    }
    c->createAttribute();
}

/**
 * Add a classifier to the data structure.
 * @param classifier   the classifier to add
 * @param parent       the tree item under which the classifier is placed
 * @param addSuper     add it to the base classifier folder
 * @param addSub       add it to the derived classifier folder
 * @param recurse      ...
 */
void RefactoringAssistant::addClassifier(UMLClassifier *classifier, QTreeWidgetItem *parent, bool addSuper, bool addSub, bool recurse)
{
    if (!classifier) {
        uWarning() << "No classifier given - do nothing!";
        return;
    }
    uDebug() << classifier->name() << " added.";
    QTreeWidgetItem *classifierItem, *item;
    if (parent) {
        classifierItem = parent;
    }
    else {
        classifierItem = new QTreeWidgetItem(this, QStringList(classifier->name()));
        m_umlObjectMap[classifierItem] = classifier;
    }

    connect(classifier, SIGNAL(modified()),
            this, SLOT(objectModified()));

    // add attributes
    connect(classifier, SIGNAL(attributeAdded(UMLClassifierListItem*)),
            this, SLOT(attributeAdded(UMLClassifierListItem*)));
    connect(classifier, SIGNAL(attributeRemoved(UMLClassifierListItem*)),
            this, SLOT(attributeRemoved(UMLClassifierListItem*)));

    QStringList itemTextAt;
    itemTextAt << i18n("Attributes") << "attributes";
    QTreeWidgetItem *attsFolder = new QTreeWidgetItem(classifierItem, itemTextAt);
    attsFolder->setIcon(0, Icon_Utils::SmallIcon(Icon_Utils::it_Folder_Orange));
    attsFolder->setExpanded(true);
    UMLAttributeList atts(classifier->getAttributeList());
    foreach(UMLAttribute* att, atts) {
        attributeAdded(att);
    }

    // add operations
    connect(classifier, SIGNAL(operationAdded(UMLClassifierListItem*)),
            this, SLOT(operationAdded(UMLClassifierListItem*)));
    connect(classifier, SIGNAL(operationRemoved(UMLClassifierListItem*)),
            this, SLOT(operationRemoved(UMLClassifierListItem*)));

    QStringList itemTextOp;
    itemTextOp << i18n("Operations") << "operations";
    QTreeWidgetItem *opsFolder = new QTreeWidgetItem(classifierItem, itemTextOp);
    opsFolder->setIcon(0, Icon_Utils::SmallIcon(Icon_Utils::it_Folder_Orange));
    opsFolder->setExpanded(true);
    UMLOperationList ops(classifier->getOpList());
    foreach(UMLOperation* op, ops) {
        operationAdded(op);
    }

    if (addSuper) {  // add base classifiers
        QTreeWidgetItem *superFolder = new QTreeWidgetItem(classifierItem, QStringList(i18n("Base Classifiers")));
        superFolder->setExpanded(true);
        UMLClassifierList super = classifier->findSuperClassConcepts();
        foreach(UMLClassifier* cl, super) {
            item = new QTreeWidgetItem(superFolder, QStringList(cl->name()));
            item->setIcon(0, Icon_Utils::SmallIcon(Icon_Utils::it_Generalisation));
            item->setExpanded(true);
            m_umlObjectMap[item] = cl;
            if (recurse) {
                addClassifier(cl, item, true, false, true);
            }
        }
    }

    if (addSub) {  // add derived classifiers
        QTreeWidgetItem *derivedFolder = new QTreeWidgetItem(classifierItem, QStringList(i18n("Derived Classifiers")));
        derivedFolder->setExpanded(true);
        UMLClassifierList derived = classifier->findSubClassConcepts();
        foreach(UMLClassifier* d, derived) {
            item = new QTreeWidgetItem(derivedFolder, QStringList(d->name()));
            item->setIcon(0, Icon_Utils::SmallIcon(Icon_Utils::it_Uniassociation));
            item->setExpanded(true);
            m_umlObjectMap[item] = d;
            if (recurse) {
                addClassifier(d, item, false, true, true);
            }
        }
    }
}

/**
 * Reimplementation of the drag move event.
 * @param event   the drag move event
 */
void RefactoringAssistant::dragMoveEvent(QDragMoveEvent *event)
 {
    //first check if we can accept dropping
    if (event->source() == this) {
        event->setDropAction(Qt::MoveAction);
        QTreeWidgetItem* target = itemAt(event->pos());
        QTreeWidgetItem* item = currentItem();
        if (target && item) {
            //first check if we can accept dropping
            QTreeWidgetItem* parent = item->parent();
            if (parent) {
                if ((target->text(1) == "operations") &&
                    (parent->text(1) == "operations")) {
uDebug() << "accept operation " << item->text(0);  //:TODO:fischer
                    event->accept();
                    return;
                }
                if ((target->text(1) == "attributes") &&
                    (parent->text(1) == "attributes")) {
uDebug() << "accept attribute " << item->text(0);  //:TODO:fischer
                    event->accept();
                    return;
                }
            }
        }
        event->ignore();
    } else {
        event->acceptProposedAction();
    }
}

/**
 * Reimplementation of the drop event.
 * @param event   the drop event
 */
void RefactoringAssistant::dropEvent(QDropEvent *event)
{
    QTreeWidgetItem* movingItem = currentItem();
    if (!movingItem) {
        event->ignore();
        return;  // no item ?
    }
    uDebug() << "dropping=" << movingItem->text(0);

    if (event->source() == this) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
uDebug() << "accept";  //:TODO:fischer
    }
    else {
        event->acceptProposedAction();
uDebug() << "acceptProposedAction";  //:TODO:fischer
        return;
    }

    QTreeWidgetItem* afterme = itemAt(event->pos());
    if (!afterme) {
        uWarning() << "Drop target not found - aborting drop!";
        return;
    }
    uDebug() << "Dropping after item = " << afterme->text(0);  //:TODO:fischer

    // when dropping on a class, we have to put the item in the appropriate folder!
    UMLObject *movingObject;
    UMLClassifier *newClassifier;

    if ((movingItem == afterme) || !(movingObject = findUMLObject(movingItem))) {
        uWarning() << "Moving item not found or dropping after itself or item not found in uml obj map. aborting. (drop had already been accepted)";
        return;
    }
    QTreeWidgetItem* parentItem = afterme->parent();
    UMLObject::ObjectType t = movingObject->baseType();
    newClassifier = dynamic_cast<UMLClassifier*>(findUMLObject(parentItem));
    if (!newClassifier) {
        if ((parentItem->text(1) == "operations" && t == UMLObject::ot_Operation)
                || (parentItem->text(1) == "attributes" && t == UMLObject::ot_Attribute)) {
            newClassifier = dynamic_cast<UMLClassifier*>(findUMLObject(parentItem->parent()));
        }
        if (!newClassifier) {
            uWarning() << "New parent of object is not a Classifier - Drop had already been accepted - check!";
            return;
        }
    }
    if (t == UMLObject::ot_Operation) {
        uDebug() << "Moving operation";
        UMLOperation *op = static_cast<UMLOperation*>(movingObject);
        if (newClassifier->checkOperationSignature(op->name(), op->getParmList())) {
            QString msg = i18n("An operation with that signature already exists in %1.\n", newClassifier->name())
                          +
                          i18n("Choose a different name or parameter list.");
            KMessageBox::error(this, msg, i18n("Operation Name Invalid"), 0);
            return;
        }
        UMLOperation* newOp = static_cast<UMLOperation*>(op->clone());

        UMLClassifier *oldClassifier = dynamic_cast<UMLClassifier*>(op->parent());
        if (oldClassifier) {
            oldClassifier->removeOperation(op);
        }

        newClassifier->addOperation(newOp);
        m_doc->signalUMLObjectCreated(newOp);  //:TODO: really?
uDebug() << "oldClassifier=" << oldClassifier->name() << " / newClassifier=" << newClassifier->name();  //:TODO:fischer
    }
    else if (t == UMLObject::ot_Attribute) {
        uDebug() << "Moving attribute";
        UMLAttribute *att = static_cast<UMLAttribute*>(movingObject);
        if (newClassifier->getAttributeList().contains(att)) {
            QString msg = i18n("An attribute with that name already exists in %1.\n", newClassifier->name())
                          +
                          i18n("Choose a different name.");
            KMessageBox::error(this, msg, i18n("Attribute Name Invalid"), 0);
            return;
        }
        UMLAttribute* newAtt = static_cast<UMLAttribute*>(att->clone());

        UMLClassifier *oldClassifier = dynamic_cast<UMLClassifier*>(att->parent());
        if (oldClassifier) {
            oldClassifier->removeAttribute(att);
        }

        newClassifier->addAttribute(newAtt);
        m_doc->signalUMLObjectCreated(newAtt);  //:TODO: really?
uDebug() << "oldClassifier=" << oldClassifier->name() << " / newClassifier=" << newClassifier->name();  //:TODO:fischer
    }
//    emit moved();
    refactor(m_umlObject);  //:TODO:fischer
}

#include "refactoringassistant.moc"
