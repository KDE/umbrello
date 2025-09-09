/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Luis De la Parra <lparrab@gmx.net>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "refactoringassistant.h"

#include "umlattribute.h"
#include "basictypes.h"
#include "umlclassifier.h"
#include "classpropertiesdialog.h"
#include "debug_utils.h"
#include "object_factory.h"
#include "umloperation.h"
#include "umlattributedialog.h"
#include "umldoc.h"
#include "umloperationdialog.h"
#include "umlapp.h"  // Only needed for log{Warn,Error}

#include <KLocalizedString>
#include <KMessageBox>

#include <QApplication>
#include <QMenu>
#include <QPoint>

DEBUG_REGISTER(RefactoringAssistant)

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

    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(itemExecuted(QTreeWidgetItem*,int)));
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
    DEBUG() << "called for " << m_umlObject->name();

    m_alreadySeen.clear();
    addClassifier(obj, nullptr, true, true, true);
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
        return nullptr;
    }
    QTreeWidgetItem *i = const_cast<QTreeWidgetItem*>(item);
    if (m_umlObjectMap.find(i) == m_umlObjectMap.end()) {
        logWarn1("RefactoringAssistant::findUMLObject: Item with text %1 not found in uml map",
                 item->text(0));
        return nullptr;
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
    logWarn1("RefactoringAssistant::findUMLObject: Object id %1 does not have an item in the tree",
             Uml::ID::toString(obj->id()));
    return nullptr;
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
void RefactoringAssistant::setVisibilityIcon(QTreeWidgetItem *item, const UMLObject *obj)
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
 */
void RefactoringAssistant::objectModified()
{
    const UMLObject *obj = dynamic_cast<const UMLObject*>(sender());
    if (!obj)
        return;
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
    UMLOperation *op = listItem->asUMLOperation();
    DEBUG() << "operation = " << op->name();  //:TODO:
    UMLClassifier *parent = op->umlParent()->asUMLClassifier();
    if (!parent) {
        logWarn1("RefactoringAssistant::operationAdded(%1): Parent of operation is not a classifier",
                 op->name());
        return;
    }
    QTreeWidgetItem *item = findListViewItem(parent);
    if (!item) {
        return;
    }
    for (int i = 0; i < item->childCount(); ++i) {
        QTreeWidgetItem *folder = item->child(i);
        if (folder->text(1) == QStringLiteral("operations")) {
            item = new QTreeWidgetItem(folder, QStringList(op->name()));
            m_umlObjectMap[item] = op;
            connect(op, SIGNAL(modified()), this, SLOT(objectModified()));
            setVisibilityIcon(item, op);
            DEBUG() << "operation = " << op->name() << " added!";  //:TODO:
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
    UMLOperation *op = listItem->asUMLOperation();
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
    UMLAttribute *att = listItem->asUMLAttribute();
    DEBUG() << "attribute = " << att->name();  //:TODO:
    UMLClassifier *parent = att->umlParent()->asUMLClassifier();
    if (!parent) {
        logWarn1("RefactoringAssistant::attributeAdded(%1): Parent of attribute is not a classifier",
                 att->name());
        return;
    }
    QTreeWidgetItem *item = findListViewItem(parent);
    if (!item) {
        logWarn1("RefactoringAssistant::attributeAdded(%1): Parent is not in tree", att->name());
        return;
    }
    for (int i = 0; i < item->childCount(); ++i) {
        QTreeWidgetItem *folder = item->child(i);
        if (folder->text(1) == QStringLiteral("attributes")) {
            item = new QTreeWidgetItem(folder, QStringList(att->name()));
            m_umlObjectMap[item] = att;
            connect(att, SIGNAL(modified()), this, SLOT(objectModified()));
            setVisibilityIcon(item, att);
            DEBUG() << "attribute = " << att->name() << " added!";  //:TODO:
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
    UMLAttribute *att = listItem->asUMLAttribute();
    DEBUG() << "attribute = " << att->name();  //:TODO:
    QTreeWidgetItem *item = findListViewItem(att);
    if (!item) {
        logWarn1("RefactoringAssistant::attributeRemoved(%1): Attribute is not in tree", att->name());
        return;
    }
    disconnect(att, SIGNAL(modified()), this, SLOT(objectModified()));
    m_umlObjectMap.remove(item);
    delete item;
    DEBUG() << "attribute = " << att->name() << " deleted!";  //:TODO:
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
    QDialog *dia(nullptr);
    UMLObject::ObjectType t = obj->baseType();
    if (t == UMLObject::ot_Class || t == UMLObject::ot_Interface) {
        ClassPropertiesDialog *dialog = new ClassPropertiesDialog(this, obj, true);
        if (dialog && dialog->exec()) {
            // need to update something?
        }
        delete dialog;
    }
    else if (t == UMLObject::ot_Operation) {
        dia = new UMLOperationDialog(this, obj->asUMLOperation());
    }
    else if (t == UMLObject::ot_Attribute) {
        dia = new UMLAttributeDialog(this, obj->asUMLAttribute());
    }
    else {
        logWarn1("RefactoringAssistant::editProperties called for unknown type %1", UMLObject::toString(t));
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
        DEBUG() << "Delete class or interface - not yet implemented!";  //:TODO:
    }
    else if (t == UMLObject::ot_Operation) {
        QTreeWidgetItem *opNode = item->parent();
        if (opNode) {
            QTreeWidgetItem *parent = opNode->parent();
            UMLClassifier* c = findUMLObject(parent)->asUMLClassifier();
            if (!c) {
                logWarn0("No classifier - cannot delete!");
                return;
            }
            UMLOperation* op = obj->asUMLOperation();
            c->removeOperation(op);
        }
    }
    else if (t == UMLObject::ot_Attribute) {
        QTreeWidgetItem *attrNode = item->parent();
        if (attrNode) {
            QTreeWidgetItem *parent = attrNode->parent();
            UMLClassifier* c = findUMLObject(parent)->asUMLClassifier();
            if (!c) {
                logWarn0("No classifier - cannot delete!");
                return;
            }
            UMLAttribute* attr = obj->asUMLAttribute();
            c->removeAttribute(attr);
        }
    }
    else {
        logWarn1("RefactoringAssistant::deleteItem called for unknown type %1", UMLObject::toString(t));
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
    Q_UNUSED(method);
    QAction* action = new QAction(this);
    action->setText(text);
    if (icon != Icon_Utils::N_ICONTYPES) {
        action->setIcon(Icon_Utils::SmallIcon(icon));
    }
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
        //     DEBUG() << "No context menu for objects of type " << typeid(*obj).name();
        //     return;
        // }
        m_menu->addSeparator();
        m_menu->addAction(createAction(i18n("Properties"), SLOT(editProperties()), Icon_Utils::it_Properties));
        m_menu->addAction(createAction(i18n("Delete"), SLOT(deleteItem()), Icon_Utils::it_Delete));
    }
    else { //menu for other ViewItems
        if (item->text(1) == QStringLiteral("operations")) {
            m_menu->addAction(createAction(i18n("Add Operation"), SLOT(createOperation()), Icon_Utils::it_Public_Method));
        }
        else if (item->text(1) == QStringLiteral("attributes")) {
            m_menu->addAction(createAction(i18n("Add Attribute"), SLOT(createAttribute()), Icon_Utils::it_Public_Attribute));
        }
        else {
            logWarn0("Called for unsupported item.");
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
        logWarn0("Called with no item selected");
        return;
    }
    UMLObject *obj = findUMLObject(item);
    if (!obj->asUMLClassifier()) {
        logWarn0("Called for a non-classifier object.");
        return;
    }

    //classes have classes and interfaces interfaces as super/derived classifiers
    UMLObject::ObjectType t = obj->baseType();
    UMLClassifier *super = Object_Factory::createUMLObject(t)->asUMLClassifier();
    if (!super) {
        return;
    }
    m_doc->createUMLAssociation(obj, super, Uml::AssociationType::Generalization);

    //////////////////////   Manually add the classifier to the assistant - would be nicer to do it with
    /////////////////////    a signal, like operations and attributes
    QTreeWidgetItem  *baseFolder = nullptr;
    for (int i = 0; i < item->childCount(); ++i) {
        baseFolder = item->child(i);
        if (!baseFolder) {
            logWarn0("Cannot find base folder!");
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
        logWarn0("Called with no item selected.");
        return;
    }
    UMLObject *obj = findUMLObject(item);
    if (!obj->asUMLClassifier()) {
        logWarn0("Called for a non-classifier object.");
        return;
    }

    //classes have classes and interfaces have interfaces as super/derived classifiers
    UMLObject::ObjectType t = obj->baseType();
    UMLClassifier *derived = Object_Factory::createUMLObject(t)->asUMLClassifier();
    if (!derived) {
        return;
    }
    m_doc->createUMLAssociation(derived, obj, Uml::AssociationType::Generalization);

    //////////////////////   Manually add the classifier to the assistant - would be nicer to do it with
    /////////////////////    a signal, like operations and attributes
    QTreeWidgetItem  *derivedFolder = nullptr;
    for (int i = 0; i < item->childCount(); ++i) {
        derivedFolder = item->child(i);
        if (!derivedFolder) {
            logWarn0("Cannot find derived folder!");
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
    logWarn0("Not implemented... finish addSuperClassifier() first!!");
    return;
    //  QTreeWidgetItem *item = selectedListViewItem();
    //  UMLObject *obj = findUMLObject(item);
    //  if(!obj->asUMLClassifier())
    //          return;
    //  UMLObject *n = Object_Factory::createUMLObject(UMLObject::ot_Interface));
    //  if (!n) {
    //      return;
    //  }
    //  m_doc->createUMLAssociation(n, obj, UMLObject::at_Realization);
    //  //refresh, add classifier to assistant
}

/**
 * Create new operation.
 */
void RefactoringAssistant::createOperation()
{
    QTreeWidgetItem *item = currentItem();
    if (!item) {
        logWarn0("Called with no item selected.");
        return;
    }
    UMLClassifier *c = findUMLObject(item)->asUMLClassifier();
    if (!c) {  // find parent
        QTreeWidgetItem *parent = item->parent();
        c = findUMLObject(parent)->asUMLClassifier();
        if (!c) {
            logWarn0("No classifier - cannot create!");
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
        logWarn0("Called with no item selected.");
        return;
    }
    UMLClassifier *c = findUMLObject(item)->asUMLClassifier();
    if (!c) {  // find parent
        QTreeWidgetItem *parent = item->parent();
        c = findUMLObject(parent)->asUMLClassifier();
        if (!c) {
            logWarn0("No classifier - cannot create!");
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
        logWarn0("No classifier given - do nothing!");
        return;
    }
    DEBUG() << classifier->name() << " added.";
    QTreeWidgetItem *classifierItem, *item;
    if (parent) {
        classifierItem = parent;
    }
    else {
        classifierItem = new QTreeWidgetItem(this, QStringList(classifier->name()));
        m_umlObjectMap[classifierItem] = classifier;
    }
    m_alreadySeen << classifier;

    connect(classifier, SIGNAL(modified()),
            this, SLOT(objectModified()));

    // add attributes
    connect(classifier, SIGNAL(attributeAdded(UMLClassifierListItem*)),
            this, SLOT(attributeAdded(UMLClassifierListItem*)));
    connect(classifier, SIGNAL(attributeRemoved(UMLClassifierListItem*)),
            this, SLOT(attributeRemoved(UMLClassifierListItem*)));

    QStringList itemTextAt;
    itemTextAt << i18n("Attributes") << QStringLiteral("attributes");
    QTreeWidgetItem *attsFolder = new QTreeWidgetItem(classifierItem, itemTextAt);
    attsFolder->setIcon(0, Icon_Utils::SmallIcon(Icon_Utils::it_Folder_Orange));
    attsFolder->setExpanded(true);
    UMLAttributeList atts(classifier->getAttributeList());
    for(UMLAttribute *att : atts) {
        attributeAdded(att);
    }

    // add operations
    connect(classifier, SIGNAL(operationAdded(UMLClassifierListItem*)),
            this, SLOT(operationAdded(UMLClassifierListItem*)));
    connect(classifier, SIGNAL(operationRemoved(UMLClassifierListItem*)),
            this, SLOT(operationRemoved(UMLClassifierListItem*)));

    QStringList itemTextOp;
    itemTextOp << i18n("Operations") << QStringLiteral("operations");
    QTreeWidgetItem *opsFolder = new QTreeWidgetItem(classifierItem, itemTextOp);
    opsFolder->setIcon(0, Icon_Utils::SmallIcon(Icon_Utils::it_Folder_Orange));
    opsFolder->setExpanded(true);
    UMLOperationList ops(classifier->getOperationsList());
    for(UMLOperation *op : ops) {
        operationAdded(op);
    }

    if (addSuper) {  // add base classifiers
        QTreeWidgetItem *superFolder = new QTreeWidgetItem(classifierItem, QStringList(i18n("Base Classifiers")));
        superFolder->setExpanded(true);
        UMLClassifierList super = classifier->findSuperClassConcepts();
        for(UMLClassifier *cl : super) {
            item = new QTreeWidgetItem(superFolder, QStringList(cl->name()));
            item->setIcon(0, Icon_Utils::SmallIcon(Icon_Utils::it_Generalisation));
            item->setExpanded(true);
            m_umlObjectMap[item] = cl;
            if (recurse) {
                if (m_alreadySeen.contains(cl)) {
                    DEBUG() << "super class already seen" << cl;
                    continue;
                }
                addClassifier(cl, item, true, false, true);
            }
        }
    }

    if (addSub) {  // add derived classifiers
        QTreeWidgetItem *derivedFolder = new QTreeWidgetItem(classifierItem, QStringList(i18n("Derived Classifiers")));
        derivedFolder->setExpanded(true);
        UMLClassifierList derived = classifier->findSubClassConcepts();
        for(UMLClassifier *d : derived) {
            item = new QTreeWidgetItem(derivedFolder, QStringList(d->name()));
            item->setIcon(0, Icon_Utils::SmallIcon(Icon_Utils::it_Uniassociation));
            item->setExpanded(true);
            m_umlObjectMap[item] = d;
            if (recurse) {
                if (m_alreadySeen.contains(d)) {
                    DEBUG() << "derived class already seen" << d;
                    continue;
                }
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
                if ((target->text(1) == QStringLiteral("operations")) &&
                    (parent->text(1) == QStringLiteral("operations"))) {
DEBUG() << "accept operation " << item->text(0);  //:TODO:fischer
                    event->accept();
                    return;
                }
                if ((target->text(1) == QStringLiteral("attributes")) &&
                    (parent->text(1) == QStringLiteral("attributes"))) {
DEBUG() << "accept attribute " << item->text(0);  //:TODO:fischer
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
    DEBUG() << "dropping=" << movingItem->text(0);

    if (event->source() == this) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
DEBUG() << "accept";  //:TODO:fischer
    }
    else {
        event->acceptProposedAction();
DEBUG() << "acceptProposedAction";  //:TODO:fischer
        return;
    }

    QTreeWidgetItem* afterme = itemAt(event->pos());
    if (!afterme) {
        logWarn0("Drop target not found - aborting drop!");
        return;
    }
    DEBUG() << "Dropping after item = " << afterme->text(0);  //:TODO:fischer

    // when dropping on a class, we have to put the item in the appropriate folder!
    UMLObject *movingObject;
    UMLClassifier *newClassifier;

    if ((movingItem == afterme) || !(movingObject = findUMLObject(movingItem))) {
        logWarn0("Moving item not found or dropping after itself or item not found in uml obj map. aborting. (drop had already been accepted)");
        return;
    }
    QTreeWidgetItem* parentItem = afterme->parent();
    UMLObject::ObjectType t = movingObject->baseType();
    newClassifier = findUMLObject(parentItem)->asUMLClassifier();
    if (!newClassifier) {
        const QString parentText = parentItem->text(1);
        if ((parentText == QStringLiteral("operations") && t == UMLObject::ot_Operation) ||
            (parentText == QStringLiteral("attributes") && t == UMLObject::ot_Attribute)) {
            newClassifier = findUMLObject(parentItem->parent())->asUMLClassifier();
        }
        if (!newClassifier) {
            logWarn0("New parent of object is not a Classifier - Drop had already been accepted - check!");
            return;
        }
    }
    if (t == UMLObject::ot_Operation) {
        DEBUG() << "Moving operation";
        UMLOperation *op = movingObject->asUMLOperation();
        if (newClassifier->checkOperationSignature(op->name(), op->getParmList())) {
            QString msg = i18n("An operation with that signature already exists in %1.\n", newClassifier->name())
                          +
                          i18n("Choose a different name or parameter list.");
            KMessageBox::error(this, msg, i18n("Operation Name Invalid"));
            return;
        }
        UMLOperation* newOp = op->clone()->asUMLOperation();

        UMLClassifier *oldClassifier = op->umlParent()->asUMLClassifier();
        if (oldClassifier) {
            oldClassifier->removeOperation(op);
            DEBUG() << "oldClassifier=" << oldClassifier->name() << " / newClassifier=" << newClassifier->name();  //:TODO:fischer
        }

        newClassifier->addOperation(newOp);
        m_doc->signalUMLObjectCreated(newOp);  //:TODO: really?
    }
    else if (t == UMLObject::ot_Attribute) {
        DEBUG() << "Moving attribute";
        UMLAttribute *att = movingObject->asUMLAttribute();
        if (newClassifier->getAttributeList().contains(att)) {
            QString msg = i18n("An attribute with that name already exists in %1.\n", newClassifier->name())
                          +
                          i18n("Choose a different name.");
            KMessageBox::error(this, msg, i18n("Attribute Name Invalid"));
            return;
        }
        UMLAttribute* newAtt = att->clone()->asUMLAttribute();

        UMLClassifier *oldClassifier = att->umlParent()->asUMLClassifier();
        if (oldClassifier) {
            oldClassifier->removeAttribute(att);
            DEBUG() << "oldClassifier=" << oldClassifier->name() << " / newClassifier=" << newClassifier->name();  //:TODO:fischer
        }

        newClassifier->addAttribute(newAtt);
        m_doc->signalUMLObjectCreated(newAtt);  //:TODO: really?
    }
//    Q_EMIT moved();
    refactor(m_umlObject);  //:TODO:fischer
}

