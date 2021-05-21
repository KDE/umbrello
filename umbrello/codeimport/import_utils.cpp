/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2005-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "import_utils.h"

// app includes
#include "association.h"
#include "artifact.h"
#include "classifier.h"
#include "datatype.h"
#include "debug_utils.h"
#include "folder.h"
#include "enum.h"
#include "object_factory.h"
#include "operation.h"
#include "package.h"
#include "template.h"
#include "uml.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umlobject.h"

// kde includes
#include <KMessageBox>
#include <KLocalizedString>

#ifdef Q_OS_WIN
#define PATH_SEPARATOR QLatin1Char(';')
#else
#define PATH_SEPARATOR QLatin1Char(':')
#endif

DEBUG_REGISTER_DISABLED(Import_Utils)
#undef DBG_SRC
#define DBG_SRC QLatin1String("Import_Utils")

namespace Import_Utils {

/**
 * Flag manipulated by createUMLObject().
 * Global state is generally bad, I know.
 * It would be cleaner to make this into a return value from
 * createUMLObject().
 */
bool bNewUMLObjectWasCreated = false;

/**
 * Related classifier for creation of dependencies on template
 * parameters in createUMLObject().
 */
UMLClassifier * gRelatedClassifier = 0;

/**
 * On encountering a scoped typename string where the scopes
 * have not yet been seen, we synthesize UML objects for the
 * unknown scopes (using a question dialog to the user to decide
 * whether to treat a scope as a class or as a package.)
 * However, such an unknown scope is put at the global level.
 * I.e. before calling createUMLObject() we set this flag to true.
 */
bool bPutAtGlobalScope = false;

/**
 * The include path list (see addIncludePath() and includePathList())
 */
QStringList incPathList;

/**
 * Control whether an object which is newly created by createUMLObject()
 * is put at the global scope.
 *
 * @param yesno  When set to false, the object is created at the scope
 *               given by the parentPkg argument of createUMLObject().
 */
void putAtGlobalScope(bool yesno)
{
    bPutAtGlobalScope = yesno;
}

/**
 * Set a related classifier for creation of dependencies on template
 * parameters in createUMLObject().
 */
void setRelatedClassifier(UMLClassifier *c)
{
    gRelatedClassifier = c;
}

/**
 * Control whether the creation methods solicit a new unique ID for the
 * created object.
 * By default, unique ID generation is turned on.
 *
 * @param yesno   False turns UID generation off, true turns it on.
 */
void assignUniqueIdOnCreation(bool yesno)
{
    Object_Factory::assignUniqueIdOnCreation(yesno);
}

/**
 * Returns whether the last createUMLObject() actually created
 * a new object or just returned an existing one.
 */
bool newUMLObjectWasCreated()
{
    return bNewUMLObjectWasCreated;
}

/**
 * Strip comment lines of leading whitespace and stars.
 */
QString formatComment(const QString &comment)
{
    if (comment.isEmpty())
        return comment;

    QStringList lines = comment.split(QLatin1Char('\n'));
    QString& first = lines.first();
    QRegExp wordex(QLatin1String("\\w"));
    if (first.startsWith(QLatin1String(QLatin1String("/*")))) {
        int wordpos = wordex.indexIn(first);
        if (wordpos != -1)
            first = first.mid(wordpos);  // remove comment start
        else
            lines.pop_front();  // nothing interesting on this line
    }
    if (! lines.count())
        return QString();

    QString& last = lines.last();
    int endpos = last.indexOf(QLatin1String("*/"));
    if (endpos != -1) {
        if (last.contains(wordex))
            last = last.mid(0, endpos - 1);  // remove comment end
        else
            lines.pop_back();  // nothing interesting on this line
    }
    if (! lines.count())
        return QString();

    QStringList::Iterator end(lines.end());
    for (QStringList::Iterator lit(lines.begin()); lit != end; ++lit) {
        (*lit).remove(QRegExp(QLatin1String("^\\s+")));
        (*lit).remove(QRegExp(QLatin1String("^\\*+\\s?")));
    }
    return lines.join(QLatin1String("\n"));
}

/*
UMLObject* findUMLObject(QString name, UMLObject::ObjectType type)
{
    // Why an extra wrapper? See comment at addMethodParameter()
    UMLObject * o = umldoc->findUMLObject(name, type);
    return o;
}
 */

/**
 * Find or create a document object.
 * @param type object type
 * @param inName name of uml object
 * @param parentPkg parent package
 * @param comment comment for uml object
 * @param stereotype stereotype for uml object
 * @param searchInParentPackageOnly flags to search only in parent package
 * @param remapParent flag to control remapping of parents if a uml object has been found
 * @return new object or zero
 */
UMLObject *createUMLObject(UMLObject::ObjectType type,
                           const QString& inName,
                           UMLPackage *parentPkg,
                           const QString& comment,
                           const QString& stereotype,
                           bool searchInParentPackageOnly,
                           bool remapParent)
{
    QString name = inName;
    UMLDoc *umldoc = UMLApp::app()->document();
    UMLFolder *logicalView = umldoc->rootFolder(Uml::ModelType::Logical);
    if (parentPkg == 0) {
        // DEBUG(DBG_SRC) << "Import_Utils::createUMLObject(" << name
        //     << "): parentPkg is 0, assuming Logical View";
        parentPkg = logicalView;
    } else if (parentPkg->baseType() == UMLObject::ot_Artifact) {
        DEBUG(DBG_SRC) << "Import_Utils::createUMLObject(" << name
                       << "): Artifact as parent package is not supported yet, using Logical View";
        parentPkg = logicalView;
    } else if (parentPkg->baseType() == UMLObject::ot_Association) {
        DEBUG(DBG_SRC) << "Import_Utils::createUMLObject(" << name
                       << "): Association as parent package is not supported yet, using Logical View";
        parentPkg = logicalView;
    } else if (name.startsWith(UMLApp::app()->activeLanguageScopeSeparator())) {
        name = name.mid(2);
        parentPkg = logicalView;
    }
    bNewUMLObjectWasCreated = false;
    UMLObject *o = 0;
    if (searchInParentPackageOnly) {
        o = Model_Utils::findUMLObject(parentPkg->containedObjects(), name, type);
        if (!o) {
            o = Object_Factory::createNewUMLObject(type, name, parentPkg);
            bNewUMLObjectWasCreated = true;
            bPutAtGlobalScope = false;
        }
    } else {
        o = umldoc->findUMLObject(name, type, parentPkg);
    }
    if (o == 0) {
        // Strip possible adornments and look again.
        const bool isConst = name.contains(QRegExp(QLatin1String("^const ")));
        name.remove(QRegExp(QLatin1String("^const\\s+")));
        const bool isVolatile = name.contains(QRegExp(QLatin1String("^volatile ")));
        name.remove(QRegExp(QLatin1String("^volatile\\s+")));
        const bool isMutable = name.contains(QRegExp(QLatin1String("^mutable ")));
        name.remove(QRegExp(QLatin1String("^mutable\\s+")));
        QString typeName(name);
        bool isAdorned = typeName.contains(QRegExp(QLatin1String("[^\\w:\\. ]")));
        const bool isPointer = typeName.contains(QLatin1Char('*'));
        const bool isRef = typeName.contains(QLatin1Char('&'));
        typeName.remove(QRegExp(QLatin1String("[^\\w:\\. ].*$")));
        typeName = typeName.simplified();
        UMLObject *origType = umldoc->findUMLObject(typeName, UMLObject::ot_UMLObject, parentPkg);
        if (origType == 0) {
            // Still not found. Create the stripped down type.
            if (bPutAtGlobalScope)
                parentPkg = logicalView;
            // Find, or create, the scopes.
            QStringList components;
            QString scopeSeparator = UMLApp::app()->activeLanguageScopeSeparator();
            if (typeName.contains(scopeSeparator)) {
                components = typeName.split(scopeSeparator, QString::SkipEmptyParts);
            } else if (typeName.contains(QLatin1String("..."))) {
                // Java variable length arguments
                type = UMLObject::ot_Datatype;
                parentPkg = umldoc->datatypeFolder();
                isAdorned = false;
            }
            if (components.count() > 1) {
                typeName = components.back();
                components.pop_back();
                while (components.count()) {
                    QString scopeName = components.front();
                    components.pop_front();
                    o = umldoc->findUMLObject(scopeName, UMLObject::ot_UMLObject, parentPkg);
                    if (o) {
                        parentPkg = o->asUMLPackage();
                        continue;
                    }
                    o = Object_Factory::createUMLObject(UMLObject::ot_Class, scopeName, parentPkg);
                    o->setStereotypeCmd(QLatin1String("class-or-package"));
                    // setStereotypeCmd() triggers tree view item update if not loading by default
                    if (umldoc->loading()) {
                        UMLListViewItem *item = UMLApp::app()->listView()->findUMLObject(o);
                        if (item)
                            item->updateObject();
                    }
                    parentPkg = o->asUMLPackage();
                    Model_Utils::treeViewSetCurrentItem(o);
                }
                // All scope qualified datatypes live in the global scope.
                bPutAtGlobalScope = true;
            }
            UMLObject::ObjectType t = type;
            if (type == UMLObject::ot_UMLObject || isAdorned)
                t = UMLObject::ot_Class;
            origType = Object_Factory::createUMLObject(t, typeName, parentPkg, false);
            bNewUMLObjectWasCreated = true;
            bPutAtGlobalScope = false;
        }
        if (isConst || isAdorned || isMutable || isVolatile) {
            // Create the full given type (including adornments.)
            if (isVolatile)
                name.prepend(QLatin1String("volatile "));
            if (isMutable)
                name.prepend(QLatin1String("mutable "));
            if (isConst)
                name.prepend(QLatin1String("const "));
            o = Object_Factory::createUMLObject(UMLObject::ot_Datatype, name,
                                                umldoc->datatypeFolder(),
                                                false); //solicitNewName
            UMLDatatype *dt = o ? o->asUMLDatatype() : 0;
            UMLClassifier *c = origType->asUMLClassifier();
            if (dt && c)
                dt->setOriginType(c);
            else
                uError() << "createUMLObject(" << name << "): "
                    << "origType " << typeName << " is not a UMLClassifier";
            if (dt && (isRef || isPointer))
                dt->setIsReference();
            /*
            if (isPointer) {
                UMLObject *pointerDecl = Object_Factory::createUMLObject(UMLObject::ot_Datatype, type);
                UMLClassifier *dt = pointerDecl->asUMLClassifier();
                dt->setOriginType(classifier);
                dt->setIsReference();
                classifier = dt;
            }  */
        } else {
            o = origType;
        }
    } else if (parentPkg && !bPutAtGlobalScope && remapParent) {
        UMLPackage *existingPkg = o->umlPackage();
        if (existingPkg != parentPkg && existingPkg != umldoc->datatypeFolder()) {
            if (existingPkg)
                existingPkg->removeObject(o);
            else
                uError() << "createUMLObject(" << name << "): "
                    << "o->getUMLPackage() was NULL";
            parentPkg->addObject(o);
            o->setUMLPackage(parentPkg);
            // setUMLPackage() triggers tree view item update if not loading by default
            if (umldoc->loading()) {
                UMLListViewItem *item = UMLApp::app()->listView()->findUMLObject(o);
                if (item)
                    item->updateObject();
            }
        }
    }
    QString strippedComment = formatComment(comment);
    if (! strippedComment.isEmpty()) {
        o->setDoc(strippedComment);
    }
    if (o && !stereotype.isEmpty()) {
        o->setStereotype(stereotype);
    }
    if (gRelatedClassifier == 0 || gRelatedClassifier == o)
        return o;
    QRegExp templateInstantiation(QLatin1String("^[\\w:\\.]+\\s*<(.*)>"));
    int pos = templateInstantiation.indexIn(name);
    if (pos == -1)
        return o;
    // Create dependencies on template parameters.
    QString caption = templateInstantiation.cap(1);
    const QStringList params = caption.split(QRegExp(QLatin1String("[^\\w:\\.]+")));
    if (!params.count())
        return o;
    QStringList::ConstIterator end(params.end());
    for (QStringList::ConstIterator it(params.begin()); it != end; ++it) {
        UMLObject *p = umldoc->findUMLObject(*it, UMLObject::ot_UMLObject, parentPkg);
        if (p == 0 || p->isUMLDatatype())
            continue;
        const Uml::AssociationType::Enum at = Uml::AssociationType::Dependency;
        UMLAssociation *assoc = umldoc->findAssociation(at, gRelatedClassifier, p);
        if (assoc)
            continue;
        assoc = new UMLAssociation(at, gRelatedClassifier, p);
        assoc->setUMLPackage(umldoc->rootFolder(Uml::ModelType::Logical));
        umldoc->addAssociation(assoc);
    }
    if (o == 0) {
        uError() << "is NULL!";
    }
    return o;
}

/**
 * Create hierarchical tree of UML objects
 *
 * This method creates the UML object specified by #type and #name including an optional namespace hierarchy
 * if included in the #name e.g. NamespaceA::ClassA in C++.
 *
 * @param type  type of UML object to create
 * @param name  name of UML object
 * @param topLevelParent  UML package to add the hierarchy of UML objects
 * @return pointer to created or found UML object
 */
UMLObject* createUMLObjectHierarchy(UMLObject::ObjectType type, const QString &name, UMLPackage *topLevelParent)
{
    UMLPackage *parent = topLevelParent;
    QString objectName;
    QString scopeSeparator = UMLApp::app()->activeLanguageScopeSeparator();
    UMLObject *o = nullptr;
    if (name.contains(scopeSeparator)) {
        QStringList components =  name.split(scopeSeparator);
        objectName = components.takeLast();
        foreach(const QString scopeName, components) {
            o = parent->findObject(scopeName);
            if (o && (o->isUMLPackage() || o->isUMLClassifier())) {
                parent = o->asUMLPackage();
                continue;
            }
            o = Object_Factory::createNewUMLObject(UMLObject::ot_Class, scopeName, parent, false);
            parent->addObject(o);
            o->setStereotypeCmd(QLatin1String("class-or-package"));
            parent = o->asUMLPackage();
        }
    } else {
        objectName = name;
    }
    o = parent->findObject(objectName);
    if (o && (o->isUMLPackage() || o->isUMLClassifier()))
        return o;
    o = Object_Factory::createNewUMLObject(type, objectName, parent);
    parent->addObject(o);
    return o;
}

/**
 * Create a UMLOperation.
 * The reason for this method is to not generate any Qt signals.
 * Instead, these are generated by insertMethod().
 * (If we generated a creation signal prematurely, i.e. without
 * the method parameters being known yet, then that would lead to
 * a conflict with a pre-existing parameterless method of the same
 * name.)
 */
UMLOperation* makeOperation(UMLClassifier *parent, const QString &name)
{
    UMLOperation *op = Object_Factory::createOperation(parent, name);
    return op;
}

/**
 * Create a UMLAttribute and insert it into the document.
 * Use the specified existing attrType.
 */
UMLAttribute* insertAttribute(UMLClassifier *owner,
                           Uml::Visibility::Enum scope,
                           const QString& name,
                           UMLClassifier *attrType,
                           const QString& comment /* =QString() */,
                           bool isStatic /* =false */)
{
    UMLObject::ObjectType ot = owner->baseType();
    Uml::ProgrammingLanguage::Enum pl = UMLApp::app()->activeLanguage();
    if (! (ot == UMLObject::ot_Class ||
           (ot == UMLObject::ot_Interface && pl == Uml::ProgrammingLanguage::Java))) {
        DEBUG(DBG_SRC) << "insertAttribute: Don not know what to do with "
                 << owner->name() << " (object type " << UMLObject::toString(ot) << ")";
        return 0;
    }
    UMLObject *o = owner->findChildObject(name, UMLObject::ot_Attribute);
    if (o) {
        return o->asUMLAttribute();
    }

    UMLAttribute *attr = owner->addAttribute(name, attrType, scope);
    attr->setStatic(isStatic);
    QString strippedComment = formatComment(comment);
    if (! strippedComment.isEmpty()) {
        attr->setDoc(strippedComment);
    }

    UMLApp::app()->document()->setModified(true);
    return attr;
}

/**
 * Create a UMLAttribute and insert it into the document.
 */
UMLAttribute *insertAttribute(UMLClassifier *owner, Uml::Visibility::Enum scope,
                           const QString& name,
                           const QString& type,
                           const QString& comment /* =QString() */,
                           bool isStatic /* =false */)
{
    UMLObject *attrType = owner->findTemplate(type);
    if (attrType == 0) {
        bPutAtGlobalScope = true;
        gRelatedClassifier = owner;
        attrType = createUMLObject(UMLObject::ot_UMLObject, type, owner);
        gRelatedClassifier = 0;
        bPutAtGlobalScope = false;
    }
    return insertAttribute (owner, scope, name,
                            attrType->asUMLClassifier(),
                            comment, isStatic);
}

/**
  * Insert the UMLOperation into the given classifier.
  *
  * @param klass          The classifier into which the operation shall be added.
  * @param op             Reference to pointer to the temporary UMLOperation
  *                       for insertion.  The caller relinquishes ownership of the
  *                       object pointed to.  If a UMLOperation of same signature
  *                       already exists  at the classifier then the incoming
  *                       UMLOperation is deleted and the pointer is set to the
  *                       existing UMLOperation.
  * @param scope          The Uml::Visibility of the method
  * @param type           The return type
  * @param isStatic       boolean switch to decide if method is static
  * @param isAbstract     boolean switch to decide if method is abstract
  * @param isFriend       true boolean switch to decide if methods is a friend function
  * @param isConstructor  boolean switch to decide if methods is a constructor
  * @param isDestructor   boolean switch to decide if methods is a destructor
  * @param comment        The Documentation for this method
  */
void insertMethod(UMLClassifier *klass, UMLOperation* &op,
                  Uml::Visibility::Enum scope, const QString& type,
                  bool isStatic, bool isAbstract,
                  bool isFriend, bool isConstructor,
                  bool isDestructor, const QString& comment)
{
    op->setVisibilityCmd(scope);
    if (!type.isEmpty()     // return type may be missing (constructor/destructor)
        && type != QLatin1String("void")) {
        if (type == klass->name()) {
            op->setType(klass);
        } else {
            UMLObject *typeObj = klass->findTemplate(type);
            if (typeObj == 0) {
                bPutAtGlobalScope = true;
                gRelatedClassifier = klass;
                typeObj = createUMLObject(UMLObject::ot_UMLObject, type, klass);
                gRelatedClassifier = 0;
                bPutAtGlobalScope = false;
                op->setType(typeObj);
            }
        }
    }

    op->setStatic(isStatic);
    op->setAbstract(isAbstract);

    // if the operation is friend, add it as a stereotype
    if (isFriend)
        op->setStereotype(QLatin1String("friend"));
    // if the operation is a constructor, add it as a stereotype
    if (isConstructor)
        op->setStereotype(QLatin1String("constructor"));
    if (isDestructor)
        op->setStereotype(QLatin1String("destructor"));

    QString strippedComment = formatComment(comment);
    if (! strippedComment.isEmpty()) {
        op->setDoc(strippedComment);
    }

    UMLAttributeList params = op->getParmList();
    UMLOperation *exist = klass->checkOperationSignature(op->name(), params);
    if (exist) {
        // copy contents to existing operation
        exist->setVisibilityCmd(scope);
        exist->setStatic(isStatic);
        exist->setAbstract(isAbstract);
        if (! strippedComment.isEmpty())
            exist->setDoc(strippedComment);
        UMLAttributeList exParams = exist->getParmList();
        for (UMLAttributeListIt it(params), exIt(exParams) ; it.hasNext() ;) {
            UMLAttribute *param = it.next(), *exParam = exIt.next();
            exParam->setName(param->name());
            exParam->setVisibilityCmd(param->visibility());
            exParam->setStatic(param->isStatic());
            exParam->setAbstract(param->isAbstract());
            exParam->setDoc(param->doc());
            exParam->setInitialValue(param->getInitialValue());
            exParam->setParmKind(param->getParmKind());
        }
        // delete incoming UMLOperation and pass out the existing one
        delete op;
        op = exist;
    } else {
        klass->addOperation(op);
    }
}

/**
 * Add an argument to a UMLOperation.
 * The parentPkg arg is only used for resolving possible scope
 * prefixes in the `type'.
 */
UMLAttribute* addMethodParameter(UMLOperation *method,
                                 const QString& type,
                                 const QString& name)
{
    UMLClassifier *owner = method->umlParent()->asUMLClassifier();
    UMLObject *typeObj = owner ? owner->findTemplate(type) : 0;
    if (typeObj == 0) {
        bPutAtGlobalScope = true;
        gRelatedClassifier = owner;
        typeObj = createUMLObject(UMLObject::ot_UMLObject, type, owner);
        gRelatedClassifier = 0;
        bPutAtGlobalScope = false;
    }
    UMLAttribute *attr = Object_Factory::createAttribute(method, name, typeObj);
    method->addParm(attr);
    return attr;
}

/**
 * Add an enum literal to a UMLEnum.
 */
void addEnumLiteral(UMLEnum *enumType, const QString &literal, const QString &comment, const QString &value)
{
    UMLObject *el = enumType->addEnumLiteral(literal, Uml::ID::None, value);
    el->setDoc(comment);
}

/**
 * Create a generalization from the given child classifier to the given
 * parent classifier.
 */
UMLAssociation *createGeneralization(UMLClassifier *child, UMLClassifier *parent)
{
    // if the child is an interface, so is the parent.
    if (child->isInterface())
        parent->setBaseType(UMLObject::ot_Interface);
    Uml::AssociationType::Enum association = Uml::AssociationType::Generalization;

    if (parent->isInterface() && !child->isInterface()) {
        // if the parent is an interface, but the child is not, then
        // this is really realization.
        //
        association = Uml::AssociationType::Realization;
    }
    UMLAssociation *assoc = new UMLAssociation(association, child, parent);
    if (child->umlPackage()) {
        assoc->setUMLPackage(child->umlPackage());
        child->addAssociationEnd(assoc);
    } else {
        uDebug() << "Import_Utils::createGeneralization(child " << child->name()
                 << ", parent " << parent->name() << ") : Package is not set on child";
        UMLDoc *umldoc = UMLApp::app()->document();
        UMLPackage *owningPackage = umldoc->rootFolder(Uml::ModelType::Logical);
        assoc->setUMLPackage(owningPackage);
        umldoc->addAssociation(assoc);
    }
    return assoc;
}

/**
 * Create a subdir with the given name.
 */
UMLFolder *createSubDir(const QString& name,
                        UMLFolder *parentPkg,
                        const QString &comment)
{
    UMLDoc *umldoc = UMLApp::app()->document();
    if (!parentPkg) {
        parentPkg = umldoc->rootFolder(Uml::ModelType::Component);
    }

    UMLObject::ObjectType type = UMLObject::ot_Folder;
    UMLFolder *o = umldoc->findUMLObjectRaw(parentPkg, name, type)->asUMLFolder();
    if (o)
        return o;
    o = Object_Factory::createUMLObject(type, name, parentPkg, false)->asUMLFolder();
    if (o)
        o->setDoc(comment);
    return o;
}


/**
  * Create a folder for artifacts
 */
UMLObject *createArtifactFolder(const QString& name,
                                UMLPackage *parentPkg,
                                const QString &comment)
{
    Q_UNUSED(parentPkg);

    UMLObject::ObjectType type = UMLObject::ot_Folder;
    UMLDoc *umldoc = UMLApp::app()->document();
    UMLFolder *componentView = umldoc->rootFolder(Uml::ModelType::Component);
    UMLObject *o = umldoc->findUMLObjectRaw(componentView, name, type);
    if (o)
        return o;
    o = Object_Factory::createUMLObject(type, name, componentView, false);
    UMLFolder *a = o->asUMLFolder();
    a->setDoc(comment);
    DEBUG(DBG_SRC) << name << comment;
    return o;
}

/**
 * Create an artifact with the given name.
 */
UMLObject *createArtifact(const QString& name,
                          UMLFolder *parentPkg,
                          const QString &comment)
{
    UMLDoc *umldoc = UMLApp::app()->document();
    if (!parentPkg) {
        parentPkg = umldoc->rootFolder(Uml::ModelType::Component);
    }

    UMLObject::ObjectType type = UMLObject::ot_Artifact;
    UMLObject *o = umldoc->findUMLObjectRaw(parentPkg, name, type);
    if (o)
        return o;
    o = Object_Factory::createUMLObject(type, name, parentPkg, false);
    UMLArtifact *a = o->asUMLArtifact();
    a->setDrawAsType(UMLArtifact::file);
    a->setDoc(comment);
    DEBUG(DBG_SRC) << name << comment;
    return o;
}

/**
 * Create a generalization from the existing child UMLObject to the given
 * parent class name.
 * This method does not handle scopes well and is only a last resort.
 * The method
 *   createGeneralization(UMLClassifier *child, UMLClassifier *parent)
 * should be used instead.
 */
void createGeneralization(UMLClassifier *child, const QString &parentName)
{
    const QString& scopeSep = UMLApp::app()->activeLanguageScopeSeparator();
    UMLObject *parentObj = 0;
    if (parentName.contains(scopeSep)) {
        QStringList split = parentName.split(scopeSep);
        QString className = split.last();
        split.pop_back(); // remove the classname
        UMLPackage *parent = 0;
        UMLPackage *current = 0;
        for (QStringList::Iterator it = split.begin(); it != split.end(); ++it) {
            QString name = (*it);
            UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Package,
                                                          name, parent,
                                                          QString(), QString(),
                                                          true, false);
            current = ns->asUMLPackage();
            parent = current;
        }
        UMLObject::ObjectType type = UMLObject::ot_Class;
        if (child->baseType() == UMLObject::ot_Interface)
          type = UMLObject::ot_Interface;
        parentObj = Import_Utils::createUMLObject(type, className, parent,
                                                  QString(), QString(), true, false);
    } else {
        parentObj = createUMLObject(UMLObject::ot_Class, parentName);
    }
    UMLClassifier *parent = parentObj->asUMLClassifier();
    createGeneralization(child, parent);
}

