/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/
#include "classifier.h"

// app includes
#include "association.h"
#include "debug_utils.h"
#include "umlassociationlist.h"
#include "operation.h"
#include "attribute.h"
#include "template.h"
#include "enumliteral.h"
#include "entityattribute.h"
#include "enum.h"
#include "entity.h"
#include "stereotype.h"
#include "umldoc.h"
#include "uml.h"
#include "uniqueid.h"
#include "object_factory.h"
#include "model_utils.h"
#include "idchangelog.h"
#include "umloperationdialog.h"
#include "umlattributedialog.h"
#include "umltemplatedialog.h"
#include "optionstate.h"
#include "icon_utils.h"
#include "instance.h"
#include "instanceattribute.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QPointer>

using namespace Uml;

/**
 * @brief holds set of classifiers for recursive loop detection
 */
class UMLClassifierSet: public QSet<UMLClassifier *> {
public:
    UMLClassifierSet() : level(0)
    {
    }
    int level;
};

/**
 * Sets up a Classifier.
 *
 * @param name   The name of the Concept.
 * @param id     The unique id of the Concept.
 */
UMLClassifier::UMLClassifier(const QString & name, Uml::ID::Type id)
  : UMLPackage(name, id)
{
    m_BaseType = UMLObject::ot_Class;  // default value
    m_pClassAssoc = 0;
}

/**
 * Standard deconstructor.
 */
UMLClassifier::~UMLClassifier()
{
}

/**
 * Reimplementation of method from class UMLObject for controlling the
 * exact type of this classifier: class, interface, or datatype.
 * @param ot   the base type to set
 */
void UMLClassifier::setBaseType(UMLObject::ObjectType ot)
{
    m_BaseType = ot;
    Icon_Utils::IconType newIcon;
    switch (ot) {
        case ot_Interface:
            UMLObject::setStereotypeCmd(QLatin1String("interface"));
            UMLObject::m_bAbstract = true;
            newIcon = Icon_Utils::it_Interface;
            break;
        case ot_Class:
            UMLObject::setStereotypeCmd(QString());
            UMLObject::m_bAbstract = false;
            newIcon = Icon_Utils::it_Class;
            break;
        case ot_Datatype:
            UMLObject::setStereotypeCmd(QLatin1String("datatype"));
            UMLObject::m_bAbstract = false;
            newIcon = Icon_Utils::it_Datatype;
            break;
        case ot_Package:
            UMLObject::setStereotypeCmd(QString());
            UMLObject::m_bAbstract = false;
            newIcon = Icon_Utils::it_Package;
            break;
        default:
            uError() << "cannot set to type " << ot;
            return;
    }
    Model_Utils::treeViewChangeIcon(this, newIcon);
}

/**
 * Returns true if this classifier represents an interface.
 */
bool UMLClassifier::isInterface() const
{
    return (m_BaseType == ot_Interface);
}

/**
 * Checks whether an operation is valid based on its signature -
 * An operation is "valid" if the operation's name and parameter list
 * are unique in the classifier.
 *
 * @param name      Name of the operation to check.
 * @param opParams  The operation's argument list.
 * @param exemptOp  Pointer to the exempt method (optional.)
 * @return  NULL if the signature is valid (ok), else return a pointer
 *          to the existing UMLOperation that causes the conflict.
 */
UMLOperation * UMLClassifier::checkOperationSignature(
        const QString& name,
        UMLAttributeList opParams,
        UMLOperation *exemptOp)
{
    UMLOperationList list = findOperations(name);
    if (list.count() == 0) {
        return 0;
    }
    const int inputParmCount = opParams.count();

    // there is at least one operation with the same name... compare the parameter list
    foreach (UMLOperation* test, list) {
        if (test == exemptOp) {
            continue;
        }
        UMLAttributeList testParams = test->getParmList();
        const int pCount = testParams.count();
        if (pCount != inputParmCount) {
            continue;
        }
        int i = 0;
        while (i < pCount) {
            // The only criterion for equivalence is the parameter types.
            // (Default values are not considered.)
            if(testParams.at(i)->getTypeName() != opParams.at(i)->getTypeName())
                break;
            i++;
        }
        if (i == pCount) { // all parameters matched->the signature is not unique
            return test;
        }
    }
    // we did not find an exact match, so the signature is unique (acceptable)
    return 0;
}

/**
 * Find an operation of the given name and parameter signature.
 *
 * @param name     The name of the operation to find.
 * @param params   The parameter descriptors of the operation to find.
 *
 * @return  The operation found.  Will return 0 if none found.
 */
UMLOperation* UMLClassifier::findOperation(const QString& name,
                                           Model_Utils::NameAndType_List params)
{
    UMLOperationList list = findOperations(name);
    if (list.count() == 0) {
        return 0;
    }
    // if there are operation(s) with the same name then compare the parameter list
    const int inputParmCount = params.count();

    foreach (UMLOperation* test, list) {
        UMLAttributeList testParams = test->getParmList();
        const int pCount = testParams.count();
        if (inputParmCount == 0 && pCount == 0)
            return test;
        if (inputParmCount != pCount)
            continue;
        int i = 0;
        for (; i < pCount; ++i) {
            Model_Utils::NameAndType_ListIt nt(params.begin() + i);
            UMLClassifier *type = (*nt).m_type->asUMLClassifier();
            UMLClassifier *testType = testParams.at(i)->getType();
            if (type == 0 && testType == 0) { //no parameter type
                continue;
            } else if (type == 0) {  //template parameter
                if (testType->name() != QLatin1String("class"))
                    break;
            } else if (type != testType)
                break;
        }
        if (i == pCount)
            return test;  // all parameters matched
    }
    return 0;
}

