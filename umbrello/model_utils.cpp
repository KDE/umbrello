/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "model_utils.h"

// app includes
#include "floatingtextwidget.h"
#define DBG_SRC QStringLiteral("Model_Utils")
#include "debug_utils.h"
#include "umlobject.h"
#include "umlpackagelist.h"
#include "uniqueconstraint.h"
#include "package.h"
#include "folder.h"
#include "classifier.h"
#include "enum.h"
#include "instance.h"
#include "entity.h"
#include "template.h"
#include "operation.h"
#include "attribute.h"
#include "association.h"
#include "umlrole.h"
#include "umldoc.h"
#include "uml.h"
#include "umllistview.h"
#include "umllistviewitem.h"
#include "umlscene.h"
#include "umlview.h"
#include "codegenerator.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QRegExp>
#include <QStringList>

DEBUG_REGISTER(Model_Utils)

namespace Model_Utils {

/**
 * Determines whether the given widget type is cloneable.
 *
 * @param type  The input WidgetType.
 * @return      True if the given type is cloneable.
 */
bool isCloneable(WidgetBase::WidgetType type)
{
    switch (type) {
    case WidgetBase::wt_Actor:
    case WidgetBase::wt_UseCase:
    case WidgetBase::wt_Class:
    case WidgetBase::wt_Interface:
    case WidgetBase::wt_Enum:
    case WidgetBase::wt_Datatype:
    case WidgetBase::wt_Package:
    case WidgetBase::wt_Component:
    case WidgetBase::wt_Port:
    case WidgetBase::wt_Node:
    case WidgetBase::wt_Artifact:
    case WidgetBase::wt_Instance:
    case WidgetBase::wt_Entity:
        return true;
    default:
        return false;
    }
}

/**
 * Normalize a type name with respect to interspersed spaces.
 * @param type  Input type name e.g. from a user text entry dialog.
 * @return      Normalized type name.
 */
QString normalize(QString type)
{
    QString str = type.simplified();
    int pos;
    // Remove space between word and non word
    QRegExp word_nonword(QStringLiteral("\\w \\W"));
    pos = 0;
    while ((pos = word_nonword.indexIn(str, pos)) != -1) {
        str.remove(++pos, 1);
    }
    // Remove space between non word and word
    QRegExp nonword_word(QStringLiteral("\\W \\w"));
    pos = 0;
    while ((pos = nonword_word.indexIn(str, pos)) != -1) {
        str.remove(++pos, 1);
    }
    // Remove space between non word and non word
    QRegExp nonword_nonword(QStringLiteral("\\W \\W"));
    pos = 0;
    while ((pos = nonword_nonword.indexIn(str, pos)) != -1) {
        str.remove(++pos, 1);
    }
    return str;
}

/**
 * Seek the given id in the given list of objects.
 * Each list element may itself contain other objects
 * and the search is done recursively.
 *
 * @param id       The unique ID to seek.
 * @param inList   The UMLObjectList in which to search.
 * @return Pointer to the UMLObject that matches the ID (NULL if none matches).
 */
UMLObject* findObjectInList(Uml::ID::Type id, const UMLObjectList& inList)
{
    for (UMLObjectListIt oit(inList); oit.hasNext();) {
        UMLObject *obj = oit.next();
        uIgnoreZeroPointer(obj);
        if (obj->id() == id)
            return obj;
        UMLObject *o;
        UMLObject::ObjectType t = obj->baseType();
        switch (t) {
        case UMLObject::ot_Folder:
        case UMLObject::ot_Package:
        case UMLObject::ot_Component:
            o = obj->asUMLPackage()->findObjectById(id);
            if (o)
                return o;
            break;
        case UMLObject::ot_Interface:
        case UMLObject::ot_Class:
        case UMLObject::ot_Enum:
            o = obj->asUMLClassifier()->findChildObjectById(id);
            if (o == nullptr &&
                    (t == UMLObject::ot_Interface || t == UMLObject::ot_Class))
                o = obj->asUMLPackage()->findObjectById(id);
            if (o)
                return o;
            break;
        case UMLObject::ot_Instance:
            o = obj->asUMLInstance()->findChildObjectById(id);
            if (o)
                return o;
            break;
        case UMLObject::ot_Entity:
            o = obj->asUMLEntity()->findChildObjectById(id);
            if (o)
                return o;
            o = obj->asUMLPackage()->findObjectById(id);
            if (o)
                return o;
            break;
        case UMLObject::ot_Association:
            {
                UMLAssociation *assoc = obj->asUMLAssociation();
                UMLRole *rA = assoc->getUMLRole(Uml::RoleType::A);
                if (rA->id() == id)
                    return rA;
                UMLRole *rB = assoc->getUMLRole(Uml::RoleType::B);
                if (rB->id() == id)
                    return rB;
            }
            break;
        default:
            break;
        }
    }
    return nullptr;
}

/**
 * Find the UML object of the given type and name in the passed-in list.
 *
 * @param inList        List in which to seek the object.
 * @param inName        Name of the object to find.
 * @param type          ObjectType of the object to find (optional.)
 *                      When the given type is ot_UMLObject the type is
 *                      disregarded, i.e. the given name is the only
 *                      search criterion.
 * @param currentObj    Object relative to which to search (optional.)
 *                      If given then the enclosing scope(s) of this
 *                      object are searched before the global scope.
 * @return      Pointer to the UMLObject found, or NULL if not found.
 */
UMLObject* findUMLObject(const UMLObjectList& inList,
                         const QString& inName,
                         UMLObject::ObjectType type /* = ot_UMLObject */,
                         UMLObject  *currentObj /* = nullptr */)
{
    const bool caseSensitive = UMLApp::app()->activeLanguageIsCaseSensitive();
    QString name = normalize(inName);
    const bool atGlobalScope = name.startsWith(QStringLiteral("::"));
    if (atGlobalScope) {
        name = name.mid(2);
        currentObj = nullptr;
    }
    QStringList components;
#ifdef TRY_BUGFIX_120862
    // If we have a pointer or a reference in cpp we need to remove
    // the asterisks and ampersands in order to find the appropriate object
    if (UMLApp::app()->getActiveLanguage() == Uml::pl_Cpp) {
        if (name.endsWith(QLatin1Char('*')))
            name.remove(QLatin1Char('*'));
        else if (name.contains(QLatin1Char('&')))
            name.remove(QLatin1Char('&'));
    }
#endif
    QString scopeSeparator = UMLApp::app()->activeLanguageScopeSeparator();
    if (name.contains(scopeSeparator))
        components = name.split(scopeSeparator);
    QString nameWithoutFirstPrefix;
    if (components.size() > 1) {
        name = components.front();
        components.pop_front();
        nameWithoutFirstPrefix = components.join(scopeSeparator);
    }
    if (currentObj) {
        UMLPackage  *pkg = nullptr;
        if (currentObj->asUMLClassifierListItem()) {
            currentObj = currentObj->umlParent();
        }
        pkg = currentObj->asUMLPackage();
        if (pkg == nullptr || pkg->baseType() == UMLObject::ot_Association)
            pkg = currentObj->umlPackage();
        // Remember packages that we've seen - for avoiding cycles.
        UMLPackageList seenPkgs;
        for (; pkg; pkg = currentObj->umlPackage()) {
            if (nameWithoutFirstPrefix.isEmpty()
                && (type == UMLObject::ot_UMLObject ||
                    type == UMLObject::ot_Folder ||
                    type == UMLObject::ot_Package || type == pkg->baseType())) {
                if (caseSensitive) {
                    if (pkg->name() == name)
                        return pkg;
                } else if (pkg->name().toLower() == name.toLower()) {
                    return pkg;
                }
            }
            if (seenPkgs.indexOf(pkg) != -1) {
                logError2("Model_Utils::findUMLObject(%1): breaking out of cycle involving %2",
                          name, pkg->name());
                break;
            }
            seenPkgs.append(pkg);

            // exclude non package type
            // pg->asUMLPackage() fails for unknown reason
            // see https://bugs.kde.org/show_bug.cgi?id=341709
            UMLObject::ObjectType foundType = pkg->baseType();
            if (foundType != UMLObject::ot_Package &&
                foundType != UMLObject::ot_Folder &&
                foundType != UMLObject::ot_Class &&
                foundType != UMLObject::ot_Interface &&
                foundType != UMLObject::ot_Component) {
                continue;
            }
            UMLObjectList objectsInCurrentScope = pkg->containedObjects();
            for (UMLObjectListIt oit(objectsInCurrentScope); oit.hasNext();) {
                UMLObject *obj = oit.next();
                uIgnoreZeroPointer(obj);
                if (caseSensitive) {
                    if (obj->name() != name)
                        continue;
                } else if (obj->name().toLower() != name.toLower()) {
                    continue;
                }
                UMLObject::ObjectType foundType = obj->baseType();
                if (nameWithoutFirstPrefix.isEmpty()) {
                    if (type != UMLObject::ot_UMLObject && type != foundType) {
                        logDebug3("findUMLObject type mismatch for %1 (seeking type: %2, found type: %3)",
                                  name, UMLObject::toString(type), UMLObject::toString(foundType));
                        // Class, Interface, and Datatype are all Classifiers
                        // and are considered equivalent.
                        // The caller must be prepared to handle possible mismatches.
                        if ((type == UMLObject::ot_Class ||
                             type == UMLObject::ot_Interface ||
                             type == UMLObject::ot_Datatype) &&
                            (foundType == UMLObject::ot_Class ||
                             foundType == UMLObject::ot_Interface ||
                             foundType == UMLObject::ot_Datatype)) {
                            return obj;
                        }
                        // Code import may set <<class-or-package>> stereotype
                        if ((type == UMLObject::ot_Package || type == UMLObject::ot_Class)
                            && obj->stereotype() == QStringLiteral("class-or-package")) {
                            return obj;
                        }
                        continue;
                    }
                    return obj;
                }
                if (foundType != UMLObject::ot_Package &&
                    foundType != UMLObject::ot_Folder &&
                    foundType != UMLObject::ot_Class &&
                    foundType != UMLObject::ot_Interface &&
                    foundType != UMLObject::ot_Component) {
                    logDebug2("findUMLObject found %1 %2 is not a package (?)", UMLObject::toString(foundType), name);
                    continue;
                }
                UMLPackage *pkg = obj->asUMLPackage();
                return findUMLObject(pkg->containedObjects(),
                                      nameWithoutFirstPrefix, type);
            }
            currentObj = pkg;
        }
    }
    for (UMLObjectListIt oit(inList); oit.hasNext();) {
        UMLObject *obj = oit.next();
        uIgnoreZeroPointer(obj);
        if (caseSensitive) {
            if (obj->name() != name)
                continue;
        } else if (obj->name().toLower() != name.toLower()) {
            continue;
        }
        UMLObject::ObjectType foundType = obj->baseType();
        if (nameWithoutFirstPrefix.isEmpty()) {
            if (type != UMLObject::ot_UMLObject && type != foundType) {
                // Code import may set <<class-or-package>> stereotype
                if ((type == UMLObject::ot_Package || type == UMLObject::ot_Class)
                    && obj->stereotype() == QStringLiteral("class-or-package")) {
                    return obj;
                }
                logDebug3("findUMLObject type mismatch for %1 (seeking type: %2, found type: %3)",
                          name, UMLObject::toString(type), UMLObject::toString(foundType));
                continue;
            }
            return obj;
        }
        if (foundType != UMLObject::ot_Package &&
            foundType != UMLObject::ot_Folder &&
            foundType != UMLObject::ot_Class &&
            foundType != UMLObject::ot_Interface &&
            foundType != UMLObject::ot_Component) {
            logDebug2("findUMLObject found %1 (%2) is not a package (?)", name, UMLObject::toString(foundType));
            continue;
        }
        const UMLPackage *pkg = obj->asUMLPackage();
        return findUMLObject(pkg->containedObjects(),
                              nameWithoutFirstPrefix, type);
    }
    return nullptr;
}

/**
 * Find the UML object of the given type and name in the passed-in list.
 * This method searches for the raw name.
 *
 * @param inList        List in which to seek the object.
 * @param name          Name of the object to find.
 * @param type          ObjectType of the object to find (optional.)
 *                      When the given type is ot_UMLObject the type is
 *                      disregarded, i.e. the given name is the only
 *                      search criterion.
 * @param currentObj    Object relative to which to search (optional.)
 *                      If given then the enclosing scope(s) of this
 *                      object are searched before the global scope.
 * @return      Pointer to the UMLObject found, or NULL if not found.
 */
UMLObject* findUMLObjectRaw(const UMLObjectList& inList,
                            const QString& name,
                            UMLObject::ObjectType type /* = ot_UMLObject */,
                            UMLObject  *currentObj /*= nullptr*/)
{
    Q_UNUSED(currentObj);
    for (UMLObjectListIt oit(inList); oit.hasNext();) {
        UMLObject *obj = oit.next();
        if (obj->name() == name && type == obj->baseType())
            return obj;
    }
    return nullptr;
}

/**
 * Find the UML object of the given type and name in the passed-in list.
 * This method searches for the raw name.
 *
 * @param inList        List in which to seek the object.
 * @param name          Name of the object to find.
 * @param type          ObjectType of the object to find (optional.)
 *                      When the given type is ot_UMLObject the type is
 *                      disregarded, i.e. the given name is the only
 *                      search criterion.
 * @return      Pointer to the UMLObject found, or NULL if not found.
 */
UMLObject* findUMLObjectRecursive(const UMLObjectList& inList,
                                  const QString& name,
                                  UMLObject::ObjectType type /* = ot_UMLObject */)
{
    for(UMLObject *obj : inList) {
        if (obj->name() == name && type == obj->baseType())
            return obj;
        const UMLPackage *pkg = obj->asUMLPackage();
        if (pkg && pkg->containedObjects().size() > 0) {
            UMLObject *o = findUMLObjectRecursive(pkg->containedObjects(), name, type);
            if (o)
                return o;
        }
    }
    return nullptr;
}

/**
 * Get the root folder of the given UMLObject.
 */
UMLPackage* rootPackage(UMLObject* obj)
{
    if (obj == nullptr)
        return nullptr;
    UMLPackage* root = obj->umlPackage();
    if (root == nullptr) {
        root = obj->asUMLPackage();
    } else {
        while (root->umlPackage() != nullptr) {
            root = root->umlPackage();
        }
    }
    return root;
}

/**
 * Add the given list of views to the tree view.
 * @param viewList   the list of views to add
 */
void treeViewAddViews(const UMLViewList& viewList)
{
    UMLListView* tree = UMLApp::app()->listView();
    for(UMLView *v :  viewList) {
        if (tree->findItem(v->umlScene()->ID()) != nullptr) {
            continue;
        }
        tree->createDiagramItem(v);
    }
}

/**
 * Change an icon of an object in the tree view.
 * @param object   the object in the treeViewAddViews
 * @param to       the new icon type for the given object
 */
void treeViewChangeIcon(UMLObject* object, Icon_Utils::IconType to)
{
    UMLListView* tree = UMLApp::app()->listView();
    tree->changeIconOf(object, to);
}

/**
 * Set the given object to the current item in the tree view.
 * @param object   the object which will be the current item
 */
void treeViewSetCurrentItem(UMLObject* object)
{
    UMLListView* tree = UMLApp::app()->listView();
    UMLListViewItem* item = tree->findUMLObject(object);
    tree->setCurrentItem(item);
}

/**
 * Move an object to a new container in the tree view.
 * @param container   the new container for the object
 * @param object      the to be moved object
 */
void treeViewMoveObjectTo(UMLObject* container, UMLObject* object)
{
    UMLListView *listView = UMLApp::app()->listView();
    UMLListViewItem *newParent = listView->findUMLObject(container);
    listView->moveObject(object->id(),
                   Model_Utils::convert_OT_LVT(object),
                   newParent);
}

/**
 * Return the current UMLObject from the tree view.
 * @return   the UML object of the current item
 */
UMLObject* treeViewGetCurrentObject()
{
    UMLListView *listView = UMLApp::app()->listView();
    UMLListViewItem *current = dynamic_cast<UMLListViewItem*>(listView->currentItem());
    if (current == nullptr)
        return nullptr;
    return current->umlObject();
}

/**
 * Return the UMLPackage if the current item
 * in the tree view is a package. Return the
 * closest package in the tree view or NULL otherwise
 *
 * @return   the package or NULL
 */
UMLPackage* treeViewGetPackageFromCurrent()
{
    UMLListView *listView = UMLApp::app()->listView();
    UMLListViewItem *parentItem = (UMLListViewItem*)listView->currentItem();
    while (parentItem) {
        UMLListViewItem::ListViewType lvt = parentItem->type();
        if (Model_Utils::typeIsContainer(lvt)) {
            UMLObject *o = parentItem->umlObject();
            return o->asUMLPackage();
        }

        // selected item is not a container, try to find the
        // container higher up in the tree view
        parentItem = static_cast<UMLListViewItem*>(parentItem->parent());
    }

    return nullptr;
}

/**
 * Build the diagram name from the tree view.

 * The function returns a relative path constructed from the folder hierarchy.
 * @param id   the id of the diaram
 * @return     the constructed diagram name
 */
QString treeViewBuildDiagramName(Uml::ID::Type id)
{
    UMLListView *listView = UMLApp::app()->listView();
    UMLListViewItem* listViewItem = listView->findItem(id);

    if (listViewItem) {
        QString name = listViewItem->text(0);
        listViewItem = static_cast<UMLListViewItem*>(listViewItem->parent());
        
        // Relies on the tree structure of the UMLListView. There are a base "Views" folder
        // and five children, one for each view type (Logical, use case, components, deployment
        // and entity relationship)
        while (listView->rootView(listViewItem->type()) == nullptr) {
            name.insert(0, listViewItem->text(0) + QLatin1Char('/'));
            listViewItem = static_cast<UMLListViewItem*>(listViewItem->parent());
            if (listViewItem == nullptr)
                break;
        }
        return name;
    }
    else {
        logWarn1("Model_Utils::treeViewBuildDiagramName: diagram with id %1 not found",
                 Uml::ID::toString(id));
        return QString();
    }
}

/**
 * Returns a name for the new object, appended with a number
 * if the default name is taken e.g. new_actor, new_actor_1
 * etc.
 * @param type      The object type.
 * @param parentPkg The package in which to compare the name.
 * @param prefix    The prefix to use (optional)
 *                  If no prefix is given then a type related
 *                  prefix will be chosen internally.
 */
QString uniqObjectName(UMLObject::ObjectType type, UMLPackage *parentPkg, QString prefix)
{
    QString currentName = prefix;
    if (currentName.isEmpty()) {
        switch(type) {
        case UMLObject::ot_Actor:               currentName = i18n("new_actor");                  break;
        case UMLObject::ot_Artifact:            currentName = i18n("new_artifact");               break;
        case UMLObject::ot_Association:         currentName = i18n("new_association");            break;
        case UMLObject::ot_Attribute:           currentName = i18n("new_attribute");              break;
        case UMLObject::ot_Category:            currentName = i18n("new_category");               break;
        case UMLObject::ot_CheckConstraint:     currentName = i18n("new_check_constraint");       break;
        case UMLObject::ot_Class:               currentName = i18n("new_class");                  break;
        case UMLObject::ot_Component:           currentName = i18n("new_component");              break;
        case UMLObject::ot_Datatype:            currentName = i18n("new_datatype");               break;
        case UMLObject::ot_Entity:              currentName = i18n("new_entity");                 break;
        case UMLObject::ot_EntityAttribute:     currentName = i18n("new_entity_attribute");       break;
        case UMLObject::ot_EntityConstraint:    currentName = i18n("new_entity_constraint");      break;
        case UMLObject::ot_Enum:                currentName = i18n("new_enum");                   break;
        case UMLObject::ot_EnumLiteral:         currentName = i18n("new_enum_literal");           break;
        case UMLObject::ot_Folder:              currentName = i18n("new_folder");                 break;
        case UMLObject::ot_ForeignKeyConstraint:currentName = i18n("new_foreign_key_constraint"); break;
        case UMLObject::ot_Instance:            currentName = i18n("new_instance");               break;
        case UMLObject::ot_InstanceAttribute:   currentName = i18n("new_instance_attribute");     break;
        case UMLObject::ot_Interface:           currentName = i18n("new_interface");              break;
        case UMLObject::ot_Node:                currentName = i18n("new_node");                   break;
        case UMLObject::ot_Operation:           currentName = i18n("new_operation");              break;
        case UMLObject::ot_Package:             currentName = i18n("new_package");                break;
        case UMLObject::ot_Port:                currentName = i18n("new_port");                   break;
        case UMLObject::ot_Role:                currentName = i18n("new_role");                   break;
        case UMLObject::ot_Stereotype:          currentName = i18n("new_stereotype");             break;
        case UMLObject::ot_Template:            currentName = i18n("new_template");               break;
        case UMLObject::ot_UniqueConstraint:    currentName = i18n("new_unique_constraint");      break;
        case UMLObject::ot_UseCase:             currentName = i18n("new_use_case");               break;
        default:
            currentName = i18n("new_object");
            logWarn1("Model_Utils::uniqObjectName unknown object type %1", UMLObject::toString(type));
        }
    }
    UMLDoc *doc = UMLApp::app()->document();
    QString name = currentName;
    for (int number = 1; !doc->isUnique(name, parentPkg); ++number)  {
        name = currentName + QLatin1Char('_') + QString::number(number);
    }
    return name;
}

/**
 * Returns translated title string used by uml object related dialogs
 * @param type uml object type
 * @return translated title string
 */
QString newTitle(UMLObject::ObjectType type)
{
    switch(type) {
    case UMLObject::ot_Actor:               return i18n("New actor");
    case UMLObject::ot_Artifact:            return i18n("New artifact");
    case UMLObject::ot_Association:         return i18n("New association");
    case UMLObject::ot_Attribute:           return i18n("New attribute");
    case UMLObject::ot_Category:            return i18n("New category");
    case UMLObject::ot_CheckConstraint:     return i18n("New check constraint");
    case UMLObject::ot_Class:               return i18n("New class");
    case UMLObject::ot_Component:           return i18n("New component");
    case UMLObject::ot_Datatype:            return i18n("New datatype");
    case UMLObject::ot_Entity:              return i18n("New entity");
    case UMLObject::ot_EntityAttribute:     return i18n("New entity attribute");
    case UMLObject::ot_EntityConstraint:    return i18n("New entity constraint");
    case UMLObject::ot_Enum:                return i18n("New enum");
    case UMLObject::ot_EnumLiteral:         return i18n("New enum literal");
    case UMLObject::ot_Folder:              return i18n("New folder");
    case UMLObject::ot_ForeignKeyConstraint:return i18n("New foreign key constraint");
    case UMLObject::ot_Instance:            return i18n("New instance");
    case UMLObject::ot_InstanceAttribute:   return i18n("New instance attribute");
    case UMLObject::ot_Interface:           return i18n("New interface");
    case UMLObject::ot_Node:                return i18n("New node");
    case UMLObject::ot_Operation:           return i18n("New operation");
    case UMLObject::ot_Package:             return i18n("New package");
    case UMLObject::ot_Port:                return i18n("New port");
    case UMLObject::ot_Role:                return i18n("New role");
    case UMLObject::ot_Stereotype:          return i18n("New stereotype");
    case UMLObject::ot_Template:            return i18n("New template");
    case UMLObject::ot_UniqueConstraint:    return i18n("New unique constraint");
    case UMLObject::ot_UseCase:             return i18n("New use case");
    default:
        logWarn1("Model_Utils::newTitle unknown object type %1", UMLObject::toString(type));
        return i18n("New UML object");
    }
}

/**
 * Returns translated text string used by uml object related dialogs
 * @param type uml object type
 * @return translated text string
 */
QString newText(UMLObject::ObjectType type)
{
    switch(type) {
    case UMLObject::ot_Actor:               return i18n("Enter the name of the new actor:");
    case UMLObject::ot_Artifact:            return i18n("Enter the name of the new artifact:");
    case UMLObject::ot_Association:         return i18n("Enter the name of the new association:");
    case UMLObject::ot_Attribute:           return i18n("Enter the name of the new attribute:");
    case UMLObject::ot_Category:            return i18n("Enter the name of the new category:");
    case UMLObject::ot_CheckConstraint:     return i18n("Enter the name of the new check constraint:");
    case UMLObject::ot_Class:               return i18n("Enter the name of the new class:");
    case UMLObject::ot_Component:           return i18n("Enter the name of the new component:");
    case UMLObject::ot_Datatype:            return i18n("Enter the name of the new datatype:");
    case UMLObject::ot_Entity:              return i18n("Enter the name of the new entity:");
    case UMLObject::ot_EntityAttribute:     return i18n("Enter the name of the new entity attribute:");
    case UMLObject::ot_EntityConstraint:    return i18n("Enter the name of the new entity constraint:");
    case UMLObject::ot_Enum:                return i18n("Enter the name of the new enum:");
    case UMLObject::ot_EnumLiteral:         return i18n("Enter the name of the new enum literal:");
    case UMLObject::ot_Folder:              return i18n("Enter the name of the new folder:");
    case UMLObject::ot_ForeignKeyConstraint:return i18n("Enter the name of the new foreign key constraint:");
    case UMLObject::ot_Instance:            return i18n("Enter the name of the new instance:");
    case UMLObject::ot_InstanceAttribute:   return i18n("Enter the name of the new instance attribute:");
    case UMLObject::ot_Interface:           return i18n("Enter the name of the new interface:");
    case UMLObject::ot_Node:                return i18n("Enter the name of the new node:");
    case UMLObject::ot_Operation:           return i18n("Enter the name of the new operation:");
    case UMLObject::ot_Package:             return i18n("Enter the name of the new package:");
    case UMLObject::ot_Port:                return i18n("Enter the name of the new port:");
    case UMLObject::ot_Role:                return i18n("Enter the name of the new role:");
    case UMLObject::ot_Stereotype:          return i18n("Enter the name of the new stereotype:");
    case UMLObject::ot_Template:            return i18n("Enter the name of the new template:");
    case UMLObject::ot_UniqueConstraint:    return i18n("Enter the name of the new unique constraint:");
    case UMLObject::ot_UseCase:             return i18n("Enter the name of the new use case:");
    default:
        logWarn1("Model_utilS::newText unknown object type %1", UMLObject::toString(type));
        return i18n("Enter the name of the new UML object");
    }
}

/**
 * Returns translated title string used by uml object related dialogs
 * @param type uml object type
 * @return translated title string
 */
QString renameTitle(UMLObject::ObjectType type)
{
    switch(type) {
    case UMLObject::ot_Actor:               return i18n("Rename actor");
    case UMLObject::ot_Artifact:            return i18n("Rename artifact");
    case UMLObject::ot_Association:         return i18n("Rename association");
    case UMLObject::ot_Attribute:           return i18n("Rename attribute");
    case UMLObject::ot_Category:            return i18n("Rename category");
    case UMLObject::ot_CheckConstraint:     return i18n("Rename check constraint");
    case UMLObject::ot_Class:               return i18n("Rename class");
    case UMLObject::ot_Component:           return i18n("Rename component");
    case UMLObject::ot_Datatype:            return i18n("Rename datatype");
    case UMLObject::ot_Entity:              return i18n("Rename entity");
    case UMLObject::ot_EntityAttribute:     return i18n("Rename entity attribute");
    case UMLObject::ot_EntityConstraint:    return i18n("Rename entity constraint");
    case UMLObject::ot_Enum:                return i18n("Rename enum");
    case UMLObject::ot_EnumLiteral:         return i18n("Rename enum literal");
    case UMLObject::ot_Folder:              return i18n("Rename folder");
    case UMLObject::ot_ForeignKeyConstraint:return i18n("Rename foreign key constraint");
    case UMLObject::ot_Instance:            return i18n("Rename instance");
    case UMLObject::ot_InstanceAttribute:   return i18n("Rename instance attribute");
    case UMLObject::ot_Interface:           return i18n("Rename interface");
    case UMLObject::ot_Node:                return i18n("Rename node");
    case UMLObject::ot_Operation:           return i18n("Rename operation");
    case UMLObject::ot_Package:             return i18n("Rename package");
    case UMLObject::ot_Port:                return i18n("Rename port");
    case UMLObject::ot_Role:                return i18n("Rename role");
    case UMLObject::ot_Stereotype:          return i18n("Rename stereotype");
    case UMLObject::ot_Template:            return i18n("Rename template");
    case UMLObject::ot_UniqueConstraint:    return i18n("Rename unique constraint");
    case UMLObject::ot_UseCase:             return i18n("Rename use case");
    default:
        logWarn1("Model_Utils::renameTitle unknown object type %1", UMLObject::toString(type));
        return i18n("Rename UML object");
    }
}

/**
 * Returns translated text string used by uml object related dialogs
 * @param type uml object type
 * @return translated text string
 */
QString renameText(UMLObject::ObjectType type)
{
    switch(type) {
    case UMLObject::ot_Actor:               return i18n("Enter the new name of the actor:");
    case UMLObject::ot_Artifact:            return i18n("Enter the new name of the artifact:");
    case UMLObject::ot_Association:         return i18n("Enter the new name of the association:");
    case UMLObject::ot_Attribute:           return i18n("Enter the new name of the attribute:");
    case UMLObject::ot_Category:            return i18n("Enter the new name of the category:");
    case UMLObject::ot_CheckConstraint:     return i18n("Enter the new name of the check constraint:");
    case UMLObject::ot_Class:               return i18n("Enter the new name of the class:");
    case UMLObject::ot_Component:           return i18n("Enter the new name of the component:");
    case UMLObject::ot_Datatype:            return i18n("Enter the new name of the datatype:");
    case UMLObject::ot_Entity:              return i18n("Enter the new name of the entity:");
    case UMLObject::ot_EntityAttribute:     return i18n("Enter the new name of the entity attribute:");
    case UMLObject::ot_EntityConstraint:    return i18n("Enter the new name of the entity constraint:");
    case UMLObject::ot_Enum:                return i18n("Enter the new name of the enum:");
    case UMLObject::ot_EnumLiteral:         return i18n("Enter the new name of the enum literal:");
    case UMLObject::ot_Folder:              return i18n("Enter the new name of the folder:");
    case UMLObject::ot_ForeignKeyConstraint:return i18n("Enter the new name of the foreign key constraint:");
    case UMLObject::ot_Instance:            return i18n("Enter the new name of the instance:");
    case UMLObject::ot_InstanceAttribute:   return i18n("Enter the new name of the instance attribute:");
    case UMLObject::ot_Interface:           return i18n("Enter the new name of the interface:");
    case UMLObject::ot_Node:                return i18n("Enter the new name of the node:");
    case UMLObject::ot_Operation:           return i18n("Enter the new name of the operation:");
    case UMLObject::ot_Package:             return i18n("Enter the new name of the package:");
    case UMLObject::ot_Port:                return i18n("Enter the new name of the port:");
    case UMLObject::ot_Role:                return i18n("Enter the new name of the role:");
    case UMLObject::ot_Stereotype:          return i18n("Enter the new name of the stereotype:");
    case UMLObject::ot_Template:            return i18n("Enter the new name of the template:");
    case UMLObject::ot_UniqueConstraint:    return i18n("Enter the new name of the unique constraint:");
    case UMLObject::ot_UseCase:             return i18n("Enter the new name of the use case:");
    default:
        logWarn1("Model_Utils::renameText unknown object type %1", UMLObject::toString(type));
        return i18n("Enter the new name of the UML object");
    }
}

/**
 * Return the xmi.id (XMI-1) or xmi:id (XMI-2) of a QDomElement.
 */
QString getXmiId(QDomElement element)
{
    QString idStr = element.attribute(QStringLiteral("xmi:id"));
    if (idStr.isEmpty())
        idStr = element.attribute(QStringLiteral("xmi.id"));
    return idStr;
}

/**
 * Return the text of an \<ownedComment\> XMI element from a QDomElement.
 */
QString loadCommentFromXMI(QDomElement elem)
{
    QString body = elem.attribute(QStringLiteral("body"));
    if (body.isEmpty()) {
        QDomNode innerNode = elem.firstChild();
        QDomElement innerElem = innerNode.toElement();
        while (!innerElem.isNull()) {
            QString innerTag = innerElem.tagName();
            if (UMLDoc::tagEq(innerTag, QStringLiteral("body"))) {
                body = innerElem.text();
                break;
            }
            innerNode = innerNode.nextSibling();
            innerElem = innerNode.toElement();
        }
    }
    return body;
}

/**
 * Return true if the given tag is one of the common XMI
 * attributes, such as:
 * "name" | "visibility" | "isRoot" | "isLeaf" | "isAbstract" |
 * "isActive" | "ownerScope"
 */
bool isCommonXMI1Attribute(const QString &tag)
{
    bool retval = (UMLDoc::tagEq(tag, QStringLiteral("name")) ||
                   UMLDoc::tagEq(tag, QStringLiteral("visibility")) ||
                   UMLDoc::tagEq(tag, QStringLiteral("isRoot")) ||
                   UMLDoc::tagEq(tag, QStringLiteral("isLeaf")) ||
                   UMLDoc::tagEq(tag, QStringLiteral("isAbstract")) ||
                   UMLDoc::tagEq(tag, QStringLiteral("isSpecification")) ||
                   UMLDoc::tagEq(tag, QStringLiteral("isActive")) ||
                   UMLDoc::tagEq(tag, QStringLiteral("namespace")) ||
                   UMLDoc::tagEq(tag, QStringLiteral("ownerScope")) ||
                   UMLDoc::tagEq(tag, QStringLiteral("ModelElement.stereotype")) ||
                   UMLDoc::tagEq(tag, QStringLiteral("GeneralizableElement.generalization")) ||
                   UMLDoc::tagEq(tag, QStringLiteral("specialization")) ||   //NYI
                   UMLDoc::tagEq(tag, QStringLiteral("clientDependency")) || //NYI
                   UMLDoc::tagEq(tag, QStringLiteral("supplierDependency"))  //NYI
                 );
    return retval;
}

/**
 * Return true if the given type is common among the majority
 * of programming languages, such as "bool" or "boolean".
 */
bool isCommonDataType(QString type)
{
    CodeGenerator *gen = UMLApp::app()->generator();
    if (gen == nullptr) {
        // When no code generator is set we use UMLPrimitiveTypes
        for (int i = 0; i < Uml::PrimitiveTypes::Reserved; i++) {
            if (type == Uml::PrimitiveTypes::toString(i))
                return true;
        }
        return false;
    }
    const bool caseSensitive = UMLApp::app()->activeLanguageIsCaseSensitive();
    const QStringList dataTypes = gen->defaultDatatypes();
    QStringList::ConstIterator end(dataTypes.end());
    for (QStringList::ConstIterator it = dataTypes.begin(); it != end; ++it) {
        if (caseSensitive) {
            if (type == *it)
                return true;
        } else if (type.toLower() == (*it).toLower()) {
            return true;
        }
    }
    return false;
}

/**
 * Return true if the given object type is a classifier list item type.
 */
bool isClassifierListitem(UMLObject::ObjectType type)
{
    if (type == UMLObject::ot_Attribute ||
        type == UMLObject::ot_Operation ||
        type == UMLObject::ot_Template ||
        type == UMLObject::ot_EntityAttribute ||
        type == UMLObject::ot_EnumLiteral ||
        type == UMLObject::ot_UniqueConstraint ||
        type == UMLObject::ot_ForeignKeyConstraint  ||
        type == UMLObject::ot_CheckConstraint) {
        // UMLObject::ot_InstanceAttribute needs to be handled separately
        // because UMLInstanceAttribute is not a UMLClassifierListItem.
        return true;
    } else {
        return false;
    }
}

/**
 * Try to guess the correct container folder type of a UMLObject.
 * Object types that can't be guessed are mapped to Uml::ModelType::Logical.
 * NOTE: This function exists mainly for handling pre-1.5.5 files
 *       and should not be used for new code.
 */
Uml::ModelType::Enum guessContainer(UMLObject *o)
{
    UMLObject::ObjectType ot = o->baseType();
    if (ot == UMLObject::ot_Package && o->stereotype() == QStringLiteral("subsystem"))
        return Uml::ModelType::Component;
    Uml::ModelType::Enum mt = Uml::ModelType::N_MODELTYPES;
    switch (ot) {
        case UMLObject::ot_Package:   // trouble: package can also appear in Component view
        case UMLObject::ot_Interface:
        case UMLObject::ot_Datatype:
        case UMLObject::ot_Enum:
        case UMLObject::ot_Class:
        case UMLObject::ot_Attribute:
        case UMLObject::ot_Operation:
        case UMLObject::ot_EnumLiteral:
        case UMLObject::ot_Template:
        case UMLObject::ot_Instance:
        case UMLObject::ot_InstanceAttribute:
            mt = Uml::ModelType::Logical;
            break;
        case UMLObject::ot_Actor:
        case UMLObject::ot_UseCase:
            mt = Uml::ModelType::UseCase;
            break;
        case UMLObject::ot_Component:
        case UMLObject::ot_Port:
        case UMLObject::ot_Artifact:  // trouble: artifact can also appear at Deployment
            mt = Uml::ModelType::Component;
            break;
        case UMLObject::ot_Node:
            mt = Uml::ModelType::Deployment;
            break;
        case UMLObject::ot_Entity:
        case UMLObject::ot_EntityAttribute:
        case UMLObject::ot_UniqueConstraint:
        case UMLObject::ot_ForeignKeyConstraint:
        case UMLObject::ot_CheckConstraint:
        case UMLObject::ot_Category:
            mt = Uml::ModelType::EntityRelationship;
            break;
        case UMLObject::ot_Association:
            {
                UMLAssociation *assoc = o->asUMLAssociation();
                UMLDoc *umldoc = UMLApp::app()->document();
                for (int r = Uml::RoleType::A; r <= Uml::RoleType::B; ++r) {
                    UMLObject *roleObj = assoc->getObject(Uml::RoleType::fromInt(r));
                    if (roleObj == nullptr) {
                        // Ouch! we have been called while types are not yet resolved
                        return Uml::ModelType::N_MODELTYPES;
                    }
                    UMLPackage *pkg = roleObj->umlPackage();
                    if (pkg) {
                        while (pkg->umlPackage()) {  // wind back to root
                            pkg = pkg->umlPackage();
                        }
                        const Uml::ModelType::Enum m = umldoc->rootFolderType(pkg);
                        if (m != Uml::ModelType::N_MODELTYPES)
                            return m;
                    }
                    mt = guessContainer(roleObj);
                    if (mt != Uml::ModelType::Logical)
                        break;
                }
            }
            break;
        default:
            break;
    }
    return mt;
}

/**
 * Parse a direction string into the Uml::ParameterDirection::Enum.
 *
 * @param input  The string to parse: "in", "out", or "inout"
 *               optionally followed by whitespace.
 * @param result The corresponding Uml::ParameterDirection::Enum.
 * @return       Length of the string matched, excluding the optional
 *               whitespace.
 */
int stringToDirection(QString input, Uml::ParameterDirection::Enum & result)
{
    QRegExp dirx(QStringLiteral("^(in|out|inout)"));
    int pos = dirx.indexIn(input);
    if (pos == -1)
        return 0;
    const QString dirStr = dirx.capturedTexts().first();
    int dirLen = dirStr.length();
    if (input.length() > dirLen && !input[dirLen].isSpace())
        return 0;       // no match after all.
    if (dirStr == QStringLiteral("out"))
        result = Uml::ParameterDirection::Out;
    else if (dirStr == QStringLiteral("inout"))
        result = Uml::ParameterDirection::InOut;
    else
        result = Uml::ParameterDirection::In;
    return dirLen;
}

/**
 * Parses a template parameter given in UML syntax.
 *
 * @param t             Input text of the template parameter.
 *                      Example:  parname : partype
 *                      or just:  parname          (for class type)
 * @param nmTp          NameAndType returned by this method.
 * @param owningScope   Pointer to the owning scope of the template param.
 * @return      Error status of the parse, PS_OK for success.
 */
Parse_Status parseTemplate(QString t, NameAndType& nmTp, UMLClassifier *owningScope)
{
    UMLDoc *pDoc = UMLApp::app()->document();

    t = t.trimmed();
    if (t.isEmpty())
        return PS_Empty;

    QStringList nameAndType = t.split(QRegExp(QStringLiteral("\\s*:\\s*")));
    if (nameAndType.count() == 2) {
        UMLObject *pType = nullptr;
        if (nameAndType[1] != QStringLiteral("class")) {
            pType = pDoc->findUMLObject(nameAndType[1], UMLObject::ot_UMLObject, owningScope);
            if (pType == nullptr)
                return PS_Unknown_ArgType;
        }
        nmTp = NameAndType(nameAndType[0], pType);
    } else {
        nmTp = NameAndType(t, nullptr);
    }
    return PS_OK;
}

/**
 * Parses an attribute given in UML syntax.
 *
 * @param a             Input text of the attribute in UML syntax.
 *                      Example:  argname : argtype
 * @param nmTp          NameAndType returned by this method.
 * @param owningScope   Pointer to the owning scope of the attribute.
 * @param vis           Optional pointer to visibility (return value.)
 *                      The visibility may be given at the beginning of the
 *                      attribute text in mnemonic form as follows:
 *                      "+"  stands for public
 *                      "#"  stands for protected
 *                      "-"  stands for private
 *                      "~"  stands for implementation level visibility
 *
 * @return      Error status of the parse, PS_OK for success.
 */
Parse_Status parseAttribute(QString a, NameAndType& nmTp, UMLClassifier *owningScope,
                            Uml::Visibility::Enum *vis /* = nullptr */)
{
    UMLDoc *pDoc = UMLApp::app()->document();

    a = a.simplified();
    if (a.isEmpty())
        return PS_Empty;

    int colonPos = a.indexOf(QLatin1Char(':'));
    if (colonPos < 0) {
        nmTp = NameAndType(a, nullptr);
        return PS_OK;
    }
    QString name = a.left(colonPos).trimmed();
    if (vis) {
        QRegExp mnemonicVis(QStringLiteral("^([\\+\\#\\-\\~] *)"));
        int pos = mnemonicVis.indexIn(name);
        if (pos == -1) {
            *vis = Uml::Visibility::Private;  // default value
        } else {
            QString caption = mnemonicVis.cap(1);
            QString strVis = caption.left(1);
            if (strVis == QStringLiteral("+"))
                *vis = Uml::Visibility::Public;
            else if (strVis == QStringLiteral("#"))
                *vis = Uml::Visibility::Protected;
            else if (strVis == QStringLiteral("-"))
                *vis = Uml::Visibility::Private;
            else
                *vis = Uml::Visibility::Implementation;
        }
        name.remove(mnemonicVis);
    }
    Uml::ParameterDirection::Enum pd = Uml::ParameterDirection::In;
    if (name.startsWith(QStringLiteral("in "))) {
        pd = Uml::ParameterDirection::In;
        name = name.mid(3);
    } else if (name.startsWith(QStringLiteral("inout "))) {
        pd = Uml::ParameterDirection::InOut;
        name = name.mid(6);
    } else if (name.startsWith(QStringLiteral("out "))) {
        pd = Uml::ParameterDirection::Out;
        name = name.mid(4);
    }
    a = a.mid(colonPos + 1).trimmed();
    if (a.isEmpty()) {
        nmTp = NameAndType(name, nullptr, pd);
        return PS_OK;
    }
    QStringList typeAndInitialValue = a.split(QRegExp(QStringLiteral("\\s*=\\s*")));
    const QString &type = typeAndInitialValue[0];
    UMLObject *pType = pDoc->findUMLObject(type, UMLObject::ot_UMLObject, owningScope);
    if (pType == nullptr) {
        nmTp = NameAndType(name, nullptr, pd);
        return PS_Unknown_ArgType;
    }
    QString initialValue;
    if (typeAndInitialValue.count() == 2) {
        initialValue = typeAndInitialValue[1];
    }
    nmTp = NameAndType(name, pType, pd, initialValue);
    return PS_OK;
}

/**
 * Parses an operation given in UML syntax.
 *
 * @param m             Input text of the operation in UML syntax.
 *                      Example of a two-argument operation returning "void":
 *                      methodname (arg1name : arg1type, arg2name : arg2type) : void
 * @param desc          OpDescriptor returned by this method.
 * @param owningScope   Pointer to the owning scope of the operation.
 * @return      Error status of the parse, PS_OK for success.
 */
Parse_Status parseOperation(QString m, OpDescriptor& desc, UMLClassifier *owningScope)
{
    UMLDoc *pDoc = UMLApp::app()->document();

    m = m.simplified();
    if (m.isEmpty())
        return PS_Empty;
    if (m.contains(QRegExp(QStringLiteral("operator *()")))) {
        // C++ special case: two sets of parentheses
        desc.m_name = QStringLiteral("operator()");
        m.remove(QRegExp(QStringLiteral("operator *()")));
    } else {
        /**
         * The search pattern includes everything up to the opening parenthesis
         * because UML also permits non programming-language oriented designs
         * using narrative names, for example "check water temperature".
         */
        QRegExp beginningUpToOpenParenth(QStringLiteral("^([^\\(]+)"));
        int pos = beginningUpToOpenParenth.indexIn(m);
        if (pos == -1)
            return PS_Illegal_MethodName;
        desc.m_name = beginningUpToOpenParenth.cap(1);
    }
    desc.m_pReturnType = nullptr;
    QRegExp pat = QRegExp(QStringLiteral("\\) *:(.*)$"));
    int pos = pat.indexIn(m);
    if (pos != -1) {  // return type is optional
        QString retType = pat.cap(1);
        retType = retType.trimmed();
        if (retType != QStringLiteral("void")) {
            UMLObject *pRetType = owningScope ? owningScope->findTemplate(retType) : nullptr;
            if (pRetType == nullptr) {
                pRetType = pDoc->findUMLObject(retType, UMLObject::ot_UMLObject, owningScope);
                if (pRetType == nullptr)
                    return PS_Unknown_ReturnType;
            }
            desc.m_pReturnType = pRetType;
        }
    }
    // Remove possible empty parentheses ()
    m.remove(QRegExp(QStringLiteral("\\s*\\(\\s*\\)")));
    desc.m_args.clear();
    pat = QRegExp(QStringLiteral("\\((.*)\\)"));
    pos = pat.indexIn(m);
    if (pos == -1)  // argument list is optional
        return PS_OK;
    QString arglist = pat.cap(1);
    arglist = arglist.trimmed();
    if (arglist.isEmpty())
        return PS_OK;
    const QStringList args = arglist.split(QRegExp(QStringLiteral("\\s*, \\s*")));
    for (QStringList::ConstIterator lit = args.begin(); lit != args.end(); ++lit) {
        NameAndType nmTp;
        Parse_Status ps = parseAttribute(*lit, nmTp, owningScope);
        if (ps)
            return ps;
        desc.m_args.append(nmTp);
    }
    return PS_OK;
}

/**
 * Parses a constraint.
 *
 * @param m             Input text of the constraint
 *
 * @param name          The name returned by this method
 * @param owningScope   Pointer to the owning scope of the constraint
 * @return       Error status of the parse, PS_OK for success.
 */
Parse_Status parseConstraint(QString m, QString& name, UMLEntity* owningScope)
{
    Q_UNUSED(owningScope);
    m = m.simplified();
    if (m.isEmpty())
        return PS_Empty;

    int colonPos = m.indexOf(QLatin1Char(':'));
    if (colonPos < 0) {
        name = m;
        return PS_OK;
    }

    name = m.left(colonPos).trimmed();
    return PS_OK;
}

/**
 * Returns the Parse_Status as a text.
 */
QString psText(Parse_Status value)
{
    const QString text[] = {
                               i18n("OK"), i18nc("parse status", "Empty"), i18n("Malformed argument"),
                               i18n("Unknown argument type"), i18n("Illegal method name"),
                               i18n("Unknown return type"), i18n("Unspecified error")
                           };
    return text[(unsigned) value];
}

/**
 * Return true if the listview type is one of the predefined root views
 * (root, logical, usecase, component, deployment, datatype, or entity-
 * relationship view.)
 */
bool typeIsRootView(UMLListViewItem::ListViewType type)
{
    switch (type) {
        case UMLListViewItem::lvt_View:
        case UMLListViewItem::lvt_Logical_View:
        case UMLListViewItem::lvt_UseCase_View:
        case UMLListViewItem::lvt_Component_View:
        case UMLListViewItem::lvt_Deployment_View:
        case UMLListViewItem::lvt_EntityRelationship_Model:
            return true;
            break;
        default:
            break;
    }
    return false;
}

/**
 * Return true if the listview type also has a widget representation in diagrams.
 */
bool typeIsCanvasWidget(UMLListViewItem::ListViewType type)
{
    switch (type) {
        case UMLListViewItem::lvt_Actor:
        case UMLListViewItem::lvt_UseCase:
        case UMLListViewItem::lvt_Class:
        case UMLListViewItem::lvt_Package:
        case UMLListViewItem::lvt_Logical_Folder:
        case UMLListViewItem::lvt_UseCase_Folder:
        case UMLListViewItem::lvt_Component_Folder:
        case UMLListViewItem::lvt_Deployment_Folder:
        case UMLListViewItem::lvt_EntityRelationship_Folder:
        case UMLListViewItem::lvt_Subsystem:
        case UMLListViewItem::lvt_Component:
        case UMLListViewItem::lvt_Port:
        case UMLListViewItem::lvt_Node:
        case UMLListViewItem::lvt_Artifact:
        case UMLListViewItem::lvt_Interface:
        case UMLListViewItem::lvt_Datatype:
        case UMLListViewItem::lvt_Enum:
        case UMLListViewItem::lvt_Instance:
        case UMLListViewItem::lvt_Entity:
        case UMLListViewItem::lvt_Category:
            return true;
            break;
        default:
            break;
    }
    return false;
}

/**
 * Return true if the listview type is a logical, usecase or component folder.
 */
bool typeIsFolder(UMLListViewItem::ListViewType type)
{
    if (typeIsRootView(type) ||
            type == UMLListViewItem::lvt_Datatype_Folder ||
            type == UMLListViewItem::lvt_Logical_Folder ||
            type == UMLListViewItem::lvt_UseCase_Folder ||
            type == UMLListViewItem::lvt_Component_Folder ||
            type == UMLListViewItem::lvt_Deployment_Folder ||
            type == UMLListViewItem::lvt_EntityRelationship_Folder) {
        return true;
    } else {
        return false;
    }
}

/**
 * Return true if the listview type may act as a container for other objects,
 * i.e. if it is a folder, package, subsystem, or component.
 */
bool typeIsContainer(UMLListViewItem::ListViewType type)
{
    if (typeIsFolder(type))
        return true;
    return (type == UMLListViewItem::lvt_Package ||
            type == UMLListViewItem::lvt_Subsystem ||
            type == UMLListViewItem::lvt_Component ||
            type == UMLListViewItem::lvt_Class ||
            type == UMLListViewItem::lvt_Interface);
}

/**
 * Return true if the listview type is an attribute, operation, or template.
 */
bool typeIsClassifierList(UMLListViewItem::ListViewType type)
{
    if (type == UMLListViewItem::lvt_Attribute ||
        type == UMLListViewItem::lvt_Operation ||
        type == UMLListViewItem::lvt_Template ||
        type == UMLListViewItem::lvt_EntityAttribute ||
        type == UMLListViewItem::lvt_UniqueConstraint ||
        type == UMLListViewItem::lvt_ForeignKeyConstraint ||
        type == UMLListViewItem::lvt_PrimaryKeyConstraint ||
        type == UMLListViewItem::lvt_CheckConstraint  ||
        type == UMLListViewItem::lvt_EnumLiteral) {
        //  UMLListViewItem::lvt_InstanceAttribute must be handled separately
        //  because UMLInstanceAttribute is not a UMLClassifierListItem.
        return true;
    } else {
        return false;
    }
}

/**
 * Return true if the listview type is a classifier (Class, Entity, Enum)
 */
bool typeIsClassifier(UMLListViewItem::ListViewType type)
{
    if (type == UMLListViewItem::lvt_Class ||
         type == UMLListViewItem::lvt_Interface ||
         type == UMLListViewItem::lvt_Entity ||
         type == UMLListViewItem::lvt_Enum) {
        return true;
    }
    return false;
}

/**
 * Return true if the listview type is a settings entry.
 */
bool typeIsProperties(UMLListViewItem::ListViewType type)
{
    switch (type) {
    case UMLListViewItem::lvt_Properties:
    case UMLListViewItem::lvt_Properties_AutoLayout:
    case UMLListViewItem::lvt_Properties_Class:
    case UMLListViewItem::lvt_Properties_CodeImport:
    case UMLListViewItem::lvt_Properties_CodeGeneration:
    case UMLListViewItem::lvt_Properties_CodeViewer:
    case UMLListViewItem::lvt_Properties_Font:
    case UMLListViewItem::lvt_Properties_General:
    case UMLListViewItem::lvt_Properties_UserInterface:
        return true;
        break;
    default:
        break;
    }
    return false;
}

/**
 * Check if a listviewitem of type childType is allowed
 * as child of type parentType
 */
bool typeIsAllowedInType(UMLListViewItem::ListViewType childType,
                         UMLListViewItem::ListViewType parentType)
{
    switch (childType) {
    case UMLListViewItem::lvt_Class:
    case UMLListViewItem::lvt_Package:
    case UMLListViewItem::lvt_Interface:
    case UMLListViewItem::lvt_Enum:
    case UMLListViewItem::lvt_Instance:
        return parentType == UMLListViewItem::lvt_Logical_View ||
               parentType == UMLListViewItem::lvt_Class ||
               parentType == UMLListViewItem::lvt_Package ||
               parentType == UMLListViewItem::lvt_Logical_Folder;
    case UMLListViewItem::lvt_Attribute:
        return parentType == UMLListViewItem::lvt_Class;
    case UMLListViewItem::lvt_EnumLiteral:
        return parentType == UMLListViewItem::lvt_Enum;
    case UMLListViewItem::lvt_EntityAttribute:
        return parentType == UMLListViewItem::lvt_Entity;
    case UMLListViewItem::lvt_InstanceAttribute:
        return parentType == UMLListViewItem::lvt_Instance;
    case UMLListViewItem::lvt_Operation:
        return parentType == UMLListViewItem::lvt_Class ||
               parentType == UMLListViewItem::lvt_Interface;
    case UMLListViewItem::lvt_Datatype:
        return parentType == UMLListViewItem::lvt_Logical_Folder ||
               parentType == UMLListViewItem::lvt_Datatype_Folder ||
               parentType == UMLListViewItem::lvt_Class ||
               parentType == UMLListViewItem::lvt_Interface ||
               parentType == UMLListViewItem::lvt_Package;
    case UMLListViewItem::lvt_Class_Diagram:
    case UMLListViewItem::lvt_Collaboration_Diagram:
    case UMLListViewItem::lvt_State_Diagram:
    case UMLListViewItem::lvt_Activity_Diagram:
    case UMLListViewItem::lvt_Sequence_Diagram:
    case UMLListViewItem::lvt_Object_Diagram:
        return parentType == UMLListViewItem::lvt_Logical_Folder ||
               parentType == UMLListViewItem::lvt_Logical_View;
    case UMLListViewItem::lvt_Logical_Folder:
        return parentType == UMLListViewItem::lvt_Package ||
               parentType == UMLListViewItem::lvt_Logical_Folder ||
               parentType == UMLListViewItem::lvt_Logical_View;
    case UMLListViewItem::lvt_UseCase_Folder:
        return parentType == UMLListViewItem::lvt_UseCase_Folder ||
               parentType == UMLListViewItem::lvt_UseCase_View;
    case UMLListViewItem::lvt_Component_Folder:
        return parentType == UMLListViewItem::lvt_Component_Folder ||
               parentType == UMLListViewItem::lvt_Component_View;
    case UMLListViewItem::lvt_Deployment_Folder:
        return parentType == UMLListViewItem::lvt_Deployment_Folder ||
               parentType == UMLListViewItem::lvt_Deployment_View;
    case UMLListViewItem::lvt_EntityRelationship_Folder:
        return parentType == UMLListViewItem::lvt_EntityRelationship_Folder ||
               parentType == UMLListViewItem::lvt_EntityRelationship_Model;
    case UMLListViewItem::lvt_Actor:
    case UMLListViewItem::lvt_UseCase:
    case UMLListViewItem::lvt_UseCase_Diagram:
        return parentType == UMLListViewItem::lvt_UseCase_Folder ||
               parentType == UMLListViewItem::lvt_UseCase_View;
    case UMLListViewItem::lvt_Subsystem:
        return parentType == UMLListViewItem::lvt_Component_Folder ||
               parentType == UMLListViewItem::lvt_Subsystem ||
               parentType == UMLListViewItem::lvt_Component_View;
    case UMLListViewItem::lvt_Component:
        return parentType == UMLListViewItem::lvt_Component_Folder ||
               parentType == UMLListViewItem::lvt_Component ||
               parentType == UMLListViewItem::lvt_Subsystem ||
               parentType == UMLListViewItem::lvt_Component_View;
    case UMLListViewItem::lvt_Port:
        return parentType == UMLListViewItem::lvt_Component ||
               parentType == UMLListViewItem::lvt_Subsystem;
    case UMLListViewItem::lvt_Artifact:
    case UMLListViewItem::lvt_Component_Diagram:
        return parentType == UMLListViewItem::lvt_Component_Folder ||
               parentType == UMLListViewItem::lvt_Component_View;
    case UMLListViewItem::lvt_Node:
    case UMLListViewItem::lvt_Deployment_Diagram:
        return parentType == UMLListViewItem::lvt_Deployment_Folder ||
               parentType == UMLListViewItem::lvt_Deployment_View;
    case UMLListViewItem::lvt_Entity:
    case UMLListViewItem::lvt_EntityRelationship_Diagram:
    case UMLListViewItem::lvt_Category:
        return parentType == UMLListViewItem::lvt_EntityRelationship_Folder ||
               parentType == UMLListViewItem::lvt_EntityRelationship_Model;
    default:
        return false;
    }
}

/**
 * Return true if the listview type is a diagram.
 */
bool typeIsDiagram(UMLListViewItem::ListViewType type)
{
    if (type == UMLListViewItem::lvt_Class_Diagram ||
            type == UMLListViewItem::lvt_Collaboration_Diagram ||
            type == UMLListViewItem::lvt_State_Diagram ||
            type == UMLListViewItem::lvt_Activity_Diagram ||
            type == UMLListViewItem::lvt_Sequence_Diagram ||
            type == UMLListViewItem::lvt_UseCase_Diagram ||
            type == UMLListViewItem::lvt_Component_Diagram ||
            type == UMLListViewItem::lvt_Deployment_Diagram ||
            type == UMLListViewItem::lvt_EntityRelationship_Diagram ||
            type == UMLListViewItem::lvt_Object_Diagram) {
        return true;
    } else {
        return false;
    }
}

/**
 * Return the Model_Type which corresponds to the given DiagramType.
 */
Uml::ModelType::Enum convert_DT_MT(Uml::DiagramType::Enum dt)
{
    Uml::ModelType::Enum mt;
    switch (dt) {
        case Uml::DiagramType::UseCase:
            mt = Uml::ModelType::UseCase;
            break;
        case Uml::DiagramType::Collaboration:
        case Uml::DiagramType::Class:
        case Uml::DiagramType::Object:
        case Uml::DiagramType::Sequence:
        case Uml::DiagramType::State:
        case Uml::DiagramType::Activity:
            mt = Uml::ModelType::Logical;
            break;
        case Uml::DiagramType::Component:
            mt = Uml::ModelType::Component;
            break;
        case Uml::DiagramType::Deployment:
            mt = Uml::ModelType::Deployment;
            break;
        case Uml::DiagramType::EntityRelationship:
            mt = Uml::ModelType::EntityRelationship;
            break;
        default:
            logError1("Model_Utils::convert_DT_MT: illegal input value %1", dt);
            mt = Uml::ModelType::N_MODELTYPES;
            break;
    }
    return mt;
}

/**
 * Return the ListViewType which corresponds to the given Model_Type.
 */
UMLListViewItem::ListViewType convert_MT_LVT(Uml::ModelType::Enum mt)
{
    UMLListViewItem::ListViewType lvt = UMLListViewItem::lvt_Unknown;
    switch (mt) {
        case Uml::ModelType::Logical:
            lvt = UMLListViewItem::lvt_Logical_View;
            break;
        case Uml::ModelType::UseCase:
            lvt = UMLListViewItem::lvt_UseCase_View;
            break;
        case Uml::ModelType::Component:
            lvt = UMLListViewItem::lvt_Component_View;
            break;
        case Uml::ModelType::Deployment:
            lvt = UMLListViewItem::lvt_Deployment_View;
            break;
        case Uml::ModelType::EntityRelationship:
            lvt = UMLListViewItem::lvt_EntityRelationship_Model;
            break;
        default:
            break;
    }
    return lvt;
}

/**
 * Return the Model_Type which corresponds to the given ListViewType.
 * Returns Uml::N_MODELTYPES if the list view type given does not map
 * to a Model_Type.
 */
Uml::ModelType::Enum convert_LVT_MT(UMLListViewItem::ListViewType lvt)
{
    Uml::ModelType::Enum mt = Uml::ModelType::N_MODELTYPES;
    switch (lvt) {
        case UMLListViewItem::lvt_Logical_View:
            mt = Uml::ModelType::Logical;
            break;
        case UMLListViewItem::lvt_UseCase_View:
            mt = Uml::ModelType::UseCase;
            break;
        case UMLListViewItem::lvt_Component_View:
            mt = Uml::ModelType::Component;
            break;
        case UMLListViewItem::lvt_Deployment_View:
            mt = Uml::ModelType::Deployment;
            break;
        case UMLListViewItem::lvt_EntityRelationship_Model:
            mt = Uml::ModelType::EntityRelationship;
            break;
        default:
            break;
    }
    return mt;
}

/**
 * Convert a diagram type enum to the equivalent list view type.
 */
UMLListViewItem::ListViewType convert_DT_LVT(Uml::DiagramType::Enum dt)
{
    UMLListViewItem::ListViewType type =  UMLListViewItem::lvt_Unknown;
    switch(dt) {
    case Uml::DiagramType::UseCase:
        type = UMLListViewItem::lvt_UseCase_Diagram;
        break;

    case Uml::DiagramType::Class:
        type = UMLListViewItem::lvt_Class_Diagram;
        break;

    case Uml::DiagramType::Object:
        type = UMLListViewItem::lvt_Object_Diagram;
        break;

    case Uml::DiagramType::Sequence:
        type = UMLListViewItem::lvt_Sequence_Diagram;
        break;

    case Uml::DiagramType::Collaboration:
        type = UMLListViewItem::lvt_Collaboration_Diagram;
        break;

    case Uml::DiagramType::State:
        type = UMLListViewItem::lvt_State_Diagram;
        break;

    case Uml::DiagramType::Activity:
        type = UMLListViewItem::lvt_Activity_Diagram;
        break;

    case Uml::DiagramType::Component:
        type = UMLListViewItem::lvt_Component_Diagram;
        break;

    case Uml::DiagramType::Deployment:
        type = UMLListViewItem::lvt_Deployment_Diagram;
        break;

    case Uml::DiagramType::EntityRelationship:
        type = UMLListViewItem::lvt_EntityRelationship_Diagram;
        break;

    default:
        logWarn1("Model_Utils::convert_DT_LVT() called on unknown diagram type %1", dt);
    }
    return type;
}

/**
 * Convert an object's type to the equivalent list view type
 *
 * @param o  Pointer to the UMLObject whose type shall be converted
 *           to the equivalent ListViewType.  We cannot just
 *           pass in a UMLObject::ObjectType because a UMLFolder is mapped
 *           to different ListViewType values, depending on its
 *           location in one of the predefined modelviews (Logical/
 *           UseCase/etc.)
 * @return  The equivalent ListViewType.
 */
UMLListViewItem::ListViewType convert_OT_LVT(UMLObject *o)
{
    UMLObject::ObjectType ot = o->baseType();
    UMLListViewItem::ListViewType type =  UMLListViewItem::lvt_Unknown;
    switch(ot) {
    case UMLObject::ot_UseCase:
        type = UMLListViewItem::lvt_UseCase;
        break;

    case UMLObject::ot_Actor:
        type = UMLListViewItem::lvt_Actor;
        break;

    case UMLObject::ot_Class:
        type = UMLListViewItem::lvt_Class;
        break;

    case UMLObject::ot_Package:
        if (o->stereotype() == QStringLiteral("subsystem"))
            type = UMLListViewItem::lvt_Subsystem;
        else
            type = UMLListViewItem::lvt_Package;
        break;

    case UMLObject::ot_Folder:
        {
            UMLDoc *umldoc = UMLApp::app()->document();
            UMLPackage *p = o->asUMLPackage();
            do {
                const Uml::ModelType::Enum mt = umldoc->rootFolderType(p);
                if (mt != Uml::ModelType::N_MODELTYPES) {
                    switch (mt) {
                        case Uml::ModelType::Logical:
                            type = UMLListViewItem::lvt_Logical_Folder;
                            break;
                        case Uml::ModelType::UseCase:
                            type = UMLListViewItem::lvt_UseCase_Folder;
                            break;
                        case Uml::ModelType::Component:
                            type = UMLListViewItem::lvt_Component_Folder;
                            break;
                        case Uml::ModelType::Deployment:
                            type = UMLListViewItem::lvt_Deployment_Folder;
                            break;
                        case Uml::ModelType::EntityRelationship:
                            type = UMLListViewItem::lvt_EntityRelationship_Folder;
                            break;
                        default:
                            break;
                    }
                    return type;
                }
            } while ((p = p->umlPackage()) != nullptr);
            logError1("Model_Utils::convert_OT_LVT(%1): internal error - "
                      "object is not properly nested in folder", o->name());
        }
        break;

    case UMLObject::ot_Component:
        type = UMLListViewItem::lvt_Component;
        break;

    case UMLObject::ot_Port:
        type = UMLListViewItem::lvt_Port;
        break;

    case UMLObject::ot_Node:
        type = UMLListViewItem::lvt_Node;
        break;

    case UMLObject::ot_Artifact:
        type = UMLListViewItem::lvt_Artifact;
        break;

    case UMLObject::ot_Interface:
        type = UMLListViewItem::lvt_Interface;
        break;

    case UMLObject::ot_Datatype:
        type = UMLListViewItem::lvt_Datatype;
        break;

    case UMLObject::ot_Enum:
        type = UMLListViewItem::lvt_Enum;
        break;

    case UMLObject::ot_EnumLiteral:
        type = UMLListViewItem::lvt_EnumLiteral;
        break;

    case UMLObject::ot_Entity:
        type = UMLListViewItem::lvt_Entity;
        break;

    case UMLObject::ot_Category:
        type = UMLListViewItem::lvt_Category;
        break;

    case UMLObject::ot_EntityAttribute:
        type = UMLListViewItem::lvt_EntityAttribute;
        break;

    case UMLObject::ot_UniqueConstraint: {
         const UMLEntity* ent = o->umlParent()->asUMLEntity();
         const UMLUniqueConstraint* uc = o->asUMLUniqueConstraint();
         if (ent->isPrimaryKey(uc)) {
             type = UMLListViewItem::lvt_PrimaryKeyConstraint;
         } else {
             type = UMLListViewItem::lvt_UniqueConstraint;
         }
         break;
        }

    case UMLObject::ot_ForeignKeyConstraint:
        type = UMLListViewItem::lvt_ForeignKeyConstraint;
        break;

    case UMLObject::ot_CheckConstraint:
        type = UMLListViewItem::lvt_CheckConstraint;
        break;

    case UMLObject::ot_Attribute:
        type = UMLListViewItem::lvt_Attribute;
        break;

    case UMLObject::ot_Operation:
        type = UMLListViewItem::lvt_Operation;
        break;

    case UMLObject::ot_Template:
        type = UMLListViewItem::lvt_Template;
        break;

    case UMLObject::ot_Association:
        type = UMLListViewItem::lvt_Association;
        break;

    case UMLObject::ot_Instance:
        type = UMLListViewItem::lvt_Instance;
        break;

    case UMLObject::ot_InstanceAttribute:
        type = UMLListViewItem::lvt_InstanceAttribute;
        break;

    default:
        break;
    }
    return type;
}

/**
 * Converts a list view type enum to the equivalent object type.
 *
 * @param lvt   The ListViewType to convert.
 * @return  The converted ObjectType if the listview type
 *          has a UMLObject::ObjectType representation, else 0.
 */
UMLObject::ObjectType convert_LVT_OT(UMLListViewItem::ListViewType lvt)
{
    UMLObject::ObjectType ot = (UMLObject::ObjectType)0;
    switch (lvt) {
    case UMLListViewItem::lvt_UseCase:
        ot = UMLObject::ot_UseCase;
        break;

    case UMLListViewItem::lvt_Actor:
        ot = UMLObject::ot_Actor;
        break;

    case UMLListViewItem::lvt_Class:
        ot = UMLObject::ot_Class;
        break;

    case UMLListViewItem::lvt_Package:
    case UMLListViewItem::lvt_Subsystem:
        ot = UMLObject::ot_Package;
        break;

    case UMLListViewItem::lvt_Component:
        ot = UMLObject::ot_Component;
        break;

    case UMLListViewItem::lvt_Port:
        ot = UMLObject::ot_Port;
        break;

    case UMLListViewItem::lvt_Node:
        ot = UMLObject::ot_Node;
        break;

    case UMLListViewItem::lvt_Artifact:
        ot = UMLObject::ot_Artifact;
        break;

    case UMLListViewItem::lvt_Interface:
        ot = UMLObject::ot_Interface;
        break;

    case UMLListViewItem::lvt_Datatype:
        ot = UMLObject::ot_Datatype;
        break;

    case UMLListViewItem::lvt_Enum:
        ot = UMLObject::ot_Enum;
        break;

    case UMLListViewItem::lvt_Entity:
        ot = UMLObject::ot_Entity;
        break;

    case UMLListViewItem::lvt_Category:
        ot = UMLObject::ot_Category;
        break;

    case UMLListViewItem::lvt_EntityAttribute:
        ot = UMLObject::ot_EntityAttribute;
        break;

    case UMLListViewItem::lvt_UniqueConstraint:
        ot = UMLObject::ot_UniqueConstraint;
        break;

    case UMLListViewItem::lvt_PrimaryKeyConstraint:
        ot = UMLObject::ot_UniqueConstraint;
        break;

    case UMLListViewItem::lvt_ForeignKeyConstraint:
        ot = UMLObject::ot_ForeignKeyConstraint;
        break;

    case UMLListViewItem::lvt_CheckConstraint:
        ot = UMLObject::ot_CheckConstraint;
        break;

    case UMLListViewItem::lvt_Attribute:
        ot = UMLObject::ot_Attribute;
        break;

    case UMLListViewItem::lvt_Operation:
        ot = UMLObject::ot_Operation;
        break;

    case UMLListViewItem::lvt_Template:
        ot = UMLObject::ot_Template;
        break;

    case UMLListViewItem::lvt_EnumLiteral:
        ot = UMLObject::ot_EnumLiteral;
        break;

    case UMLListViewItem::lvt_Instance:
        ot = UMLObject::ot_Instance;
        break;

    case UMLListViewItem::lvt_InstanceAttribute:
        ot = UMLObject::ot_InstanceAttribute;
        break;

    default:
        if (typeIsFolder(lvt))
            ot = UMLObject::ot_Folder;
        break;
    }
    return ot;
}

/**
 * Return the IconType which corresponds to the given listview type.
 *
 * @param lvt  ListViewType to convert.
 * @param o    Optional UMLObject pointer is only used if @p lvt is lvt_Class:
 *             If the stereotype <<class-or-package>> is applied on the object
 *             then Icon_Utils::it_ClassOrPackage is returned.
 * @return  The Icon_Utils::IconType corresponding to the lvt.
 *          Returns it_Home in case no mapping to IconType exists.
 */
Icon_Utils::IconType convert_LVT_IT(UMLListViewItem::ListViewType lvt, UMLObject *o)
{
    Icon_Utils::IconType icon = Icon_Utils::it_Home;
    switch (lvt) {
        case UMLListViewItem::lvt_UseCase_View:
        case UMLListViewItem::lvt_UseCase_Folder:
            icon = Icon_Utils::it_Folder_Grey;
            break;
        case UMLListViewItem::lvt_Logical_View:
        case UMLListViewItem::lvt_Logical_Folder:
            icon = Icon_Utils::it_Folder_Green;
            break;
        case UMLListViewItem::lvt_Datatype_Folder:
            icon = Icon_Utils::it_Folder_Orange;
            break;
        case UMLListViewItem::lvt_Component_View:
        case UMLListViewItem::lvt_Component_Folder:
            icon = Icon_Utils::it_Folder_Red;
            break;
        case UMLListViewItem::lvt_Deployment_View:
        case UMLListViewItem::lvt_Deployment_Folder:
            icon = Icon_Utils::it_Folder_Violet;
            break;
        case UMLListViewItem::lvt_EntityRelationship_Model:
        case UMLListViewItem::lvt_EntityRelationship_Folder:
            icon = Icon_Utils::it_Folder_Cyan;
            break;
        case UMLListViewItem::lvt_Actor:
            icon = Icon_Utils::it_Actor;
            break;
        case UMLListViewItem::lvt_Association:
            icon = Icon_Utils::it_Association;
            break;
        case UMLListViewItem::lvt_UseCase:
            icon = Icon_Utils::it_UseCase;
            break;
        case UMLListViewItem::lvt_Class:
            if (o && o->stereotype() == QStringLiteral("class-or-package"))
                icon = Icon_Utils::it_ClassOrPackage;
            else
                icon = Icon_Utils::it_Class;
            break;
        case UMLListViewItem::lvt_Package:
            icon = Icon_Utils::it_Package;
            break;
        case UMLListViewItem::lvt_Subsystem:
            icon = Icon_Utils::it_Subsystem;
            break;
        case UMLListViewItem::lvt_Component:
            icon = Icon_Utils::it_Component;
            break;
        case UMLListViewItem::lvt_Port:
            icon = Icon_Utils::it_Port;
            break;
        case UMLListViewItem::lvt_Node:
            icon = Icon_Utils::it_Node;
            break;
        case UMLListViewItem::lvt_Artifact:
            icon = Icon_Utils::it_Artifact;
            break;
        case UMLListViewItem::lvt_Interface:
            icon = Icon_Utils::it_Interface;
            break;
        case UMLListViewItem::lvt_Datatype:
            icon = Icon_Utils::it_Datatype;
            break;
        case UMLListViewItem::lvt_Enum:
            icon = Icon_Utils::it_Enum;
            break;
        case UMLListViewItem::lvt_Entity:
            icon = Icon_Utils::it_Entity;
            break;
        case UMLListViewItem::lvt_Category:
            icon = Icon_Utils::it_Category;
            break;
        case UMLListViewItem::lvt_Template:
            icon = Icon_Utils::it_Template;
            break;
        case UMLListViewItem::lvt_Attribute:
            icon = Icon_Utils::it_Private_Attribute;
            break;
        case UMLListViewItem::lvt_EntityAttribute:
            icon = Icon_Utils::it_Private_Attribute;
            break;
        case UMLListViewItem::lvt_EnumLiteral:
            icon = Icon_Utils::it_Public_Attribute;
            break;
        case UMLListViewItem::lvt_Operation:
            icon = Icon_Utils::it_Public_Method;
            break;
        case UMLListViewItem::lvt_UniqueConstraint:
            icon = Icon_Utils::it_Unique_Constraint;
            break;
        case UMLListViewItem::lvt_PrimaryKeyConstraint:
            icon = Icon_Utils::it_PrimaryKey_Constraint;
            break;
        case UMLListViewItem::lvt_ForeignKeyConstraint:
            icon = Icon_Utils::it_ForeignKey_Constraint;
            break;
        case UMLListViewItem::lvt_CheckConstraint:
            icon = Icon_Utils::it_Check_Constraint;
            break;
        case UMLListViewItem::lvt_Class_Diagram:
            icon = Icon_Utils::it_Diagram_Class;
            break;
        case UMLListViewItem::lvt_Object_Diagram:
            icon = Icon_Utils::it_Diagram_Object;
            break;
        case UMLListViewItem::lvt_UseCase_Diagram:
            icon = Icon_Utils::it_Diagram_Usecase;
            break;
        case UMLListViewItem::lvt_Sequence_Diagram:
            icon = Icon_Utils::it_Diagram_Sequence;
            break;
        case UMLListViewItem::lvt_Collaboration_Diagram:
            icon = Icon_Utils::it_Diagram_Collaboration;
            break;
        case UMLListViewItem::lvt_State_Diagram:
            icon = Icon_Utils::it_Diagram_State;
            break;
        case UMLListViewItem::lvt_Activity_Diagram:
            icon = Icon_Utils::it_Diagram_Activity;
            break;
        case UMLListViewItem::lvt_Component_Diagram:
            icon = Icon_Utils::it_Diagram_Component;
            break;
        case UMLListViewItem::lvt_Deployment_Diagram:
            icon = Icon_Utils::it_Diagram_Deployment;
            break;
        case UMLListViewItem::lvt_EntityRelationship_Diagram:
            icon = Icon_Utils::it_Diagram_EntityRelationship;
            break;
        case UMLListViewItem::lvt_Properties:
            icon = Icon_Utils::it_Properties;
            break;
        case UMLListViewItem::lvt_Properties_AutoLayout:
            icon = Icon_Utils::it_Properties_AutoLayout;
            break;
        case UMLListViewItem::lvt_Properties_Class:
            icon = Icon_Utils::it_Properties_Class;
            break;
        case UMLListViewItem::lvt_Properties_CodeImport:
            icon = Icon_Utils::it_Properties_CodeImport;
            break;
        case UMLListViewItem::lvt_Properties_CodeGeneration:
            icon = Icon_Utils::it_Properties_CodeGeneration;
            break;
        case UMLListViewItem::lvt_Properties_CodeViewer:
            icon = Icon_Utils::it_Properties_CodeViewer;
            break;
        case UMLListViewItem::lvt_Properties_Font:
            icon = Icon_Utils::it_Properties_Font;
            break;
        case UMLListViewItem::lvt_Properties_General:
            icon = Icon_Utils::it_Properties_General;
            break;
        case UMLListViewItem::lvt_Properties_UserInterface:
            icon = Icon_Utils::it_Properties_UserInterface;
            break;
        case UMLListViewItem::lvt_Instance:
            icon = Icon_Utils::it_Instance;
        break;
        case UMLListViewItem::lvt_InstanceAttribute:
            icon = Icon_Utils::it_Private_Attribute;
        break;
        default:
            break;
    }
    return icon;
}

/**
 * Return the DiagramType which corresponds to the given listview type.
 *
 * @param lvt  ListViewType to convert.
 * @return  The Uml::DiagramType corresponding to the lvt.
 *          Returns dt_Undefined in case no mapping to DiagramType exists.
 */
Uml::DiagramType::Enum convert_LVT_DT(UMLListViewItem::ListViewType lvt)
{
    Uml::DiagramType::Enum dt = Uml::DiagramType::Undefined;
    switch (lvt) {
        case UMLListViewItem::lvt_Class_Diagram:
            dt = Uml::DiagramType::Class;
            break;
        case UMLListViewItem::lvt_UseCase_Diagram:
            dt = Uml::DiagramType::UseCase;
            break;
        case UMLListViewItem::lvt_Sequence_Diagram:
            dt = Uml::DiagramType::Sequence;
            break;
        case UMLListViewItem::lvt_Collaboration_Diagram:
            dt = Uml::DiagramType::Collaboration;
            break;
        case UMLListViewItem::lvt_State_Diagram:
            dt = Uml::DiagramType::State;
            break;
        case UMLListViewItem::lvt_Activity_Diagram:
            dt = Uml::DiagramType::Activity;
            break;
        case UMLListViewItem::lvt_Component_Diagram:
            dt = Uml::DiagramType::Component;
            break;
        case UMLListViewItem::lvt_Deployment_Diagram:
            dt = Uml::DiagramType::Deployment;
            break;
        case UMLListViewItem::lvt_EntityRelationship_Diagram:
            dt = Uml::DiagramType::EntityRelationship;
            break;
    case UMLListViewItem::lvt_Object_Diagram:
            dt = Uml::DiagramType::Object;
        break;
        default:
            break;
    }
    return dt;
}

/**
 * Converts a list view type enum to the equivalent settings dialog type.
 *
 * @param type   The ListViewType to convert.
 * @return  The converted settings dialog type
 */
MultiPageDialogBase::PageType convert_LVT_PT(UMLListViewItem::ListViewType type)
{
    MultiPageDialogBase::PageType pt = MultiPageDialogBase::GeneralPage;
    switch (type) {
    case UMLListViewItem::lvt_Properties:
        pt = MultiPageDialogBase::GeneralPage;
        break;
    case UMLListViewItem::lvt_Properties_AutoLayout:
        pt = MultiPageDialogBase::AutoLayoutPage;
        break;
    case UMLListViewItem::lvt_Properties_Class:
        pt = MultiPageDialogBase::ClassPage;
        break;
    case UMLListViewItem::lvt_Properties_CodeImport:
        pt = MultiPageDialogBase::CodeImportPage;
        break;
    case UMLListViewItem::lvt_Properties_CodeGeneration:
        pt = MultiPageDialogBase::CodeGenerationPage;
        break;
    case UMLListViewItem::lvt_Properties_CodeViewer:
        pt = MultiPageDialogBase::CodeViewerPage;
        break;
    case UMLListViewItem::lvt_Properties_Font:
        pt = MultiPageDialogBase::FontPage;
        break;
    case UMLListViewItem::lvt_Properties_General:
        pt = MultiPageDialogBase::GeneralPage;
        break;
    case UMLListViewItem::lvt_Properties_UserInterface:
        pt = MultiPageDialogBase::UserInterfacePage;
        break;
    default:
        break;
    }
    return pt;
}

/**
 * Return the Model_Type which corresponds to the given ObjectType.
 */
Uml::ModelType::Enum convert_OT_MT(UMLObject::ObjectType ot)
{
    Uml::ModelType::Enum mt = Uml::ModelType::N_MODELTYPES;
    switch (ot) {
        case UMLObject::ot_Actor:
        case UMLObject::ot_UseCase:
            mt = Uml::ModelType::UseCase;
            break;
        case UMLObject::ot_Component:
        case UMLObject::ot_Port:
        case UMLObject::ot_Artifact:
        case UMLObject::ot_SubSystem:
            mt = Uml::ModelType::Component;
            break;
        case UMLObject::ot_Node:
            mt = Uml::ModelType::Deployment;
            break;
        case UMLObject::ot_Entity:
        case UMLObject::ot_EntityAttribute:
        case UMLObject::ot_UniqueConstraint:
        case UMLObject::ot_ForeignKeyConstraint:
        case UMLObject::ot_CheckConstraint:
        case UMLObject::ot_Category:
            mt = Uml::ModelType::EntityRelationship;
            break;
        default:
            mt = Uml::ModelType::Logical;
            break;
    }
    return mt;
}

/**
 * Converts from the UpdateDeleteAction enum to a QString
 * @param uda The UpdateDeleteAction enum literal
 */
QString updateDeleteActionToString(UMLForeignKeyConstraint::UpdateDeleteAction uda)
{
    switch(uda) {
     case UMLForeignKeyConstraint::uda_NoAction:
         return QStringLiteral("NO ACTION");
     case  UMLForeignKeyConstraint::uda_Restrict:
         return QStringLiteral("RESTRICT");
     case UMLForeignKeyConstraint::uda_Cascade:
         return QStringLiteral("CASCADE");
     case  UMLForeignKeyConstraint::uda_SetNull:
         return QStringLiteral("SET NULL");
     case  UMLForeignKeyConstraint::uda_SetDefault:
         return QStringLiteral("SET DEFAULT");
     default:
         return QString();
    }
}

/**
 * Return true if the object type is allowed in the related diagram
 * @param o UML object instance
 * @param scene diagram instance
 * @return true type is allowed
 * @return false type is not allowed
 */
bool typeIsAllowedInDiagram(UMLObject* o, UMLScene *scene)
{
    //make sure dragging item onto correct diagram
    // classifier - class, seq, coll diagram
    // actor, usecase - usecase diagram
    UMLObject::ObjectType ot = o->baseType();
    Uml::ID::Type id = o->id();
    Uml::DiagramType::Enum diagramType = scene->type();
    bool bAccept = true;

    switch (diagramType) {
    case Uml::DiagramType::UseCase:
        if ((scene->widgetOnDiagram(id) && ot == UMLObject::ot_Actor) ||
            (ot != UMLObject::ot_Actor && ot != UMLObject::ot_UseCase))
            bAccept = false;
        break;
    case Uml::DiagramType::Class:
        if (scene->widgetOnDiagram(id) ||
            (ot != UMLObject::ot_Class &&
             ot != UMLObject::ot_Package &&
             ot != UMLObject::ot_Interface &&
             ot != UMLObject::ot_Enum &&
             ot != UMLObject::ot_Datatype &&
             ot != UMLObject::ot_Instance)) {
            bAccept = false;
        }
        break;
    case Uml::DiagramType::Object:
        if (scene->widgetOnDiagram(id) || ot != UMLObject::ot_Instance)
            bAccept = false;
        break;
    case Uml::DiagramType::Sequence:
        if (ot != UMLObject::ot_Class &&
            ot != UMLObject::ot_Interface &&
            ot != UMLObject::ot_Actor)
            bAccept = false;
        break;
    case Uml::DiagramType::Collaboration:
        if (ot != UMLObject::ot_Class &&
            ot != UMLObject::ot_Interface &&
            ot != UMLObject::ot_Instance &&
            ot != UMLObject::ot_Actor)
            bAccept = false;
        break;
    case Uml::DiagramType::Deployment:
        if (scene->widgetOnDiagram(id))
            bAccept = false;
        else if (ot != UMLObject::ot_Interface &&
                 ot != UMLObject::ot_Package &&
                 ot != UMLObject::ot_Component &&
                 ot != UMLObject::ot_Class &&
                 ot != UMLObject::ot_Node)
            bAccept = false;
        else if (ot == UMLObject::ot_Package &&
                 o->stereotype() != QStringLiteral("subsystem"))
            bAccept = false;
        break;
    case Uml::DiagramType::Component:
        if (scene->widgetOnDiagram(id) ||
            (ot != UMLObject::ot_Interface &&
             ot != UMLObject::ot_Package &&
             ot != UMLObject::ot_Component &&
             ot != UMLObject::ot_Port &&
             ot != UMLObject::ot_Artifact &&
             ot != UMLObject::ot_Class))
            bAccept = false;
        else if (ot == UMLObject::ot_Class && !o->isAbstract())
            bAccept = false;
        else if (ot == UMLObject::ot_Port) {
            const bool componentOnDiagram = scene->widgetOnDiagram(o->umlPackage()->id());
            bAccept = componentOnDiagram;
        }
        break;
    case Uml::DiagramType::EntityRelationship:
        if (scene->widgetOnDiagram(id) ||
            (ot != UMLObject::ot_Entity &&
             ot != UMLObject::ot_Category))
            bAccept = false;
        break;
    default:
        break;
    }
    return bAccept;
}

/**
 * Return true if the widget type is allowed in the related diagram
 * @param w UML widget object
 * @param scene diagram instance
 * @return true type is allowed
 * @return false type is not allowed
 */
bool typeIsAllowedInDiagram(UMLWidget* w, UMLScene *scene)
{
    UMLWidget::WidgetType wt = w->baseType();
    Uml::DiagramType::Enum diagramType = scene->type();
    bool bAccept = true;

    // TODO: check additional widgets
    switch (diagramType) {
    case Uml::DiagramType::Activity:
    case Uml::DiagramType::Class:
    case Uml::DiagramType::Object:
    case Uml::DiagramType::Collaboration:
    case Uml::DiagramType::Component:
    case Uml::DiagramType::Deployment:
    case Uml::DiagramType::EntityRelationship:
    case Uml::DiagramType::Sequence:
    case Uml::DiagramType::State:
    case Uml::DiagramType::UseCase:
    default:
        switch(wt) {
        case WidgetBase::wt_Note:
            break;
        case WidgetBase::wt_Text:
            {
                FloatingTextWidget *ft = w->asFloatingTextWidget();
                if (ft && ft->textRole() != Uml::TextRole::Floating) {
                    bAccept = false;
                }
            }
            break;
        default:
            bAccept = false;
            break;
        }
        break;
    }
    return bAccept;
}

/**
 * return true if given object type supports associations
 * @param type uml object type to check
 */
bool hasAssociations(UMLObject::ObjectType type)
{
    switch (type) {
        case UMLObject::ot_Actor:
        case UMLObject::ot_UseCase:
        case UMLObject::ot_Class:
        case UMLObject::ot_Package:
        case UMLObject::ot_Component:
        case UMLObject::ot_Node:
        case UMLObject::ot_Artifact:
        case UMLObject::ot_Interface:
        case UMLObject::ot_Enum:
        case UMLObject::ot_Entity:
        case UMLObject::ot_Datatype:
        case UMLObject::ot_Category:
        case UMLObject::ot_Instance:
            return true;
        default:
            return false;
    }
}
}  // namespace Model_Utils

