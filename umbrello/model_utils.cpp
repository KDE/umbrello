/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "model_utils.h"

// app includes
#include "debug_utils.h"
#include "umlobject.h"
#include "umlpackagelist.h"
#include "uniqueconstraint.h"
#include "package.h"
#include "folder.h"
#include "classifier.h"
#include "enum.h"
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
#include <klocale.h>

// qt includes
#include <QtCore/QRegExp>
#include <QtCore/QStringList>

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
    case WidgetBase::wt_Node:
    case WidgetBase::wt_Artifact:
        return true;
    default:
        return false;
    }
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
UMLObject* findObjectInList(Uml::IDType id, const UMLObjectList& inList)
{
    for (UMLObjectListIt oit(inList); oit.hasNext(); ) {
        UMLObject *obj = oit.next();
        if (obj->id() == id)
            return obj;
        UMLObject *o;
        UMLObject::ObjectType t = obj->baseType();
        switch (t) {
        case UMLObject::ot_Folder:
        case UMLObject::ot_Package:
        case UMLObject::ot_Component:
            o = static_cast<UMLPackage*>(obj)->findObjectById(id);
            if (o)
                return o;
            break;
        case UMLObject::ot_Interface:
        case UMLObject::ot_Class:
        case UMLObject::ot_Enum:
        case UMLObject::ot_Entity:
            o = static_cast<UMLClassifier*>(obj)->findChildObjectById(id);
            if (o == NULL &&
                    (t == UMLObject::ot_Interface || t == UMLObject::ot_Class))
                o = ((UMLPackage*)obj)->findObjectById(id);
            if (o)
                return o;
            break;
        case UMLObject::ot_Association:
            {
                UMLAssociation *assoc = static_cast<UMLAssociation*>(obj);
                UMLRole *rA = assoc->getUMLRole(Uml::A);
                if (rA->id() == id)
                    return rA;
                UMLRole *rB = assoc->getUMLRole(Uml::B);
                if (rB->id() == id)
                    return rB;
            }
            break;
        default:
            break;
        }
    }
    return NULL;
}