/**
 * Creates an operation in the current document.
 * The new operation is initialized with name, id, etc.
 * If a method with the given profile already exists in the classifier,
 * no new method is created and the existing operation is returned.
 * If no name is provided, or if the params are 0, an Operation
 * Dialog is shown to ask the user for a name and parameters.
 * The operation's signature is checked for validity within the parent
 * classifier.
 *
 * @param name           The operation name (will be chosen internally if
 *                       none given.)
 * @param isExistingOp   Optional pointer to bool. If supplied, the bool is
 *                       set to true if an existing operation is returned.
 * @param params         Optional list of parameter names and types.
 *                       If supplied, new operation parameters are
 *                       constructed using this list.
 * @return The new operation, or NULL if the operation could not be
 *         created because for example, the user canceled the dialog
 *         or no appropriate name can be found.
 */
UMLOperation* UMLClassifier::createOperation(
        const QString &name /*=QString()*/,
        bool *isExistingOp  /*=0*/,
        Model_Utils::NameAndType_List *params  /*=0*/)
{
    bool nameNotSet = (name.isNull() || name.isEmpty());
    if (! nameNotSet) {
        Model_Utils::NameAndType_List parList;
        if (params)
            parList = *params;
        UMLOperation* existingOp = findOperation(name, parList);
        if (existingOp != 0) {
            if (isExistingOp != 0)
                *isExistingOp = true;
            return existingOp;
        }
    }
    // we did not find an exact match, so the signature is unique
    UMLOperation *op = new UMLOperation(this, name);
    if (params) {
        for (Model_Utils::NameAndType_ListIt it = params->begin(); it != params->end(); ++it) {
            const Model_Utils::NameAndType &nt = *it;
            UMLAttribute *par = new UMLAttribute(op, nt.m_name, Uml::ID::None, Uml::Visibility::Private,
                                                 nt.m_type, nt.m_initialValue);
            par->setParmKind(nt.m_direction);
            op->addParm(par);
        }
    }

    if (isInterface()) {
        op->setAbstract(true);
        op->setVirtual(true);
    }

    // Only show the operation dialog if no name was provided (allows quick-create
    // from listview)
    if (nameNotSet) {
        op->setName(uniqChildName(UMLObject::ot_Operation));

        while (true) {
            QPointer<UMLOperationDialog> operationDialog = new UMLOperationDialog(0, op);
            if(operationDialog->exec() != QDialog::Accepted) {
                delete op;
                delete operationDialog;
                return 0;
            } else if (checkOperationSignature(op->name(), op->getParmList())) {
                KMessageBox::information(0,
                                         i18n("An operation with the same name and signature already exists. You cannot add it again."));
            } else {
                break;
            }
            delete operationDialog;
        }
    }

    // operation name is ok, formally add it to the classifier
    if (! addOperation(op)) {
        delete op;
        return 0;
    }

    UMLDoc *umldoc = UMLApp::app()->document();
    umldoc->signalUMLObjectCreated(op);
    return op;
}

/**
 * Appends an operation to the classifier.
 * This function is mainly intended for the clipboard.
 *
 * @param op         Pointer to the UMLOperation to add.
 * @param position   Inserted at the given position.
 * @return           True if the operation was added successfully.
 */
bool UMLClassifier::addOperation(UMLOperation* op, int position)
{
    Q_ASSERT(op);
    if (subordinates().indexOf(op) != -1) {
        uDebug() << "findRef(" << op->name() << ") finds op (bad)";
        return false;
    }
    if (checkOperationSignature(op->name(), op->getParmList())) {
        uDebug() << "checkOperationSignature(" << op->name() << ") op is non-unique";
        return false;
    }

    if (position >= 0 && position <= subordinates().count()) {
        uDebug() << op->name() << ": inserting at position " << position;
        subordinates().insert(position, op);
        UMLClassifierListItemList itemList = getFilteredList(UMLObject::ot_Operation);
        QString buf;
        foreach (UMLClassifierListItem* currentAtt, itemList) {
            buf.append(QLatin1Char(' ') + currentAtt->name());
        }
        uDebug() << "  list after change: " << buf;
    }
    else {
        subordinates().append(op);
    }
    emit operationAdded(op);
    UMLObject::emitModified();
    connect(op, SIGNAL(modified()), this, SIGNAL(modified()));
    return true;
}

/**
 * Appends an operation to the classifier.
 * @see bool addOperation(UMLOperation* Op, int position = -1)
 * This function is mainly intended for the clipboard.
 *
 * @param op    Pointer to the UMLOperation to add.
 * @param log   Pointer to the IDChangeLog.
 * @return      True if the operation was added successfully.
 */
bool UMLClassifier::addOperation(UMLOperation* op, IDChangeLog* log)
{
    if (addOperation(op, -1)) {
        return true;
    }
    else if (log) {
        log->removeChangeByNewID(op->id());
    }
    return false;
}

/**
 * Remove an operation from the Classifier.
 * The operation is not deleted so the caller is responsible for what
 * happens to it after this.
 *
 * @param op   The operation to remove.
 * @return     Count of the remaining operations after removal, or
 *             -1 if the given operation was not found.
 */
int UMLClassifier::removeOperation(UMLOperation *op)
{
    if (op == 0) {
        uDebug() << "called on NULL op";
        return -1;
    }
    if (!subordinates().removeAll(op)) {
        uDebug() << "cannot find op " << op->name() << " in list";
        return -1;
    }
    // disconnection needed.
    // note that we don't delete the operation, just remove it from the Classifier
    disconnect(op, SIGNAL(modified()), this, SIGNAL(modified()));
    emit operationRemoved(op);
    UMLObject::emitModified();
    return subordinates().count();
}

/**
 * Create and add a just created template.
 * @param currentName   the name of the template
 * @return              the template or NULL
 */