/**
 * Remap UMLObject instance in case it does not have the correct type.
 *
 * @param ns uml object instance with incorrect class
 * @param currentScope parent uml object
 * @return newly created UMLEnum instance or zero in case of error
 */
UMLEnum *remapUMLEnum(UMLObject *ns, UMLPackage *currentScope)
{
    if (ns == 0) {
        return 0;
    }
    QString comment = ns->doc();
    QString name = ns->name();
    QString stereotype = ns->stereotype();
    Uml::Visibility::Enum visibility = ns->visibility();
    UMLApp::app()->document()->removeUMLObject(ns, true);
    if (currentScope == 0)
        currentScope = UMLApp::app()->document()->rootFolder(Uml::ModelType::Logical);
    UMLObject *o = Object_Factory::createNewUMLObject(UMLObject::ot_Enum, name, currentScope, false);
    if (!o) {
        DEBUG(DBG_SRC) << name << " : Object_Factory::createNewUMLObject(ot_Enum) returns null";
        return 0;
    }
    UMLEnum *e = o->asUMLEnum();
    if (!e) {
        DEBUG(DBG_SRC) << name << " : object returned by Object_Factory::createNewUMLObject is not Enum";
        return 0;
    }
    e->setDoc(comment);
    e->setStereotypeCmd(stereotype.isEmpty() ? QLatin1String("enum") : stereotype);
    e->setVisibilityCmd(visibility);
    // add to parents child list
    if (currentScope->addObject(e, false))  // false => non interactively
        return e;
    DEBUG(DBG_SRC) << name << " : name is already present in " << currentScope->name();
    return 0;
}