/**
 * Find the UML object of the given type and name in the passed-in list.
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
UMLObject* findUMLObject(const UMLObjectList& inList,
                         const QString& inName,
                         UMLObject::ObjectType type /* = ot_UMLObject */,
                         UMLObject *currentObj /* = NULL */)
{
    const bool caseSensitive = UMLApp::app()->activeLanguageIsCaseSensitive();
    QString name = inName;
    QStringList components;
#ifdef TRY_BUGFIX_120682
    // If we have a pointer or a reference in cpp we need to remove
    // the asterisks and ampersands in order to find the appropriate object
    if (UMLApp::app()->getActiveLanguage() == Uml::pl_Cpp) {
        if (name.endsWith('*'))
            name.remove('*');
        else if (name.contains('&'))
            name.remove('&');
    }
#endif
    if (name.contains("::"))
        components = name.split("::");
    else if (name.contains('.'))
        components = name.split('.');
    QString nameWithoutFirstPrefix;
    if (components.size() > 1) {
        name = components.front();
        components.pop_front();
        nameWithoutFirstPrefix = components.join("::");
    }
    if (currentObj) {
        UMLPackage *pkg = NULL;
        if (dynamic_cast<UMLClassifierListItem*>(currentObj)) {
            currentObj = static_cast<UMLObject*>(currentObj->parent());
        }
        pkg = dynamic_cast<UMLPackage*>(currentObj);
        if (pkg == NULL)
            pkg = currentObj->umlPackage();
        // Remember packages that we've seen - for avoiding cycles.
        UMLPackageList seenPkgs;
        for (; pkg; pkg = currentObj->umlPackage()) {
            if (nameWithoutFirstPrefix.isEmpty()) {
                if (caseSensitive) {
                    if (pkg->name() == name)
                        return pkg;
                } else if (pkg->name().toLower() == name.toLower()) {
                    return pkg;
                }
            }
            if (seenPkgs.indexOf(pkg) != -1) {
                uError() << "findUMLObject(" << name << "): "
                    << "breaking out of cycle involving "
                    << pkg->name();
                break;
            }
            seenPkgs.append(pkg);
            UMLObjectList objectsInCurrentScope = pkg->containedObjects();
            for (UMLObjectListIt oit(objectsInCurrentScope); oit.hasNext(); ) {
                UMLObject *obj = oit.next();
                if (caseSensitive) {
                    if (obj->name() != name)
                        continue;
                } else if (obj->name().toLower() != name.toLower()) {
                    continue;
                }
                UMLObject::ObjectType foundType = obj->baseType();
                if (nameWithoutFirstPrefix.isEmpty()) {
                    if (type != UMLObject::ot_UMLObject && type != foundType) {
                        uDebug() << "findUMLObject: type mismatch for "
                            << name << " (seeking type: "
                            << UMLObject::toString(type) << ", found type: "
                            << UMLObject::toString(foundType) << ")";
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
                        continue;
                    }
                    return obj;
                }
                if (foundType != UMLObject::ot_Package &&
                    foundType != UMLObject::ot_Folder &&
                    foundType != UMLObject::ot_Class &&
                    foundType != UMLObject::ot_Interface &&
                    foundType != UMLObject::ot_Component) {
                    uDebug() << "findUMLObject: found \"" << name
                        << "\" is not a package (?)";
                    continue;
                }
                UMLPackage *pkg = static_cast<UMLPackage*>(obj);
                return findUMLObject( pkg->containedObjects(),
                                      nameWithoutFirstPrefix, type );
            }
            currentObj = pkg;
        }
    }
    for (UMLObjectListIt oit(inList); oit.hasNext(); ) {
        UMLObject *obj = oit.next();
        if (caseSensitive) {
            if (obj->name() != name)
                continue;
        } else if (obj->name().toLower() != name.toLower()) {
            continue;
        }
        UMLObject::ObjectType foundType = obj->baseType();
        if (nameWithoutFirstPrefix.isEmpty()) {
            if (type != UMLObject::ot_UMLObject && type != foundType) {
                uDebug() << "findUMLObject: type mismatch for "
                    << name << " (seeking type: "
                    << UMLObject::toString(type) << ", found type: "
                    << UMLObject::toString(foundType) << ")";
                continue;
            }
            return obj;
        }
        if (foundType != UMLObject::ot_Package &&
            foundType != UMLObject::ot_Folder &&
            foundType != UMLObject::ot_Class &&
            foundType != UMLObject::ot_Interface &&
            foundType != UMLObject::ot_Component) {
            uDebug() << "findUMLObject: found \"" << name
                << "\" is not a package (?)";
            continue;
        }
        UMLPackage *pkg = static_cast<UMLPackage*>(obj);
        return findUMLObject( pkg->containedObjects(),
                              nameWithoutFirstPrefix, type );
    }
    return NULL;
}

/**
 * Find the UML object of the given type and name in the passed-in list.
 * This methods search for the raw name.
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
                            UMLObject *currentObj /*= 0*/)
{
    Q_UNUSED(currentObj);
    for (UMLObjectListIt oit(inList); oit.hasNext(); ) {
        UMLObject *obj = oit.next();
        if (obj->name() == name && type == obj->baseType())
            return obj;
    }
    return NULL;
}

/**
 * Add the given list of views to the tree view.
 * @param viewList   the list of views to add
 */