UMLObject* UMLClassifier::createTemplate(const QString& currentName /*= QString()*/)
{
    QString name = currentName;
    bool goodName = !name.isEmpty();
    if (!goodName) {
        name = uniqChildName(UMLObject::ot_Template);
    }
    UMLTemplate* newTemplate = new UMLTemplate(this, name);

    int button = QDialog::Accepted;

    while (button == QDialog::Accepted && !goodName) {
        QPointer<UMLTemplateDialog> templateDialog = new UMLTemplateDialog(0, newTemplate);
        button = templateDialog->exec();
        name = newTemplate->name();

        if (name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        }
        else if (findChildObject(name) != 0) {
            KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
        }
        else {
            goodName = true;
        }
        delete templateDialog;
    }

    if (button != QDialog::Accepted) {
        return 0;
    }

    addTemplate(newTemplate);

    UMLDoc *umldoc = UMLApp::app()->document();
    umldoc->signalUMLObjectCreated(newTemplate);
    return newTemplate;
}

/**
 * Returns the attributes for the specified scope.
 * @return   List of true attributes for the class.
 */
UMLAttributeList UMLClassifier::getAttributeList() const
{
    UMLAttributeList attributeList;
    foreach (UMLObject* listItem, subordinates()) {
        if (listItem->baseType() == UMLObject::ot_Attribute) {
            attributeList.append(listItem->asUMLAttribute());
        }
    }
    return attributeList;
}

/**
 * Returns the attributes for the specified scope.
 * @param scope   The scope of the attribute.
 * @return        List of true attributes for the class.
 */
UMLAttributeList UMLClassifier::getAttributeList(Visibility::Enum scope) const
{
    UMLAttributeList list;
    if (!isInterface())
    {
        UMLAttributeList atl = getAttributeList();
        foreach(UMLAttribute* at, atl)
        {
            uIgnoreZeroPointer(at);
            if (! at->isStatic())
            {
                if (scope == Uml::Visibility::Private)
                {
                    if ((at->visibility() == Uml::Visibility::Private) ||
                       (at->visibility() == Uml::Visibility::Implementation))
                    {
                        list.append(at);
                    }
                }
                else if (scope == at->visibility())
                {
                   list.append(at);
                }
            }
        }
    }
    return list;
}

/**
 * Returns the static attributes for the specified scope.
 *
 * @param scope   The scope of the attribute.
 * @return        List of true attributes for the class.
 */
UMLAttributeList UMLClassifier::getAttributeListStatic(Visibility::Enum scope) const
{
    UMLAttributeList list;
    if (!isInterface())
    {
        UMLAttributeList atl = getAttributeList();
        foreach(UMLAttribute* at, atl)
        {
            uIgnoreZeroPointer(at);
            if (at->isStatic())
            {
                if (scope == Uml::Visibility::Private)
                {
                    if ((at->visibility() == Uml::Visibility::Private) ||
                       (at->visibility() == Uml::Visibility::Implementation))
                    {
                        list.append(at);
                    }
                }
                else if (scope == at->visibility())
                {
                    list.append(at);
                }
            }
        }
    }
    return list;
}

/**
 * Find a list of operations with the given name.
 *
 * @param n   The name of the operation to find.
 * @return    The list of objects found; will be empty if none found.
 */
UMLOperationList UMLClassifier::findOperations(const QString &n)
{
    const bool caseSensitive = UMLApp::app()->activeLanguageIsCaseSensitive();
    UMLOperationList list;
    foreach (UMLObject*  obj, subordinates()) {
        if (obj->baseType() != UMLObject::ot_Operation)
            continue;
        UMLOperation *op = obj->asUMLOperation();
        if (caseSensitive) {
            if (obj->name() == n)
                list.append(op);
        } else if (obj->name().toLower() == n.toLower()) {
            list.append(op);
        }
    }
    return list;
}

/**
 * Find the child object by the given id.
 * @param id                  the id of the child object
 * @param considerAncestors   flag whether the ancestors should be considered during search
 * @return                    the found child object or NULL
 */
UMLObject* UMLClassifier::findChildObjectById(Uml::ID::Type id, bool considerAncestors /* =false */)
{
    UMLObject *o = UMLCanvasObject::findChildObjectById(id);
    if (o) {
        return o;
    }
    if (considerAncestors) {
        UMLClassifierList ancestors = findSuperClassConcepts();
        foreach (UMLClassifier *anc, ancestors) {
            UMLObject *o = anc->findChildObjectById(id);
            if (o) {
                return o;
            }
        }
    }
    return 0;
}

/**
 * Returns a list of concepts which inherit from this concept.
 *
 * @param type   The ClassifierType to seek.
 * @return       List of UMLClassifiers that inherit from us.
 */
UMLClassifierList UMLClassifier::findSubClassConcepts (ClassifierType type)
{
    UMLClassifierList list = getSubClasses();
    UMLAssociationList rlist = getRealizations();

    UMLClassifierList inheritingConcepts;
    Uml::ID::Type myID = id();
    foreach(UMLClassifier *c, list) {
        uIgnoreZeroPointer(c);
        if (type == ALL || (!c->isInterface() && type == CLASS)
                || (c->isInterface() && type == INTERFACE)) {
            inheritingConcepts.append(c);
        }
    }

    foreach (UMLAssociation *a, rlist) {
        uIgnoreZeroPointer(a);
        if (a->getObjectId(RoleType::A) != myID)
        {
            UMLObject* obj = a->getObject(RoleType::A);
            UMLClassifier *concept = obj->asUMLClassifier();
            if (concept && (type == ALL || (!concept->isInterface() && type == CLASS)
                            || (concept->isInterface() && type == INTERFACE))
                        && (inheritingConcepts.indexOf(concept) == -1))
            {
                inheritingConcepts.append(concept);
            }
        }
    }

    return inheritingConcepts;
}

/**
 * Returns a list of concepts which this concept inherits from.
 *
 * @param type   The ClassifierType to seek.
 * @return       List of UMLClassifiers we inherit from.
 */