/**
 * Return the list of paths set by previous calls to addIncludePath()
 * and the environment variable UMBRELLO_INCPATH.
 * This list can be used for finding included (or Ada with'ed or...)
 * files.
 */
QStringList includePathList()
{
    QStringList includePathList(incPathList);
    QString umbrello_incpath = QString::fromLatin1(qgetenv("UMBRELLO_INCPATH"));
    if (!umbrello_incpath.isEmpty()) {
        includePathList += umbrello_incpath.split(PATH_SEPARATOR);
    }
    return includePathList;
}

/**
 * Add a path to the include path list.
 */
void addIncludePath(const QString& path)
{
    if (! incPathList.contains(path))
        incPathList.append(path);
}

/**
 * Returns true if a type is an actual Datatype
 */
bool isDatatype(const QString& name, UMLPackage *parentPkg)
{
    UMLDoc *umldoc = UMLApp::app()->document();
    UMLObject * o = umldoc->findUMLObject(name, UMLObject::ot_Datatype, parentPkg);
    return (o != 0);
}

/**
 * Returns the UML package of the global scope.
 */
UMLPackage *globalScope()
{
    UMLFolder *logicalView = UMLApp::app()->document()->rootFolder(Uml::ModelType::Logical);
    return logicalView;
}

}  // end namespace Import_Utils