void treeViewAddViews(const UMLViewList& viewList)
{
    UMLListView* tree = UMLApp::app()->listView();
    foreach (UMLView* v,  viewList) {
        if (tree->findItem(v->umlScene()->getID()) != NULL) {
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
    UMLListViewItem *current = static_cast<UMLListViewItem*>(listView->currentItem());
    return current->umlObject();
}

/**
 * Return the UMLPackage if the current item
 * in the tree view is a package.
 * @return   the package or NULL
 */
UMLPackage* treeViewGetPackageFromCurrent()
{
    UMLListView *listView = UMLApp::app()->listView();
    UMLListViewItem *parentItem = (UMLListViewItem*)listView->currentItem();
    if (parentItem) {
        UMLListViewItem::ListViewType lvt = parentItem->type();
        if (Model_Utils::typeIsContainer(lvt) ||
            lvt == UMLListViewItem::lvt_Class ||
            lvt == UMLListViewItem::lvt_Interface) {
            UMLObject *o = parentItem->umlObject();
            return static_cast<UMLPackage*>(o);
        }
    }
    return NULL;
}

/**
 * Build the diagram name from the tree view.
 * @param id   the id of the diaram
 * @return     the constructed diagram name
 */
QString treeViewBuildDiagramName(Uml::IDType id)
{
    UMLListView *listView = UMLApp::app()->listView();
    UMLListViewItem* listViewItem = listView->findItem(id);

    if (listViewItem) {
        // skip the name of the first item because it's the View
        listViewItem = static_cast<UMLListViewItem*>(listViewItem->parent());
        
        // Relies on the tree structure of the UMLListView. There are a base "Views" folder
        // and five children, one for each view type (Logical, use case, components, deployment
        // and entity relationship)
        QString name;
        while (listView->rootView(listViewItem->type()) == NULL) {
            name.insert(0, listViewItem->text(0) + '/');
            listViewItem = static_cast<UMLListViewItem*>(listViewItem->parent());
            if (listViewItem == NULL)
                break;
        }
        return name;
    }
    else {
        uWarning() << "diagram not found - returning empty name!";
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
        if(type == UMLObject::ot_Class)
            currentName = i18n("new_class");
        else if(type == UMLObject::ot_Actor)
            currentName = i18n("new_actor");
        else if(type == UMLObject::ot_UseCase)
            currentName = i18n("new_usecase");
        else if(type == UMLObject::ot_Package)
            currentName = i18n("new_package");
        else if(type == UMLObject::ot_Component)
            currentName = i18n("new_component");
        else if(type == UMLObject::ot_Node)
            currentName = i18n("new_node");
        else if(type == UMLObject::ot_Artifact)
            currentName = i18n("new_artifact");
        else if(type == UMLObject::ot_Interface)
            currentName = i18n("new_interface");
        else if(type == UMLObject::ot_Datatype)
            currentName = i18n("new_datatype");
        else if(type == UMLObject::ot_Enum)
            currentName = i18n("new_enum");
        else if(type == UMLObject::ot_Entity)
            currentName = i18n("new_entity");
        else if(type == UMLObject::ot_Folder)
            currentName = i18n("new_folder");
        else if(type == UMLObject::ot_Association)
            currentName = i18n("new_association");
        else if(type == UMLObject::ot_Category)
            currentName = i18n("new_category");
        else {
            currentName = i18n("new_object");
            uWarning() << "unknown object type in umldoc::uniqObjectName()";
        }
    }
    UMLDoc *doc = UMLApp::app()->document();
    QString name = currentName;
    for (int number = 1; !doc->isUnique(name, parentPkg); ++number)  {
        name = currentName + '_' + QString::number(number);
    }
    return name;
}

/**
 * Return true if the given tag is a one of the common XMI
 * attributes, such as:
 * "name" | "visibility" | "isRoot" | "isLeaf" | "isAbstract" |
 * "isActive" | "ownerScope"
 */
bool isCommonXMIAttribute( const QString &tag )
{
    bool retval = (UMLDoc::tagEq(tag, "name") ||
                   UMLDoc::tagEq(tag, "visibility") ||
                   UMLDoc::tagEq(tag, "isRoot") ||
                   UMLDoc::tagEq(tag, "isLeaf") ||
                   UMLDoc::tagEq(tag, "isAbstract") ||
                   UMLDoc::tagEq(tag, "isSpecification") ||
                   UMLDoc::tagEq(tag, "isActive") ||
                   UMLDoc::tagEq(tag, "namespace") ||
                   UMLDoc::tagEq(tag, "ownerScope") ||
                   UMLDoc::tagEq(tag, "ModelElement.stereotype") ||
                   UMLDoc::tagEq(tag, "GeneralizableElement.generalization") ||
                   UMLDoc::tagEq(tag, "specialization") ||   //NYI
                   UMLDoc::tagEq(tag, "clientDependency") || //NYI
                   UMLDoc::tagEq(tag, "supplierDependency")  //NYI
                  );
    return retval;
}

/**
 * Return true if the given type is common among the majority
 * of programming languages, such as "bool" or "boolean".
 * TODO: Make this depend on the active programming language.
 */
bool isCommonDataType(QString type)
{
    CodeGenerator *gen = UMLApp::app()->generator();
    if (gen == NULL)
        return false;
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
        return true;
    } else {
        return false;
    }
}

/**
 * Try to guess the correct container folder type of an UMLObject.
 * Object types that can't be guessed are mapped to Uml::ModelType::Logical.
 * NOTE: This function exists mainly for handling pre-1.5.5 files
 *       and should not be used for new code.
 */
Uml::ModelType guessContainer(UMLObject *o)
{
    UMLObject::ObjectType ot = o->baseType();
    if (ot == UMLObject::ot_Package && o->stereotype() == "subsystem")
        return Uml::ModelType::Component;
    Uml::ModelType mt = Uml::ModelType::N_MODELTYPES;
    switch (ot) {
        case UMLObject::ot_Package:   // CHECK: packages may appear in other views?
        case UMLObject::ot_Interface:
        case UMLObject::ot_Datatype:
        case UMLObject::ot_Enum:
        case UMLObject::ot_Class:
        case UMLObject::ot_Attribute:
        case UMLObject::ot_Operation:
        case UMLObject::ot_EnumLiteral:
        case UMLObject::ot_Template:
            mt = Uml::ModelType::Logical;
            break;
        case UMLObject::ot_Actor:
        case UMLObject::ot_UseCase:
            mt = Uml::ModelType::UseCase;
            break;
        case UMLObject::ot_Component:
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
                UMLAssociation *assoc = static_cast<UMLAssociation*>(o);
                UMLDoc *umldoc = UMLApp::app()->document();
                for (int r = Uml::A; r <= Uml::B; ++r) {
                    UMLObject *roleObj = assoc->getObject((Uml::Role_Type)r);
                    if (roleObj == NULL) {
                        // Ouch! we have been called while types are not yet resolved
                        return Uml::ModelType::N_MODELTYPES;
                    }
                    UMLPackage *pkg = roleObj->umlPackage();
                    if (pkg) {
                        while (pkg->umlPackage()) {  // wind back to root
                            pkg = pkg->umlPackage();
                        }
                        const Uml::ModelType m = umldoc->rootFolderType(pkg);
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
 * Parse a direction string into the Uml::Parameter_Direction.
 *
 * @param input  The string to parse: "in", "out", or "inout"
 *               optionally followed by whitespace.
 * @param result The corresponding Uml::Parameter_Direction.
 * @return       Length of the string matched, excluding the optional
 *               whitespace.
 */
int stringToDirection(QString input, Uml::Parameter_Direction & result) 
{
    QRegExp dirx("^(in|out|inout)");
    int pos = dirx.indexIn(input);
    if (pos == -1)
        return 0;
    const QString dirStr = dirx.capturedTexts().first();
    int dirLen = dirStr.length();
    if (input.length() > dirLen && !input[dirLen].isSpace())
        return 0;       // no match after all.
    if (dirStr == "out")
        result = Uml::pd_Out;
    else if (dirStr == "inout")
        result = Uml::pd_InOut;
    else
        result = Uml::pd_In;
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

    QStringList nameAndType = t.split( QRegExp("\\s*:\\s*"));
    if (nameAndType.count() == 2) {
        UMLObject *pType = NULL;
        if (nameAndType[1] != "class") {
            pType = pDoc->findUMLObject(nameAndType[1], UMLObject::ot_UMLObject, owningScope);
            if (pType == NULL)
                return PS_Unknown_ArgType;
        }
        nmTp = NameAndType(nameAndType[0], pType);
    } else {
        nmTp = NameAndType(t, NULL);
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
                            Uml::Visibility *vis /* = 0 */)
{
    UMLDoc *pDoc = UMLApp::app()->document();

    a = a.simplified();
    if (a.isEmpty())
        return PS_Empty;

    int colonPos = a.indexOf(':');
    if (colonPos < 0) {
        nmTp = NameAndType(a, NULL);
        return PS_OK;
    }
    QString name = a.left(colonPos).trimmed();
    if (vis) {
        QRegExp mnemonicVis("^([\\+\\#\\-\\~] *)");
        int pos = mnemonicVis.indexIn(name);
        if (pos == -1) {
            *vis = Uml::Visibility::Private;  // default value
        } else {
            QString caption = mnemonicVis.cap(1);
            QString strVis = caption.left(1);
            if (strVis == "+")
                *vis = Uml::Visibility::Public;
            else if (strVis == "#")
                *vis = Uml::Visibility::Protected;
            else if (strVis == "-")
                *vis = Uml::Visibility::Private;
            else
                *vis = Uml::Visibility::Implementation;
        }
        name.remove(mnemonicVis);
    }
    Uml::Parameter_Direction pd = Uml::pd_In;
    if (name.startsWith(QLatin1String("in "))) {
        pd = Uml::pd_In;
        name = name.mid(3);
    } else if (name.startsWith(QLatin1String("inout "))) {
        pd = Uml::pd_InOut;
        name = name.mid(6);
    } else if (name.startsWith(QLatin1String("out "))) {
        pd = Uml::pd_Out;
        name = name.mid(4);
    }
    a = a.mid(colonPos + 1).trimmed();
    if (a.isEmpty()) {
        nmTp = NameAndType(name, NULL, pd);
        return PS_OK;
    }
    QStringList typeAndInitialValue = a.split( QRegExp("\\s*=\\s*") );
    const QString &type = typeAndInitialValue[0];
    UMLObject *pType = pDoc->findUMLObject(type, UMLObject::ot_UMLObject, owningScope);
    if (pType == NULL) {
        nmTp = NameAndType(name, NULL, pd);
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
    if (m.contains(QRegExp("operator *()"))) {
        // C++ special case: two sets of parentheses
        desc.m_name = "operator()";
        m.remove(QRegExp("operator *()"));
    } else {
        /**
         * The search pattern includes everything up to the opening parenthesis
         * because UML also permits non programming-language oriented designs
         * using narrative names, for example "check water temperature".
         */
        QRegExp beginningUpToOpenParenth( "^([^\\(]+)" );
        int pos = beginningUpToOpenParenth.indexIn(m);
        if (pos == -1)
            return PS_Illegal_MethodName;
        desc.m_name = beginningUpToOpenParenth.cap(1);
    }
    desc.m_pReturnType = NULL;
    QRegExp pat = QRegExp("\\) *:(.*)$");
    int pos = pat.indexIn(m);
    if (pos != -1) {  // return type is optional
        QString retType = pat.cap(1);
        retType = retType.trimmed();
        if (retType != "void") {
            UMLObject *pRetType = owningScope->findTemplate(retType);
            if (pRetType == NULL) {
                pRetType = pDoc->findUMLObject(retType, UMLObject::ot_UMLObject, owningScope);
                if (pRetType == NULL)
                    return PS_Unknown_ReturnType;
            }
            desc.m_pReturnType = pRetType;
        }
    }
    // Remove possible empty parentheses ()
    m.remove( QRegExp("\\s*\\(\\s*\\)") );
    desc.m_args.clear();
    pat = QRegExp( "\\((.*)\\)" );
    pos = pat.indexIn(m);
    if (pos == -1)  // argument list is optional
        return PS_OK;
    QString arglist = pat.cap(1);
    arglist = arglist.trimmed();
    if (arglist.isEmpty())
        return PS_OK;
    const QStringList args = arglist.split( QRegExp("\\s*,\\s*") );
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

    int colonPos = m.indexOf(':');
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
        case UMLListViewItem::lvt_Node:
        case UMLListViewItem::lvt_Artifact:
        case UMLListViewItem::lvt_Interface:
        case UMLListViewItem::lvt_Datatype:
        case UMLListViewItem::lvt_Enum:
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
            type == UMLListViewItem::lvt_Component);
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
        return true;
    } else {
        return false;
    }
}

/**
 * Return true if the listview type is a classifier ( Class, Entity , Enum )
 */
bool typeIsClassifier(UMLListViewItem::ListViewType type)
{
    if ( type == UMLListViewItem::lvt_Class ||
         type == UMLListViewItem::lvt_Interface ||
         type == UMLListViewItem::lvt_Entity ||
         type == UMLListViewItem::lvt_Enum ) {
        return true;
    }
    return false;
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
            type == UMLListViewItem::lvt_EntityRelationship_Diagram) {
        return true;
    } else {
        return false;
    }
}

/**
 * Return the Model_Type which corresponds to the given DiagramType.
 */
Uml::ModelType convert_DT_MT(Uml::DiagramType dt)
{
    Uml::ModelType mt;
    switch (dt) {
        case Uml::DiagramType::UseCase:
            mt = Uml::ModelType::UseCase;
            break;
        case Uml::DiagramType::Collaboration:
        case Uml::DiagramType::Class:
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
            uError() << "Model_Utils::convert_DT_MT: illegal input value " << dt;
            mt = Uml::ModelType::N_MODELTYPES;
            break;
    }
    return mt;
}

/**
 * Return the ListViewType which corresponds to the given Model_Type.
 */
UMLListViewItem::ListViewType convert_MT_LVT(Uml::ModelType mt)
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
Uml::ModelType convert_LVT_MT(UMLListViewItem::ListViewType lvt)
{
    Uml::ModelType mt = Uml::ModelType::N_MODELTYPES;
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
UMLListViewItem::ListViewType convert_DT_LVT(Uml::DiagramType dt)
{
    UMLListViewItem::ListViewType type =  UMLListViewItem::lvt_Unknown;
    switch(dt) {
    case Uml::DiagramType::UseCase:
        type = UMLListViewItem::lvt_UseCase_Diagram;
        break;

    case Uml::DiagramType::Class:
        type = UMLListViewItem::lvt_Class_Diagram;
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
        uWarning() << "convert_DT_LVT() called on unknown diagram type";
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
        type = UMLListViewItem::lvt_Package;
        break;

    case UMLObject::ot_Folder:
        {
            UMLDoc *umldoc = UMLApp::app()->document();
            UMLFolder *f = static_cast<UMLFolder*>(o);
            do {
                const Uml::ModelType mt = umldoc->rootFolderType(f);
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
            } while ((f = static_cast<UMLFolder*>(f->umlPackage())) != NULL);
            uError() << "convert_OT_LVT(" << o->name()
                << "): internal error - object is not properly nested in folder";
        }
        break;

    case UMLObject::ot_Component:
        type = UMLListViewItem::lvt_Component;
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
         UMLEntity* ent = static_cast<UMLEntity*>(o->parent());
         UMLUniqueConstraint* uc = static_cast<UMLUniqueConstraint*>( o );
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
 * @return  The Icon_Utils::IconType corresponding to the lvt.
 *          Returns it_Home in case no mapping to IconType exists.
 */
Icon_Utils::IconType convert_LVT_IT(UMLListViewItem::ListViewType lvt)
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
        case UMLListViewItem::lvt_UseCase:
            icon = Icon_Utils::it_UseCase;
            break;
        case UMLListViewItem::lvt_Class:
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
Uml::DiagramType convert_LVT_DT(UMLListViewItem::ListViewType lvt)
{
    Uml::DiagramType dt = Uml::DiagramType::Undefined;
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
        default:
            break;
    }
    return dt;
}

/**
 * Return the Model_Type which corresponds to the given ObjectType.
 */
Uml::ModelType convert_OT_MT(UMLObject::ObjectType ot)
{
    Uml::ModelType mt = Uml::ModelType::N_MODELTYPES;
    switch (ot) {
        case UMLObject::ot_Actor:
        case UMLObject::ot_UseCase:
            mt = Uml::ModelType::UseCase;
            break;
        case UMLObject::ot_Component:
        case UMLObject::ot_Artifact:
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
QString updateDeleteActionToString( UMLForeignKeyConstraint::UpdateDeleteAction uda )
{
    switch( uda ) {
     case UMLForeignKeyConstraint::uda_NoAction:
         return "NO ACTION";
     case  UMLForeignKeyConstraint::uda_Restrict:
         return "RESTRICT";
     case UMLForeignKeyConstraint::uda_Cascade:
         return "CASCADE";
     case  UMLForeignKeyConstraint::uda_SetNull:
         return "SET NULL";
     case  UMLForeignKeyConstraint::uda_SetDefault:
         return "SET DEFAULT";
     default:
         return QString();
    }
}

}  // namespace Model_Utils