UMLClassifierList UMLClassifier::findSuperClassConcepts (ClassifierType type)
{
    UMLClassifierList list = getSuperClasses();
    UMLAssociationList rlist = getRealizations();

    UMLClassifierList parentConcepts;
    Uml::ID::Type myID = id();
    foreach (UMLClassifier *concept, list) {
        uIgnoreZeroPointer(concept);
        if (type == ALL || (!concept->isInterface() && type == CLASS)
                || (concept->isInterface() && type == INTERFACE))
            parentConcepts.append(concept);
    }

    foreach (UMLAssociation *a, rlist) {
        if (a->getObjectId(RoleType::A) == myID)
        {
            UMLObject* obj = a->getObject(RoleType::B);
            UMLClassifier *concept = obj->asUMLClassifier();
            if (concept && (type == ALL || (!concept->isInterface() && type == CLASS)
                            || (concept->isInterface() && type == INTERFACE))
                        && (parentConcepts.indexOf(concept) == -1))
                parentConcepts.append(concept);
        }
    }

    return parentConcepts;
}

/**
 * Copy the internal presentation of this object into the new
 * object.
 */
void UMLClassifier::copyInto(UMLObject *lhs) const
{
    UMLClassifier *target = lhs->asUMLClassifier();
    UMLCanvasObject::copyInto(target);
    target->setBaseType(m_BaseType);
    // CHECK: association property m_pClassAssoc is not copied
    subordinates().copyInto(&(target->subordinates()));
    foreach(UMLObject *o, target->subordinates()) {
        o->setUMLParent(target);
    }
}

/**
 * Make a clone of this object.
 */
UMLObject* UMLClassifier::clone() const
{
    UMLClassifier *clone = new UMLClassifier();
    copyInto(clone);
    return clone;
}

/**
 * override setting name of classifier
 * @param strName name to set
 */
void UMLClassifier::setNameCmd(const QString &strName)
{
    if (UMLApp::app()->activeLanguage() == Uml::ProgrammingLanguage::Cpp ||
            UMLApp::app()->activeLanguage() == Uml::ProgrammingLanguage::CSharp ||
            UMLApp::app()->activeLanguage() == Uml::ProgrammingLanguage::Java) {
        foreach(UMLOperation *op, getOpList()) {
            if (op->isConstructorOperation())
                op->setNameCmd(strName);
            if (op->isDestructorOperation())
                op->setNameCmd(QLatin1String("~") + strName);
        }
    }
    UMLObject::setNameCmd(strName);
}

/**
 * Needs to be called after all UML objects are loaded from file.
 * Calls the parent resolveRef(), and calls resolveRef() on all
 * UMLClassifierListItems.
 * Overrides the method from UMLObject.
 *
 * @return  true for success.
 */
bool UMLClassifier::resolveRef()
{
    bool success = UMLPackage::resolveRef();
    // Using reentrant iteration is a bare necessity here:
    foreach (UMLObject* obj, subordinates()) {
        /**** For reference, here is the non-reentrant iteration scheme -
              DO NOT USE THIS !
        for (UMLObject *obj = subordinates().first(); obj; obj = subordinates().next())
         {  ....  }
         ****/
        if (obj->resolveRef()) {
            UMLClassifierListItem *cli = obj->asUMLClassifierListItem();
            if (!cli)
                continue;
            switch (cli->baseType()) {
                case UMLObject::ot_Attribute:
                    emit attributeAdded(cli);
                    break;
                case UMLObject::ot_Operation:
                    emit operationAdded(cli);
                    break;
                case UMLObject::ot_Template:
                    emit templateAdded(cli);
                    break;
                default:
                    break;
            }
        }
    }
    return success;
}

/**
 * Reimplemented from UMLObject.
 */
bool UMLClassifier::acceptAssociationType(AssociationType::Enum type)
{
    switch(type)
    {
        case AssociationType::Generalization:
        case AssociationType::Aggregation:
        case AssociationType::Relationship:
        case AssociationType::Dependency:
        case AssociationType::Association:
        case AssociationType::Association_Self:
        case AssociationType::Containment:
        case AssociationType::Composition:
        case AssociationType::Realization:
        case AssociationType::UniAssociation:
            return true;
        default:
            return false;
    }
    return false; //shutup compiler warning
}

/**
 * Creates an attribute for the class.
 *
 * @param name  An optional name, used by when creating through UMLListView
 * @param type  An optional type, used by when creating through UMLListView
 * @param vis   An optional visibility, used by when creating through UMLListView
 * @param init  An optional initial value, used by when creating through UMLListView
 * @return      The UMLAttribute created
 */
UMLAttribute* UMLClassifier::createAttribute(const QString &name,
                                             UMLObject *type,
                                             Visibility::Enum vis,
                                             const QString &init)
{
    Uml::ID::Type id = UniqueID::gen();
    QString currentName;
    if (name.isNull())  {
        currentName = uniqChildName(UMLObject::ot_Attribute);
    } else {
        currentName = name;
    }
    UMLAttribute* newAttribute = new UMLAttribute(this, currentName, id, vis, type, init);

    int button = QDialog::Accepted;
    bool goodName = false;

    //check for name.isNull() stops dialog being shown
    //when creating attribute via list view
    while (button == QDialog::Accepted && !goodName && name.isNull()) {
        QPointer<UMLAttributeDialog> attributeDialog = new UMLAttributeDialog(0, newAttribute);
        button = attributeDialog->exec();
        QString name = newAttribute->name();

        if(name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        } else if (findChildObject(name) != 0) {
            KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
        } else {
            goodName = true;
        }
        delete attributeDialog;
    }

    if (button != QDialog::Accepted) {
        delete newAttribute;
        return 0;
    }

    addAttribute(newAttribute);

    UMLDoc *umldoc = UMLApp::app()->document();
    umldoc->signalUMLObjectCreated(newAttribute);
    return newAttribute;
}

/**
 * Creates and adds an attribute for the class.
 *
 * @param name  an optional name, used by when creating through UMLListView
 * @param id    an optional id
 * @return      the UMLAttribute created and added
 */

