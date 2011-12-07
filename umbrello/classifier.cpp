/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
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

// kde includes
#include <klocale.h>
#include <kmessagebox.h>

// qt includes
#include <QtCore/QPointer>

using namespace Uml;

/**
 * Sets up a Classifier.
 *
 * @param name   The name of the Concept.
 * @param id     The unique id of the Concept.
 */
UMLClassifier::UMLClassifier(const QString & name, Uml::IDType id)
  : UMLPackage(name, id)
{
    m_BaseType = UMLObject::ot_Class;  // default value
    m_pClassAssoc = NULL;
    m_isRef = false;
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
            UMLObject::setStereotype("interface");
            UMLObject::m_bAbstract = true;
            newIcon = Icon_Utils::it_Interface;
            break;
        case ot_Class:
            UMLObject::setStereotype(QString());
            UMLObject::m_bAbstract = false;
            newIcon = Icon_Utils::it_Class;
            break;
        case ot_Datatype:
            UMLObject::setStereotype("datatype");
            UMLObject::m_bAbstract = false;
            newIcon = Icon_Utils::it_Datatype;
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
 * Returns true if this classifier represents a datatype.
 */
bool UMLClassifier::isDatatype() const
{
    return (m_BaseType == ot_Datatype);
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
    if ( list.count() == 0 ) {
        return NULL;
    }
    const int inputParmCount = opParams.count();

    // there is at least one operation with the same name... compare the parameter list
    foreach (UMLOperation* test, list) {
        if (test == exemptOp) {
            continue;
        }
        UMLAttributeList testParams = test->getParmList( );
        const int pCount = testParams.count();
        if ( pCount != inputParmCount ) {
            continue;
        }
        int i = 0;
        while (i < pCount) {
            // The only criterion for equivalence is the parameter types.
            // (Default values are not considered.)
            if( testParams.at(i)->getTypeName() != opParams.at(i)->getTypeName() )
                break;
            i++;
        }
        if ( i == pCount ) { // all parameters matched->the signature is not unique
            return test;
        }
    }
    // we did not find an exact match, so the signature is unique ( acceptable )
    return NULL;
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
        return NULL;
    }
    // if there are operation(s) with the same name then compare the parameter list
    const int inputParmCount = params.count();

    foreach (UMLOperation* test, list ) {
        UMLAttributeList testParams = test->getParmList();
        const int pCount = testParams.count();
        if (inputParmCount == 0 && pCount == 0)
            return test;
        if (inputParmCount != pCount)
            continue;
        int i = 0;
        for (; i < pCount; ++i) {
            Model_Utils::NameAndType_ListIt nt(params.begin() + i);
            UMLClassifier *type = dynamic_cast<UMLClassifier*>((*nt).m_type);
            UMLClassifier *testType = testParams.at(i)->getType();
            if (type == NULL && testType == NULL) { //no parameter type
                continue;
            } else if (type == NULL) {  //template parameter
                if (testType->name() != "class")
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
 * If no name is provided, or if the params are NULL, an Operation
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
        bool *isExistingOp  /*=NULL*/,
        Model_Utils::NameAndType_List *params  /*=NULL*/)
{
    bool nameNotSet = (name.isNull() || name.isEmpty());
    if (! nameNotSet) {
        Model_Utils::NameAndType_List parList;
        if (params)
            parList = *params;
        UMLOperation* existingOp = findOperation(name, parList);
        if (existingOp != NULL) {
            if (isExistingOp != NULL)
                *isExistingOp = true;
            return existingOp;
        }
    }
    // we did not find an exact match, so the signature is unique
    UMLOperation *op = new UMLOperation(this, name);
    if (params) {
        for (Model_Utils::NameAndType_ListIt it = params->begin(); it != params->end(); ++it ) {
            const Model_Utils::NameAndType &nt = *it;
            UMLAttribute *par = new UMLAttribute(op, nt.m_name, Uml::id_None, Uml::Visibility::Private,
                                                 nt.m_type, nt.m_initialValue);
            par->setParmKind(nt.m_direction);
            op->addParm(par);
        }
    }
    if (nameNotSet || params == NULL) {
        if (nameNotSet)
            op->setName( uniqChildName(UMLObject::ot_Operation) );
        while (true) {
            QPointer<UMLOperationDialog> operationDialog = new UMLOperationDialog(0, op);
            if( operationDialog->exec() != KDialog::Accepted ) {
                delete op;
                delete operationDialog;
                return NULL;
            } else if (checkOperationSignature(op->name(), op->getParmList())) {
                KMessageBox::information(0,
                                         i18n("An operation with the same name and signature already exists. You can not add it again."));
            } else {
                break;
            }
            delete operationDialog;
        }
    }

    // operation name is ok, formally add it to the classifier
    if (! addOperation(op)) {
        delete op;
        return NULL;
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
bool UMLClassifier::addOperation(UMLOperation* op, int position )
{
    Q_ASSERT(op);
    if (m_List.indexOf(op) != -1) {
        uDebug() << "findRef(" << op->name() << ") finds op (bad)";
        return false;
    }
    if (checkOperationSignature(op->name(), op->getParmList()) ) {
        uDebug() << "checkOperationSignature(" << op->name() << ") op is non-unique";
        return false;
    }

    if ( position >= 0 && position <= m_List.count() ) {
        uDebug() << op->name() << ": inserting at position " << position;
        m_List.insert(position, op);
        UMLClassifierListItemList itemList = getFilteredList(UMLObject::ot_Operation);
        QString buf;
        foreach (UMLClassifierListItem* currentAtt, itemList ) {
            buf.append(' ' + currentAtt->name());
        }
        uDebug() << "  list after change: " << buf;
    }
    else {
        m_List.append(op);
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
    if ( addOperation(op, -1) ) {
        return true;
    }
    else if (log) {
        log->removeChangeByNewID( op->id() );
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
    if (op == NULL) {
        uDebug() << "called on NULL op";
        return -1;
    }
    if (!m_List.removeAll(op)) {
        uDebug() << "cannot find op " << op->name() << " in list";
        return -1;
    }
    // disconnection needed.
    // note that we don't delete the operation, just remove it from the Classifier
    disconnect(op, SIGNAL(modified()), this, SIGNAL(modified()));
    emit operationRemoved(op);
    UMLObject::emitModified();
    return m_List.count();
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

    int button = KDialog::Accepted;

    while (button == KDialog::Accepted && !goodName) {
        QPointer<UMLTemplateDialog> templateDialog = new UMLTemplateDialog(0, newTemplate);
        button = templateDialog->exec();
        name = newTemplate->name();

        if (name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        }
        else if ( findChildObject(name) != NULL ) {
            KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
        }
        else {
            goodName = true;
        }
        delete templateDialog;
    }

    if (button != KDialog::Accepted) {
        return NULL;
    }

    addTemplate(newTemplate);

    UMLDoc *umldoc = UMLApp::app()->document();
    umldoc->signalUMLObjectCreated(newTemplate);
    return newTemplate;
}

/**
 * Returns the number of attributes for the class.
 *
 * @return  The number of attributes for the class.
 */
int UMLClassifier::attributes()
{
    UMLClassifierListItemList atts = getFilteredList(UMLObject::ot_Attribute);
    return atts.count();
}

/**
 * Returns the attributes for the specified scope.
 * @return   List of true attributes for the class.
 */
UMLAttributeList UMLClassifier::getAttributeList() const
{
    UMLAttributeList attributeList;
    foreach (UMLObject* listItem , m_List ) {
        uIgnoreZeroPointer(listItem);
        if (listItem->baseType() == UMLObject::ot_Attribute) {
            attributeList.append(static_cast<UMLAttribute*>(listItem));
        }
    }
    return attributeList;
}

/**
 * Returns the attributes for the specified scope.
 * @param scope   The scope of the attribute.
 * @return        List of true attributes for the class.
 */
UMLAttributeList UMLClassifier::getAttributeList(Uml::Visibility scope) const
{
    UMLAttributeList list;
    if (!isInterface())
    {
        UMLAttributeList atl = getAttributeList();
        foreach(UMLAttribute* at, atl )
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
UMLAttributeList UMLClassifier::getAttributeListStatic(Uml::Visibility scope) const
{
    UMLAttributeList list;
    if (!isInterface())
    {
        UMLAttributeList atl = getAttributeList();
        foreach(UMLAttribute* at, atl )
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
    foreach (UMLObject*  obj, m_List) {
        uIgnoreZeroPointer(obj);
        if (obj->baseType() != UMLObject::ot_Operation)
            continue;
        UMLOperation *op = static_cast<UMLOperation*>(obj);
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
UMLObject* UMLClassifier::findChildObjectById(Uml::IDType id, bool considerAncestors /* =false */)
{
    UMLObject *o = UMLCanvasObject::findChildObjectById(id);
    if (o) {
        return o;
    }
    if (considerAncestors) {
        UMLClassifierList ancestors = findSuperClassConcepts();
        foreach (UMLClassifier *anc , ancestors ) {
            UMLObject *o = anc->findChildObjectById(id);
            if (o) {
                return o;
            }
        }
    }
    return NULL;
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
    Uml::IDType myID = id();
    foreach(UMLClassifier *c , list ) {
        uIgnoreZeroPointer(c);
        if (type == ALL || (!c->isInterface() && type == CLASS)
                || (c->isInterface() && type == INTERFACE)) {
            inheritingConcepts.append(c);
        }
    }

    foreach (UMLAssociation *a , rlist ) {
        uIgnoreZeroPointer(a);
        if (a->getObjectId(A) != myID)
        {
            UMLObject* obj = a->getObject(A);
            UMLClassifier *concept = dynamic_cast<UMLClassifier*>(obj);
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
    Uml::IDType myID = id();
    foreach (UMLClassifier *concept , list ) {
        uIgnoreZeroPointer(concept);
        if (type == ALL || (!concept->isInterface() && type == CLASS)
                || (concept->isInterface() && type == INTERFACE))
            parentConcepts.append(concept);
    }

    foreach (UMLAssociation *a , rlist ) {
        if (a->getObjectId(A) == myID)
        {
            UMLObject* obj = a->getObject(B);
            UMLClassifier *concept = dynamic_cast<UMLClassifier*>(obj);
            if (concept && (type == ALL || (!concept->isInterface() && type == CLASS)
                            || (concept->isInterface() && type == INTERFACE))
                        && (parentConcepts.indexOf(concept) == -1))
                parentConcepts.append(concept);
        }
    }

    return parentConcepts;
}

/**
 * Overloaded '==' operator.
 */
bool UMLClassifier::operator==(const UMLClassifier & rhs) const
{
  /*
    if ( m_List.count() != rhs.m_List.count() ) {
        return false;
    }
    if ( &m_List != &(rhs.m_List) ) {
        return false;
    }
   */
    return UMLCanvasObject::operator==(rhs);
}

/**
 * Copy the internal presentation of this object into the new
 * object.
 */
void UMLClassifier::copyInto(UMLObject *lhs) const
{
    UMLClassifier *target = static_cast<UMLClassifier*>(lhs);
    UMLCanvasObject::copyInto(target);
    target->setBaseType(m_BaseType);
    // CHECK: association property m_pClassAssoc is not copied
    m_List.copyInto(&(target->m_List));
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
    foreach (UMLObject* obj, m_List ) {
        uIgnoreZeroPointer(obj);
        /**** For reference, here is the non-reentrant iteration scheme -
              DO NOT USE THIS !
        for (UMLObject *obj = m_List.first(); obj; obj = m_List.next())
         {  ....  }
         ****/
        if (obj->resolveRef()) {
            UMLClassifierListItem *cli = static_cast<UMLClassifierListItem*>(obj);
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
bool UMLClassifier::acceptAssociationType(Uml::AssociationType type)
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
                                             Uml::Visibility vis,
                                             const QString &init)
{
    Uml::IDType id = UniqueID::gen();
    QString currentName;
    if (name.isNull())  {
        currentName = uniqChildName(UMLObject::ot_Attribute);
    } else {
        currentName = name;
    }
    UMLAttribute* newAttribute = new UMLAttribute(this, currentName, id, vis, type, init);

    int button = KDialog::Accepted;
    bool goodName = false;

    //check for name.isNull() stops dialog being shown
    //when creating attribute via list view
    while (button == KDialog::Accepted && !goodName && name.isNull()) {
        QPointer<UMLAttributeDialog> attributeDialog = new UMLAttributeDialog(0, newAttribute);
        button = attributeDialog->exec();
        QString name = newAttribute->name();

        if(name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        } else if ( findChildObject(name) != NULL ) {
            KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
        } else {
            goodName = true;
        }
        delete attributeDialog;
    }

    if (button != KDialog::Accepted) {
        delete newAttribute;
        return NULL;
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

UMLAttribute* UMLClassifier::addAttribute(const QString &name, Uml::IDType id /* = Uml::id_None */)
{
    foreach (UMLObject* obj, m_List ) {
        uIgnoreZeroPointer(obj);
        if (obj->baseType() == UMLObject::ot_Attribute && obj->name() == name)
            return static_cast<UMLAttribute*>(obj);
    }
    Uml::Visibility scope = Settings::optionState().classState.defaultAttributeScope;
    UMLAttribute *a = new UMLAttribute(this, name, id, scope);
    m_List.append(a);
    emit attributeAdded(a);
    UMLObject::emitModified();
    connect(a,SIGNAL(modified()),this,SIGNAL(modified()));
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
UMLAttribute* UMLClassifier::addAttribute(const QString &name, UMLObject *type, Uml::Visibility scope)
{
    UMLAttribute *a = new UMLAttribute(this);
    a->setName(name);
    a->setVisibility(scope);
    a->setID(UniqueID::gen());
    if (type) {
        a->setType(type);
    }
    m_List.append(a);
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
    if (findChildObject(att->name()) == NULL) {
        att->setParent(this);
        if (position >= 0 && position < (int)m_List.count()) {
            m_List.insert(position, att);
        }
        else {
            m_List.append(att);
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
    if (!m_List.removeAll(att)) {
        uDebug() << "cannot find att given in list";
        return -1;
    }
    emit attributeRemoved(att);
    UMLObject::emitModified();
    // If we are deleting the object, then we don't need to disconnect..this is done auto-magically
    // for us by QObject. -b.t.
    // disconnect(att, SIGNAL(modified()), this,SIGNAL(modified()));
    delete att;
    return m_List.count();
}

/**
 * Sets the UMLAssociation for which this class shall act as an
 * association class.
 */
void UMLClassifier::setClassAssoc(UMLAssociation *assoc)
{
    m_pClassAssoc = assoc;
}

/**
 * Returns the UMLAssociation for which this class acts as an
 * association class. Returns NULL if this class does not act
 * as an association class.
 */
UMLAssociation *UMLClassifier::getClassAssoc() const
{
    return m_pClassAssoc;
}

/**
 * Return true if this classifier has abstract operations.
 */
bool UMLClassifier::hasAbstractOps ()
{
    UMLOperationList opl( getOpList() );
    foreach(UMLOperation *op , opl ) {
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
 * @return   The list of operations for the Classifier.
 */
UMLOperationList UMLClassifier::getOpList(bool includeInherited)
{
    UMLOperationList ops;
    foreach (UMLObject* li, m_List) {
        uIgnoreZeroPointer(li);
        if (li->baseType() == ot_Operation) {
            ops.append(static_cast<UMLOperation*>(li));
        }
    }
    if (includeInherited) {
        UMLClassifierList parents = findSuperClassConcepts();
        foreach (UMLClassifier* c ,  parents) {
            if (c == this) {
                uError() << "class " << c->name() << " is parent of itself ?!?";
                continue;
            }
            // get operations for each parent by recursive call
            UMLOperationList pops = c->getOpList(true);
            // add these operations to operation list, but only if unique.
            foreach (UMLOperation *po , pops ) {
                QString po_as_string(po->toString(Uml::SignatureType::SigNoVis));
                bool breakFlag = false;
                foreach (UMLOperation* o ,  ops ) {
                    if (o->toString(Uml::SignatureType::SigNoVis) == po_as_string) {
                        breakFlag = true;
                        break;
                    }
                }
                if (breakFlag == false)
                    ops.append(po);
            }
        }
    }
    return ops;
}

/**
 * Returns the entries in m_List that are of the requested type.
 * If the requested type is UMLObject::ot_UMLObject then all entries
 * are returned.
 * @param ot   the requested object type
 * @return     The list of true operations for the Concept.
 */
UMLClassifierListItemList UMLClassifier::getFilteredList(UMLObject::ObjectType ot) const
{
    UMLClassifierListItemList resultList;
    foreach (UMLObject* o, m_List) {
        uIgnoreZeroPointer(o);
        if (!o || o->baseType() == UMLObject::ot_Association) {
            continue;
        }
        UMLClassifierListItem *listItem = static_cast<UMLClassifierListItem*>(o);
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
UMLTemplate* UMLClassifier::addTemplate(const QString &name, Uml::IDType id)
{
    UMLTemplate *templt = findTemplate(name);
    if (templt) {
        return templt;
    }
    templt = new UMLTemplate(this, name, id);
    m_List.append(templt);
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
    if (findChildObject(name) == NULL) {
        newTemplate->setParent(this);
        m_List.append(newTemplate);
        emit templateAdded(newTemplate);
        UMLObject::emitModified();
        connect(newTemplate,SIGNAL(modified()),this,SIGNAL(modified()));
        return true;
    }
    else if (log) {
        log->removeChangeByNewID( newTemplate->id() );
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
    QString name = templt->name();
    if (findChildObject(name) == NULL) {
        templt->setParent(this);
        if ( position >= 0 && position <= (int)m_List.count() ) {
            m_List.insert(position, templt);
        }
        else {
            m_List.append(templt);
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
    if ( !m_List.removeAll(umltemplate) ) {
        uWarning() << "cannot find att given in list";
        return -1;
    }
    emit templateRemoved(umltemplate);
    UMLObject::emitModified();
    disconnect(umltemplate, SIGNAL(modified()), this, SIGNAL(modified()));
    return m_List.count();
}

/**
 * Seeks the template parameter of the given name.
 * @param name   the template name
 * @return       the found template or 0
 */
UMLTemplate *UMLClassifier::findTemplate(const QString& name)
{
    UMLTemplateList templParams = getTemplateList();
    foreach (UMLTemplate *templt , templParams) {
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
    foreach (UMLObject* listItem, m_List) {
        uIgnoreZeroPointer(listItem);
        if (listItem->baseType() == UMLObject::ot_Template) {
            templateList.append(static_cast<UMLTemplate*>(listItem));
        }
    }
    return templateList;
}

/**
 * Take and return a subordinate item from this classifier.
 * Ownership of the item is passed to the caller.
 *
 * @param item   Subordinate item to take.
 * @return       Index in m_List of the item taken.
 *               Return -1 if the item is not found in m_List.
 */
int UMLClassifier::takeItem(UMLClassifierListItem *item)
{
    QString buf;
    foreach (UMLObject* currentAtt, m_List ) {
        uIgnoreZeroPointer(currentAtt);
        QString txt = currentAtt->name();
        if (txt.isEmpty()) {
           txt = "Type-" + QString::number((int) currentAtt->baseType());
        }
        buf.append(' ' + currentAtt->name());
    }
    uDebug() << "  UMLClassifier::takeItem (before): m_List is " << buf;

    int index = m_List.indexOf(item);
    if (index == -1) {
        return -1;
    }
    switch (item->baseType()) {
        case UMLObject::ot_Operation: {
            if (removeOperation(dynamic_cast<UMLOperation*>(item)) < 0) {
                index = -1;
            }
            break;
        }
        case UMLObject::ot_Attribute: {
            UMLAttribute *retval = dynamic_cast<UMLAttribute*>(m_List.takeAt( index ));
            if (retval) {
                emit attributeRemoved(retval);
                UMLObject::emitModified();
            } else {
                index = -1;
            }
            break;
        }
        case UMLObject::ot_Template: {
            UMLTemplate *templt = dynamic_cast<UMLTemplate*>(m_List.takeAt( index));
            if (templt) {
                emit templateRemoved(templt);
                UMLObject::emitModified();
            } else {
                index = -1;
            }
            break;
        }
        case UMLObject::ot_EnumLiteral: {
            UMLEnumLiteral *el = dynamic_cast<UMLEnumLiteral*>(m_List.takeAt( index ) );
            if (el) {
                UMLEnum *e = static_cast<UMLEnum*>(this);
                e->signalEnumLiteralRemoved(el);
                UMLObject::emitModified();
            } else {
                index = -1;
            }
            break;
        }
        case UMLObject::ot_EntityAttribute: {
            UMLEntityAttribute* el = dynamic_cast<UMLEntityAttribute*>(m_List.takeAt( index));
            if (el) {
                UMLEntity *e = static_cast<UMLEntity*>(this);
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
 * Set the origin type (in case of e.g. typedef)
 * @param origType   the origin type to set
 */
void UMLClassifier::setOriginType(UMLClassifier *origType)
{
    m_pSecondary = origType;
}

/**
 * Get the origin type (in case of e.g. typedef)
 * @return   the origin type
 */
UMLClassifier * UMLClassifier::originType() const
{
    return static_cast<UMLClassifier*>(m_pSecondary);
}

/**
 * Set the m_isRef flag (true when dealing with a pointer type)
 * @param isRef   the flag to set
 */
void UMLClassifier::setIsReference(bool isRef)
{
    m_isRef = isRef;
}

/**
 * Get the m_isRef flag.
 * @return   true if is reference, otherwise false
 */
bool UMLClassifier::isReference() const
{
    return m_isRef;
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
    return getOpList().last() ? true : false;
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
        if (a->getObjectId(Uml::B) == id()) {
            continue;  // we need to be at the A side
        }
        QString roleNameB = a->getRoleName(Uml::B);
        if (!roleNameB.isEmpty()) {
            UMLAttributeList atl = getAttributeList();
            bool found = false;
            //make sure that an attribute with the same name doesn't already exist
            foreach (UMLAttribute *at , atl ) {
                uIgnoreZeroPointer(a);
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
 * Auxiliary to saveToXMI of inheriting classes:
 * Saves template parameters to the given QDomElement.
 */
void UMLClassifier::saveToXMI(QDomDocument & qDoc, QDomElement & qElement)
{
    QString tag;
    switch (m_BaseType) {
        case UMLObject::ot_Class:
            tag = "UML:Class";
            break;
        case UMLObject::ot_Interface:
            tag = "UML:Interface";
            break;
        case UMLObject::ot_Datatype:
            tag = "UML:DataType";
            break;
        default:
            uError() << "internal error: basetype is " << m_BaseType;
            return;
    }
    QDomElement classifierElement = UMLObject::save(tag, qDoc);
    if (m_BaseType == UMLObject::ot_Datatype && m_pSecondary != NULL)
        classifierElement.setAttribute( "elementReference",
                                        ID2STR(m_pSecondary->id()) );

    //save templates
    UMLClassifierListItemList list = getFilteredList(UMLObject::ot_Template);
    if (list.count()) {
        QDomElement tmplElement = qDoc.createElement( "UML:ModelElement.templateParameter" );
        foreach (UMLClassifierListItem *tmpl , list ) {
            tmpl->saveToXMI(qDoc, tmplElement);
        }
        classifierElement.appendChild( tmplElement );
    }

    //save generalizations (we are the subclass, the other end is the superclass)
    UMLAssociationList generalizations = getSpecificAssocs(AssociationType::Generalization);
    if (generalizations.count()) {
        QDomElement genElement = qDoc.createElement("UML:GeneralizableElement.generalization");
        foreach (UMLAssociation *a , generalizations ) {
            // We are the subclass if we are at the role A end.
            if (m_nId != a->getObjectId(Uml::A)) {
                continue;
            }
            QDomElement gElem = qDoc.createElement("UML:Generalization");
            gElem.setAttribute( "xmi.idref", ID2STR(a->id()) );
            genElement.appendChild(gElem);
        }
        if (genElement.hasChildNodes()) {
            classifierElement.appendChild( genElement );
        }
    }

    // save attributes
    QDomElement featureElement = qDoc.createElement( "UML:Classifier.feature" );
    UMLClassifierListItemList attList = getFilteredList(UMLObject::ot_Attribute);
    foreach (UMLClassifierListItem *pAtt , attList ) {
        pAtt->saveToXMI( qDoc, featureElement );
    }

    // save operations
    UMLOperationList opList = getOpList();
    foreach (UMLOperation *pOp , opList ) {
        pOp->saveToXMI( qDoc, featureElement );
    }
    if (featureElement.hasChildNodes()) {
        classifierElement.appendChild( featureElement );
    }

    // save contained objects
    if (m_objects.count()) {
        QDomElement ownedElement = qDoc.createElement( "UML:Namespace.ownedElement" );
        foreach (UMLObject* obj, m_objects ) {
            obj->saveToXMI (qDoc, ownedElement);
        }
        classifierElement.appendChild( ownedElement );
    }
    qElement.appendChild( classifierElement );
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
    UMLClassifierListItem* pObject = NULL;
    if (UMLDoc::tagEq(xmiTag, "Operation")) {
        pObject = new UMLOperation(this);
    } else if (UMLDoc::tagEq(xmiTag, "Attribute")) {
        if (baseType() != UMLObject::ot_Class)
            return NULL;
        pObject = new UMLAttribute(this);
    } else if (UMLDoc::tagEq(xmiTag, "TemplateParameter")) {
        pObject = new UMLTemplate(this);
    }
    return pObject;
}

/**
 * Auxiliary to loadFromXMI:
 * The loading of operations is implemented here.
 * Calls loadSpecialized() for any other tag.
 * Child classes can override the loadSpecialized method
 * to load its additional tags.
 */
bool UMLClassifier::load(QDomElement& element)
{
    UMLClassifierListItem *child = NULL;
    m_SecondaryId = element.attribute( "elementReference", "" );
    if (!m_SecondaryId.isEmpty()) {
        // @todo We do not currently support composition.
        m_isRef = true;
    }
    bool totalSuccess = true;
    for (QDomNode node = element.firstChild(); !node.isNull();
            node = node.nextSibling()) {
        if (node.isComment())
            continue;
        element = node.toElement();
        QString tag = element.tagName();
        if (UMLDoc::tagEq(tag, "ModelElement.templateParameter") ||
                UMLDoc::tagEq(tag, "Classifier.feature") ||
                UMLDoc::tagEq(tag, "Namespace.ownedElement") ||
                UMLDoc::tagEq(tag, "Namespace.contents")) {
            load(element);
            // Not evaluating the return value from load()
            // because we want a best effort.

        } else if ((child = makeChildObject(tag)) != NULL) {
            if (child->loadFromXMI(element)) {
                switch (child->baseType()) {
                    case UMLObject::ot_Template:
                        addTemplate( static_cast<UMLTemplate*>(child) );
                        break;
                    case UMLObject::ot_Operation:
                        if (! addOperation(static_cast<UMLOperation*>(child)) ) {
                            uError() << "error from addOperation(op)";
                            delete child;
                            totalSuccess = false;
                        }
                        break;
                    case UMLObject::ot_Attribute:
                        addAttribute( static_cast<UMLAttribute*>(child) );
                        break;
                    default:
                        break;
                }
            } else {
                uWarning() << "failed to load " << tag;
                delete child;
                totalSuccess = false;
            }
        } else if (!Model_Utils::isCommonXMIAttribute(tag)) {
            UMLObject *pObject = Object_Factory::makeObjectFromXMI(tag);
            if (pObject == NULL) {
                // Not setting totalSuccess to false
                // because we want a best effort.
                continue;
            }
            pObject->setUMLPackage(this);
            if (! pObject->loadFromXMI(element)) {
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
        if (a->getObjectId(Uml::A) == id() && !a->getRoleName(Uml::B).isEmpty()) {
            UMLClassifier *c = dynamic_cast<UMLClassifier*>(a->getObject(Uml::B));
            if(c)
                classifiers.append(c);
        } else if (a->getObjectId(Uml::B) == id() && !a->getRoleName(Uml::A).isEmpty()) {
            UMLClassifier *c = dynamic_cast<UMLClassifier*>(a->getObject(Uml::A));
            if(c)
                classifiers.append(c);
        }
    }

    return classifiers;
}
*/


#include "classifier.moc"