UMLAttribute* UMLClassifier::addAttribute(const QString &name, Uml::ID::Type id /* = Uml::id_None */)
{
    foreach (UMLObject* obj, subordinates()) {
        uIgnoreZeroPointer(obj);
        if (obj->baseType() == UMLObject::ot_Attribute && obj->name() == name)
            return obj->asUMLAttribute();
    }
    Uml::Visibility::Enum scope = Settings::optionState().classState.defaultAttributeScope;
    UMLAttribute *a = new UMLAttribute(this, name, id, scope);
    subordinates().append(a);
    emit attributeAdded(a);
    UMLObject::emitModified();
    connect(a, SIGNAL(modified()), this, SIGNAL(modified()));
    return a;
}

/**
 * Adds an already created attribute.
 * The attribute object must not belong to any other concept.
 *
 * @param name    the name of the attribute
 * @param type    the type of the attribute 
 * @param scope   the visibility of the attribute
 * @return        the just created and added attribute
 */
UMLAttribute* UMLClassifier::addAttribute(const QString &name, UMLObject *type, Visibility::Enum scope)
{
    UMLAttribute *a = new UMLAttribute(this);
    a->setName(name);
    a->setVisibility(scope);
    a->setID(UniqueID::gen());
    if (type) {
        a->setType(type);
    }
    subordinates().append(a);
    emit attributeAdded(a);
    UMLObject::emitModified();
    connect(a, SIGNAL(modified()), this, SIGNAL(modified()));
    return a;
}

/**
 * Adds an already created attribute.
 * The attribute object must not belong to any other concept.
 *
 * @param att        Pointer to the UMLAttribute.
 * @param log        Pointer to the IDChangeLog (optional.)
 * @param position   Position index for the insertion (optional.)
 *                   If the position is omitted, or if it is
 *                   negative or too large, the attribute is added
 *                   to the end of the list.
 * @return           True if the attribute was successfully added.
 */
bool UMLClassifier::addAttribute(UMLAttribute* att, IDChangeLog* log /* = 0 */,
                                 int position /* = -1 */)
{
    Q_ASSERT(att);
    if (findChildObject(att->name()) == 0) {
        att->setParent(this);
        if (position >= 0 && position < (int)subordinates().count()) {
            subordinates().insert(position, att);
        }
        else {
            subordinates().append(att);
        }
        emit attributeAdded(att);
        UMLObject::emitModified();
        connect(att, SIGNAL(modified()), this, SIGNAL(modified()));
        return true;
    } else if (log) {
        log->removeChangeByNewID(att->id());
        delete att;
    }
    return false;
}

/**
 * Removes an attribute from the class.
 *
 * @param att   The attribute to remove.
 * @return      Count of the remaining attributes after removal.
 *              Returns -1 if the given attribute was not found.
 */
int UMLClassifier::removeAttribute(UMLAttribute* att)
{
    if (!subordinates().removeAll(att)) {
        uDebug() << "cannot find att given in list";
        return -1;
    }
    // note that we don't delete the attribute, just remove it from the Classifier
    disconnect(att, SIGNAL(modified()), this, SIGNAL(modified()));
    emit attributeRemoved(att);
    UMLObject::emitModified();
    return subordinates().count();
}

/**
 * Return true if this classifier has abstract operations.
 */
bool UMLClassifier::hasAbstractOps ()
{
    UMLOperationList opl(getOpList());
    foreach(UMLOperation *op, opl) {
        uIgnoreZeroPointer(op);
        if (op->isAbstract()) {
            return true;
        }
    }
    return false;
}

/**
 * Counts the number of operations in the Classifier.
 *
 * @return   The number of operations for the Classifier.
 */
int UMLClassifier::operations()
{
    return getOpList().count();
}


/**
 * Return a list of operations for the Classifier.
 *
 * @param includeInherited   Includes operations from superclasses.
 * @param alreadyTraversed   internal used object to avoid recursive loops
 * @return   The list of operations for the Classifier.
 */
UMLOperationList UMLClassifier::getOpList(bool includeInherited, UMLClassifierSet *alreadyTraversed)
{
    UMLOperationList ops;
    foreach (UMLObject* li, subordinates()) {
        uIgnoreZeroPointer(li);
        if (li->baseType() == ot_Operation) {
            ops.append(li->asUMLOperation());
        }
    }
    if (includeInherited) {
        if (!alreadyTraversed) {
            alreadyTraversed = new UMLClassifierSet;
        }
        else
            alreadyTraversed->level++;

        if (!alreadyTraversed->contains(this))
            *alreadyTraversed << this;

        // get a list of parents of this class
        UMLClassifierList parents = findSuperClassConcepts();
        foreach(UMLClassifier *c, parents) {
            if (alreadyTraversed->contains(c)) {
                uError() << "class " << c->name() << " is starting a dependency loop!";
                continue;
            }
            // get operations for each parent by recursive call
            UMLOperationList pops = c->getOpList(true, alreadyTraversed);
            // add these operations to operation list, but only if unique.
            foreach (UMLOperation *po, pops) {
                QString po_as_string(po->toString(Uml::SignatureType::SigNoVis));
                bool breakFlag = false;
                foreach (UMLOperation* o,  ops) {
                    if (o->toString(Uml::SignatureType::SigNoVis) == po_as_string) {
                        breakFlag = true;
                        break;
                    }
                }
                if (breakFlag == false)
                    ops.append(po);
            }
            // remember this node
            *alreadyTraversed << c;
        }
        if (alreadyTraversed->level-- == 0) {
            delete alreadyTraversed;
            alreadyTraversed = 0;
        }
    }
    return ops;
}

/**
 * Returns the entries in subordinates that are of the requested type.
 * If the requested type is UMLObject::ot_UMLObject then all entries
 * are returned.
 * @param ot   the requested object type
 * @return     The list of true operations for the Concept.
 */
UMLClassifierListItemList UMLClassifier::getFilteredList(UMLObject::ObjectType ot) const
{
    UMLClassifierListItemList resultList;
    foreach (UMLObject* o, subordinates()) {
        uIgnoreZeroPointer(o);
        if (!o || o->baseType() == UMLObject::ot_Association) {
            continue;
        }
        UMLClassifierListItem *listItem = o->asUMLClassifierListItem();
        if (!listItem)
            continue;
        if (ot == UMLObject::ot_UMLObject || listItem->baseType() == ot) {
            resultList.append(listItem);
        }
    }
    return resultList;
}

/**
 * Adds an already created template.
 * The template object must not belong to any other concept.
 *
 * @param name   the name of the template
 * @param id     the id of the template
 * @return       the added template
 */
UMLTemplate* UMLClassifier::addTemplate(const QString &name, Uml::ID::Type id)
{
    UMLTemplate *templt = findTemplate(name);
    if (templt) {
        return templt;
    }
    templt = new UMLTemplate(this, name, id);
    subordinates().append(templt);
    emit templateAdded(templt);
    UMLObject::emitModified();
    connect(templt, SIGNAL(modified()), this, SIGNAL(modified()));
    return templt;
}

/**
 * Adds an already created template.
 * The template object must not belong to any other concept.
 *
 * @param newTemplate   Pointer to the UMLTemplate object to add.
 * @param log           Pointer to the IDChangeLog.
 * @return              True if the template was successfully added.
 */
bool UMLClassifier::addTemplate(UMLTemplate* newTemplate, IDChangeLog* log /* = 0*/)
{
    QString name = newTemplate->name();
    if (findChildObject(name) == 0) {
        newTemplate->setParent(this);
        subordinates().append(newTemplate);
        emit templateAdded(newTemplate);
        UMLObject::emitModified();
        connect(newTemplate, SIGNAL(modified()), this, SIGNAL(modified()));
        return true;
    }
    else if (log) {
        log->removeChangeByNewID(newTemplate->id());
        delete newTemplate;
    }
    return false;
}

/**
 * Adds an template to the class.
 * The template object must not belong to any other class.
 * TODO: If the param IDChangeLog from the method above is not being used,
 * give position a default value of -1 and the method can replace the above one.
 * @param templt     Pointer to the UMLTemplate to add.
 * @param position   The position of the template in the list.
 *                   A value of -1 will add the template at the end.
 * @return  True if the template was successfully added.
 */
bool UMLClassifier::addTemplate(UMLTemplate* templt, int position)
{
    Q_ASSERT(templt);
    QString name = templt->name();
    if (findChildObject(name) == 0) {
        templt->setParent(this);
        if (position >= 0 && position <= (int)subordinates().count()) {
            subordinates().insert(position, templt);
        }
        else {
            subordinates().append(templt);
        }
        emit templateAdded(templt);
        UMLObject::emitModified();
        connect(templt, SIGNAL(modified()), this, SIGNAL(modified()));
        return true;
    }
    //else
    return false;
}

/**
 * Removes a template from the class.
 *
 * @param umltemplate   The template to remove.
 * @return  Count of the remaining templates after removal.
 *          Returns -1 if the given template was not found.
 */
int UMLClassifier::removeTemplate(UMLTemplate* umltemplate)
{
    if (!subordinates().removeAll(umltemplate)) {
        uWarning() << "cannot find att given in list";
        return -1;
    }
    emit templateRemoved(umltemplate);
    UMLObject::emitModified();
    disconnect(umltemplate, SIGNAL(modified()), this, SIGNAL(modified()));
    return subordinates().count();
}

/**
 * Seeks the template parameter of the given name.
 * @param name   the template name
 * @return       the found template or 0
 */
UMLTemplate *UMLClassifier::findTemplate(const QString& name)
{
    UMLTemplateList templParams = getTemplateList();
    foreach (UMLTemplate *templt, templParams) {
        if (templt->name() == name) {
            return templt;
        }
    }
    return 0;
}

/**
 * Returns the number of templates for the class.
 *
 * @return  The number of templates for the class.
 */
int UMLClassifier::templates()
{
    UMLClassifierListItemList tempList = getFilteredList(UMLObject::ot_Template);
    return tempList.count();
}

/**
 * Returns the templates.
 * Same as UMLClassifier::getFilteredList(ot_Template) but
 * return type is a true UMLTemplateList.
 *
 * @return  Pointer to the list of true templates for the class.
 */
UMLTemplateList UMLClassifier::getTemplateList() const
{
    UMLTemplateList templateList;
    foreach (UMLObject* listItem, subordinates()) {
        uIgnoreZeroPointer(listItem);
        if (listItem->baseType() == UMLObject::ot_Template) {
            templateList.append(listItem->asUMLTemplate());
        }
    }
    return templateList;
}

/**
 * Take and return a subordinate item from this classifier.
 * Ownership of the item is passed to the caller.
 *
 * @param item   Subordinate item to take.
 * @return       Index in subordinates of the item taken.
 *               Return -1 if the item is not found in subordinates.
 */
int UMLClassifier::takeItem(UMLClassifierListItem *item)
{
    QString buf;
    foreach (UMLObject* currentAtt, subordinates()) {
        uIgnoreZeroPointer(currentAtt);
        QString txt = currentAtt->name();
        if (txt.isEmpty()) {
           txt = QLatin1String("Type-") + QString::number((int) currentAtt->baseType());
        }
        buf.append(QLatin1Char(' ') + currentAtt->name());
    }
    uDebug() << "  UMLClassifier::takeItem (before): subordinates() is " << buf;

    int index = subordinates().indexOf(item);
    if (index == -1) {
        return -1;
    }
    switch (item->baseType()) {
        case UMLObject::ot_Operation: {
            if (removeOperation(item->asUMLOperation()) < 0) {
                index = -1;
            }
            break;
        }
        case UMLObject::ot_Attribute: {
            UMLAttribute *retval = subordinates().takeAt(index)->asUMLAttribute();
            if (retval) {
                emit attributeRemoved(retval);
                UMLObject::emitModified();
            } else {
                index = -1;
            }
            break;
        }
        case UMLObject::ot_Template: {
            UMLTemplate *templt = subordinates().takeAt(index)->asUMLTemplate();
            if (templt) {
                emit templateRemoved(templt);
                UMLObject::emitModified();
            } else {
                index = -1;
            }
            break;
        }
        case UMLObject::ot_EnumLiteral: {
            UMLEnumLiteral *el = subordinates().takeAt(index)->asUMLEnumLiteral();
            if (el) {
                UMLEnum *e = this->asUMLEnum();
                e->signalEnumLiteralRemoved(el);
                UMLObject::emitModified();
            } else {
                index = -1;
            }
            break;
        }
        case UMLObject::ot_EntityAttribute: {
            UMLEntityAttribute* el = subordinates().takeAt(index)->asUMLEntityAttribute();
            if (el) {
                UMLEntity *e = this->asUMLEntity();
                e->signalEntityAttributeRemoved(el);
                UMLObject::emitModified();
            } else {
                index = -1;
            }
            break;
        }
        default:
            index = -1;
            break;
    }
    return index;
}

/**
 * Return true if this classifier has associations.
 * @return   true if classifier has associations
 */
bool UMLClassifier::hasAssociations()
{
    return getSpecificAssocs(AssociationType::Association).count() > 0
            || getAggregations().count() > 0
            || getCompositions().count() > 0
            || getUniAssociationToBeImplemented().count() > 0;
}

/**
 * Return true if this classifier has attributes.
 */
bool UMLClassifier::hasAttributes()
{
    return getAttributeList(Uml::Visibility::Public).count() > 0
            || getAttributeList(Uml::Visibility::Protected).count() > 0
            || getAttributeList(Uml::Visibility::Private).count() > 0
            || getAttributeListStatic(Uml::Visibility::Public).count() > 0
            || getAttributeListStatic(Uml::Visibility::Protected).count() > 0
            || getAttributeListStatic(Uml::Visibility::Private).count() > 0;
}

/**
 * Return true if this classifier has static attributes.
 */
bool UMLClassifier::hasStaticAttributes()
{
    return getAttributeListStatic(Uml::Visibility::Public).count() > 0
            || getAttributeListStatic(Uml::Visibility::Protected).count() > 0
            || getAttributeListStatic(Uml::Visibility::Private).count() > 0;
}

/**
 * Return true if this classifier has accessor methods.
 */
bool UMLClassifier::hasAccessorMethods()
{
    return hasAttributes() || hasAssociations();
}

/**
 * Return true if this classifier has operation methods.
 */
bool UMLClassifier::hasOperationMethods()
{
    return getOpList().count() > 0 ? true : false;
}

/**
 * Return true if this classifier has methods.
 */
bool UMLClassifier::hasMethods()
{
    return hasOperationMethods() || hasAccessorMethods();
}

// this is a bit too simplistic..some associations are for
// SINGLE objects, and WONT be declared as Vectors, so this
// is a bit overly inclusive (I guess that's better than the other way around)

/**
 * Return true if this classifier has vector fields.
 */
bool UMLClassifier::hasVectorFields()
{
    return hasAssociations();
}

/**
 * Return the list of unidirectional association that should show up in the code
 */
UMLAssociationList  UMLClassifier::getUniAssociationToBeImplemented()
{
    UMLAssociationList associations = getSpecificAssocs(AssociationType::UniAssociation);
    UMLAssociationList uniAssocListToBeImplemented;

    foreach (UMLAssociation *a, associations) {
        uIgnoreZeroPointer(a);
        if (a->getObjectId(RoleType::B) == id()) {
            continue;  // we need to be at the A side
        }
        QString roleNameB = a->getRoleName(RoleType::B);
        if (!roleNameB.isEmpty()) {
            UMLAttributeList atl = getAttributeList();
            bool found = false;
            //make sure that an attribute with the same name doesn't already exist
            foreach (UMLAttribute *at, atl) {
                uIgnoreZeroPointer(at);
                if (at->name() == roleNameB) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                uniAssocListToBeImplemented.append(a);
            }
        }
    }
    return uniAssocListToBeImplemented;
}

/**
 * Creates XML tag <UML:Class>, <UML:Interface>, or <UML:DataType>
 * depending on m_BaseType.
 * Saves possible template parameters, generalizations, attributes,
 * operations, and contained objects to the given QDomElement.
 */
void UMLClassifier::saveToXMI1(QXmlStreamWriter& writer)
{
    QString tag;
    switch (m_BaseType) {
        case UMLObject::ot_Class:
            tag = QLatin1String("UML:Class");
            break;
        case UMLObject::ot_Interface:
            tag = QLatin1String("UML:Interface");
            break;
        case UMLObject::ot_Package:
            UMLPackage::saveToXMI1(writer);
            return;
            break;
        default:
            uError() << "internal error: basetype is " << m_BaseType;
            return;
    }
    UMLObject::save1(tag, writer);

    //save templates
    UMLClassifierListItemList list = getFilteredList(UMLObject::ot_Template);
    if (list.count()) {
        writer.writeStartElement(QLatin1String("UML:ModelElement.templateParameter"));
        foreach (UMLClassifierListItem *tmpl, list) {
            tmpl->saveToXMI1(writer);
        }
        writer.writeEndElement();
    }

    //save generalizations (we are the subclass, the other end is the superclass)
    UMLAssociationList generalizations = getSpecificAssocs(AssociationType::Generalization);
    if (generalizations.count()) {
        writer.writeStartElement(QLatin1String("UML:GeneralizableElement.generalization"));
        foreach (UMLAssociation *a, generalizations) {
            // We are the subclass if we are at the role A end.
            if (m_nId != a->getObjectId(RoleType::A)) {
                continue;
            }
            writer.writeStartElement(QLatin1String("UML:Generalization"));
            writer.writeAttribute(QLatin1String("xmi.idref"), Uml::ID::toString(a->id()));
            writer.writeEndElement();
        }
        writer.writeEndElement();
    }

    UMLClassifierListItemList attList = getFilteredList(UMLObject::ot_Attribute);
    UMLOperationList          opList  = getOpList();

    if (attList.count() || opList.count()) {
        writer.writeStartElement(QLatin1String("UML:Classifier.feature"));
        // save attributes
        foreach (UMLClassifierListItem *pAtt, attList) {
            pAtt->saveToXMI1(writer);
        }
        // save operations
        foreach (UMLOperation *pOp, opList) {
            pOp->saveToXMI1(writer);
        }
        writer.writeEndElement();
    }

    // save contained objects
    if (m_objects.count()) {
        writer.writeStartElement(QLatin1String("UML:Namespace.ownedElement"));
        foreach (UMLObject* obj, m_objects) {
            uIgnoreZeroPointer(obj);
            obj->saveToXMI1 (writer);
        }
        writer.writeEndElement();
    }
    UMLObject::save1end(writer);   // from UMLObject::save1(tag)
}

/**
 * Create a new ClassifierListObject (attribute, operation, template)
 * according to the given XMI tag.
 * Returns NULL if the string given does not contain one of the tags
 * <UML:Attribute>, <UML:Operation>, or <UML:TemplateParameter>.
 * Used by the clipboard for paste operation.
 */
UMLClassifierListItem* UMLClassifier::makeChildObject(const QString& xmiTag)
{
    UMLClassifierListItem* pObject = 0;
    if (UMLDoc::tagEq(xmiTag, QLatin1String("Operation")) ||
        UMLDoc::tagEq(xmiTag, QLatin1String("ownedOperation"))) {
        pObject = new UMLOperation(this);
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("Attribute")) ||
               UMLDoc::tagEq(xmiTag, QLatin1String("ownedAttribute"))) {
        if (baseType() != UMLObject::ot_Class)
            return 0;
        pObject = new UMLAttribute(this);
    } else if (UMLDoc::tagEq(xmiTag, QLatin1String("TemplateParameter"))) {
        pObject = new UMLTemplate(this);
    }
    return pObject;
}

/**
 * Auxiliary to loadFromXMI:
 * The loading of operations is implemented here.
 * Calls loadFromXMI1() for any other tag.
 * Child classes can override the loadFromXMI1() method
 * to load its additional tags.
 */
bool UMLClassifier::load1(QDomElement& element)
{
    UMLClassifierListItem *child = 0;
    bool totalSuccess = true;
    for (QDomNode node = element.firstChild(); !node.isNull();
            node = node.nextSibling()) {
        if (node.isComment())
            continue;
        element = node.toElement();
        QString tag = element.tagName();
        QString stereotype = element.attribute(QLatin1String("stereotype"));
        if (UMLDoc::tagEq(tag, QLatin1String("ModelElement.templateParameter")) ||
                UMLDoc::tagEq(tag, QLatin1String("Classifier.feature")) ||
                UMLDoc::tagEq(tag, QLatin1String("Namespace.ownedElement")) ||
                UMLDoc::tagEq(tag, QLatin1String("Element.ownedElement")) ||  // Embarcadero's Describe
                UMLDoc::tagEq(tag, QLatin1String("Namespace.contents"))) {
            load1(element);
            // Not evaluating the return value from load()
            // because we want a best effort.

        } else if ((child = makeChildObject(tag)) != 0) {
            if (child->loadFromXMI1(element)) {
                switch (child->baseType()) {
                    case UMLObject::ot_Template:
                        addTemplate(child->asUMLTemplate());
                        break;
                    case UMLObject::ot_Operation:
                        if (! addOperation(child->asUMLOperation())) {
                            uError() << "error from addOperation(op)";
                            delete child;
                            totalSuccess = false;
                        }
                        break;
                    case UMLObject::ot_Attribute:
                    case UMLObject::ot_InstanceAttribute:
                        addAttribute(child->asUMLAttribute());
                        break;
                    default:
                        break;
                }
            } else {
                uWarning() << "failed to load " << tag;
                delete child;
                totalSuccess = false;
            }
        } else if (!Model_Utils::isCommonXMI1Attribute(tag)) {
            UMLObject *pObject = Object_Factory::makeObjectFromXMI(tag, stereotype);
            if (pObject == 0) {
                // Not setting totalSuccess to false
                // because we want a best effort.
                continue;
            }
            pObject->setUMLPackage(this);
            if (! pObject->loadFromXMI1(element)) {
                removeObject(pObject);
                delete pObject;
                totalSuccess = false;
            }
        }
    }
    return totalSuccess;
}

/*
UMLClassifierList UMLClassifier::getPlainAssocChildClassifierList()
{
    UMLAssociationList plainAssociations = getSpecificAssocs(Uml::AssociationType::Association);
    return findAssocClassifierObjsInRoles(&plainAssociations);
}

UMLClassifierList UMLClassifier::getAggregateChildClassifierList()
{
    UMLAssociationList aggregations = getAggregations();
    return findAssocClassifierObjsInRoles(&aggregations);
}

UMLClassifierList UMLClassifier::getCompositionChildClassifierList()
{
    UMLAssociationList compositions = getCompositions();
    return findAssocClassifierObjsInRoles(&compositions);
}

UMLClassifierList UMLClassifier::findAssocClassifierObjsInRoles (UMLAssociationList * list)
{
    UMLClassifierList classifiers;

    for (UMLAssociationListIt alit(*list); alit.hasNext(); ) {
        UMLAssociation* a = alit.next();
        // DON'T accept a classifier IF the association role is empty, by
        // convention, that means to ignore the classifier on that end of
        // the association.
        // We also ignore classifiers which are the same as the current one
        // (e.g. id matches), we only want the "other" classifiers
        if (a->getObjectId(RoleType::A) == id() && !a->getRoleName(RoleType::B).isEmpty()) {
            UMLClassifier *c = a->getObject(RoleType::B)->asUMLClassifier();
            if(c)
                classifiers.append(c);
        } else if (a->getObjectId(RoleType::B) == id() && !a->getRoleName(RoleType::A).isEmpty()) {
            UMLClassifier *c = a->getObject(RoleType::A)->asUMLClassifier();
            if(c)
                classifiers.append(c);
        }
    }

    return classifiers;
}
*/


