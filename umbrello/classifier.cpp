/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
// own header
#include "classifier.h"
// qt/kde includes
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
// app includes
#include "association.h"
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
#include "umllistview.h"
#include "uniqueid.h"
#include "object_factory.h"
#include "model_utils.h"
#include "clipboard/idchangelog.h"
#include "dialogs/umloperationdialog.h"
#include "dialogs/umlattributedialog.h"
#include "dialogs/umltemplatedialog.h"
#include "optionstate.h"

using namespace Uml;

UMLClassifier::UMLClassifier(const QString & name, Uml::IDType id)
        : UMLPackage(name, id)
{
    init();
}

UMLClassifier::~UMLClassifier() {
}

void UMLClassifier::init() {
    m_BaseType = Uml::ot_Class;  // default value
    m_pClassAssoc = NULL;
    m_isRef = false;
}

void UMLClassifier::setBaseType(Uml::Object_Type ot) {
    m_BaseType = ot;
    Uml::Icon_Type newIcon;
    switch (ot) {
        case ot_Interface:
            UMLObject::setStereotype("interface");
            UMLObject::m_bAbstract = true;
            newIcon = Uml::it_Interface;
            break;
        case ot_Class:
            UMLObject::setStereotype(QString());
            UMLObject::m_bAbstract = false;
            newIcon = Uml::it_Class;
            break;
        case ot_Datatype:
            UMLObject::setStereotype("datatype");
            UMLObject::m_bAbstract = false;
            newIcon = Uml::it_Datatype;
            break;
        default:
            kError() << "UMLClassifier::setBaseType: cannot set to type "
                << ot << endl;
            return;
    }
    // @todo get rid of direct dependencies to UMLListView
    //  (e.g. move utility methods to Model_Utils and/or use signals)
    UMLListView *listView = UMLApp::app()->getListView();
    listView->changeIconOf(this, newIcon);
}

bool UMLClassifier::isInterface() const {
    return (m_BaseType == ot_Interface);
}

bool UMLClassifier::isDatatype() const {
    return (m_BaseType == ot_Datatype);
}

UMLOperation * UMLClassifier::checkOperationSignature(
        const QString& name,
        UMLAttributeList opParams,
        UMLOperation *exemptOp)
{
    UMLOperationList list = findOperations(name);
    if( list.count() == 0 )
        return NULL;
    const int inputParmCount = opParams.count();

    // there is at least one operation with the same name... compare the parameter list
    for (UMLOperationListIt oit(list); oit.current(); ++oit)
    {
        UMLOperation* test = oit.current();
        if (test == exemptOp)
            continue;
        UMLAttributeList testParams = test->getParmList( );
        const int pCount = testParams.count();
        if( pCount != inputParmCount )
            continue;
        int i = 0;
        while (i < pCount) {
            // The only criterion for equivalence is the parameter types.
            // (Default values are not considered.)
            if( testParams.at(i)->getTypeName() != opParams.at(i)->getTypeName() )
                break;
            i++;
        }
        if( i == pCount )
        {//all parameters matched -> the signature is not unique
            return test;
        }
    }
    // we did not find an exact match, so the signature is unique ( acceptable )
    return NULL;
}

UMLOperation* UMLClassifier::findOperation(const QString& name,
                                           Model_Utils::NameAndType_List params) {
    UMLOperationList list = findOperations(name);
    if (list.count() == 0)
        return NULL;
    // If there are operation(s) with the same name then compare the parameter list
    const int inputParmCount = params.count();
    UMLOperation* test = NULL;
    for (UMLOperationListIt oit(list); (test = oit.current()) != NULL; ++oit) {
        UMLAttributeList testParams = test->getParmList();
        const int pCount = testParams.count();
        if (inputParmCount == 0 && pCount == 0)
            break;
        if (inputParmCount != pCount)
            continue;
        int i = 0;
        for (; i < pCount; ++i) {
            Model_Utils::NameAndType_ListIt nt(params.at(i));
            UMLClassifier *c = dynamic_cast<UMLClassifier*>((*nt).m_type);
            UMLClassifier *testType = testParams.at(i)->getType();
            if (c == NULL) {       //template parameter
                if (testType->getName() != "class")
                    break;
            } else if (c != testType)
                break;
        }
        if (i == pCount)
            break;  // all parameters matched
    }
    return test;
}

UMLOperation* UMLClassifier::createOperation(const QString &name /*=null*/,
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
            op->setName( uniqChildName(Uml::ot_Operation) );
        do {
            UMLOperationDialog operationDialogue(0, op);
            if( operationDialogue.exec() != QDialog::Accepted ) {
                delete op;
                return NULL;
            } else if (checkOperationSignature(op->getName(), op->getParmList())) {
                KMessageBox::information(0,
                                         i18n("An operation with the same name and signature already exists. You can not add it again."));
            } else {
                break;
            }
        } while(1);
    }

    // operation name is ok, formally add it to the classifier
    if (! addOperation(op)) {
        delete op;
        return NULL;
    }

    UMLDoc *umldoc = UMLApp::app()->getDocument();
    umldoc->signalUMLObjectCreated(op);
    return op;
}

bool UMLClassifier::addOperation(UMLOperation* op, int position )
{
    if (m_List.findRef(op) != -1) {
        kDebug() << "UMLClassifier::addOperation: findRef("
        << op->getName() << ") finds op (bad)"
        << endl;
        return false;
    }
    if (checkOperationSignature(op->getName(), op->getParmList()) ) {
        kDebug() << "UMLClassifier::addOperation: checkOperationSignature("
        << op->getName() << ") op is non-unique" << endl;
        return false;
    }

    if( position >= 0 && position <= (int)m_List.count() ) {
        kDebug() << "UMLClassifier::addOperation(" << op->getName()
            << "): inserting at position " << position << endl;
        m_List.insert(position,op);
        UMLClassifierListItemList itemList = getFilteredList(Uml::ot_Operation);
        UMLClassifierListItem* currentAtt;
        QString buf;
        for (UMLClassifierListItemListIt it0(itemList); (currentAtt = it0.current()); ++it0)
            buf.append(' ' + currentAtt->getName());
        kDebug() << "  UMLClassifier::addOperation list after change: " << buf << endl;
     } else
        m_List.append( op );
    emit operationAdded(op);
    UMLObject::emitModified();
    connect(op,SIGNAL(modified()),this,SIGNAL(modified()));
    return true;
}

bool UMLClassifier::addOperation(UMLOperation* Op, IDChangeLog* Log) {
    if( addOperation( Op, -1 ) )
        return true;
    else if( Log ) {
        Log->removeChangeByNewID( Op -> getID() );
    }
    return false;
}

int UMLClassifier::removeOperation(UMLOperation *op) {
    if (op == NULL) {
        kDebug() << "UMLClassifier::removeOperation called on NULL op"
        << endl;
        return -1;
    }
    if(!m_List.remove(op)) {
        kDebug() << "UMLClassifier::removeOperation: can't find op "
        << op->getName() << " in list" << endl;
        return -1;
    }
    // disconnection needed.
    // note that we don't delete the operation, just remove it from the Classifier
    disconnect(op,SIGNAL(modified()),this,SIGNAL(modified()));
    emit operationRemoved(op);
    UMLObject::emitModified();
    return m_List.count();
}

UMLObject* UMLClassifier::createTemplate(const QString& currentName /*= QString()*/) {
    QString name = currentName;
    bool goodName = !name.isEmpty();
    if (!goodName)
        name = uniqChildName(Uml::ot_Template);
    UMLTemplate* newTemplate = new UMLTemplate(this, name);

    int button = QDialog::Accepted;

    while (button==QDialog::Accepted && !goodName) {
        UMLTemplateDialog templateDialogue(0, newTemplate);
        button = templateDialogue.exec();
        name = newTemplate->getName();

        if(name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        } else if ( findChildObject(name) != NULL ) {
            KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
        } else {
            goodName = true;
        }
    }

    if (button != QDialog::Accepted) {
        return NULL;
    }

    addTemplate(newTemplate);

    UMLDoc *umldoc = UMLApp::app()->getDocument();
    umldoc->signalUMLObjectCreated(newTemplate);
    return newTemplate;
}

int UMLClassifier::attributes() {
    UMLClassifierListItemList atts = getFilteredList(Uml::ot_Attribute);
    return atts.count();
}

UMLAttributeList UMLClassifier::getAttributeList() const{
    UMLAttributeList attributeList;
    for (UMLObjectListIt lit(m_List); lit.current(); ++lit) {
        UMLObject *listItem = lit.current();
        if (listItem->getBaseType() == Uml::ot_Attribute) {
            attributeList.append(static_cast<UMLAttribute*>(listItem));
        }
    }
    return attributeList;
}

UMLOperationList UMLClassifier::findOperations(const QString &n) {
    const bool caseSensitive = UMLApp::app()->activeLanguageIsCaseSensitive();
    UMLOperationList list;
    for (UMLObjectListIt lit(m_List); lit.current(); ++lit) {
        UMLObject* obj = lit.current();
        if (obj->getBaseType() != Uml::ot_Operation)
            continue;
        UMLOperation *op = static_cast<UMLOperation*>(obj);
        if (caseSensitive) {
            if (obj->getName() == n)
                list.append(op);
        } else if (obj->getName().lower() == n.lower()) {
            list.append(op);
        }
    }
    return list;
}

UMLObject* UMLClassifier::findChildObjectById(Uml::IDType id, bool considerAncestors /* =false */) {
    UMLObject *o = UMLCanvasObject::findChildObjectById(id);
    if (o)
        return o;
    if (considerAncestors) {
        UMLClassifierList ancestors = findSuperClassConcepts();
        for (UMLClassifier *anc = ancestors.first(); anc; anc = ancestors.next()) {
            UMLObject *o = anc->findChildObjectById(id);
            if (o)
                return o;
        }
    }
    return NULL;
}

UMLClassifierList UMLClassifier::findSubClassConcepts (ClassifierType type) {
    UMLClassifierList list = getSubClasses();
    UMLAssociationList rlist = getRealizations();

    UMLClassifierList inheritingConcepts;
    Uml::IDType myID = getID();
    for (UMLClassifier *c = list.first(); c; c = list.next())
    {
        if (type == ALL || (!c->isInterface() && type == CLASS)
                || (c->isInterface() && type == INTERFACE))
            inheritingConcepts.append(c);
    }

    for (UMLAssociation *a = rlist.first(); a; a = rlist.next())
    {
        if (a->getObjectId(A) != myID)
        {
            UMLObject* obj = a->getObject(A);
            UMLClassifier *concept = dynamic_cast<UMLClassifier*>(obj);
            if (concept && (type == ALL || (!concept->isInterface() && type == CLASS)
                            || (concept->isInterface() && type == INTERFACE))
                        && (inheritingConcepts.findRef(concept) == -1))
                inheritingConcepts.append(concept);
        }
    }

    return inheritingConcepts;
}

UMLClassifierList UMLClassifier::findSuperClassConcepts (ClassifierType type) {
    UMLClassifierList list = getSuperClasses();
    UMLAssociationList rlist = getRealizations();

    UMLClassifierList parentConcepts;
    Uml::IDType myID = getID();
    for (UMLClassifier *concept = list.first(); concept; concept = list.next())
    {
        if (type == ALL || (!concept->isInterface() && type == CLASS)
                || (concept->isInterface() && type == INTERFACE))
            parentConcepts.append(concept);
    }

    for (UMLAssociation *a = rlist.first(); a; a = rlist.next())
    {
        if (a->getObjectId(A) == myID)
        {
            UMLObject* obj = a->getObject(B);
            UMLClassifier *concept = dynamic_cast<UMLClassifier*>(obj);
            if (concept && (type == ALL || (!concept->isInterface() && type == CLASS)
                            || (concept->isInterface() && type == INTERFACE))
                        && (parentConcepts.findRef(concept) == -1))
                parentConcepts.append(concept);
        }
    }

    return parentConcepts;
}

bool UMLClassifier::operator==( UMLClassifier & rhs ) {
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


void UMLClassifier::copyInto(UMLClassifier *rhs) const
{
    UMLCanvasObject::copyInto(rhs);
    rhs->setBaseType(m_BaseType);
    // CHECK: association property m_pClassAssoc is not copied
    m_List.copyInto(&(rhs->m_List));
}

UMLObject* UMLClassifier::clone() const {
    UMLClassifier *clone = new UMLClassifier();
    copyInto(clone);
    return clone;
}

bool UMLClassifier::resolveRef() {
    bool success = UMLPackage::resolveRef();
    // Using reentrant iteration is a bare necessity here:
    for (UMLObjectListIt oit(m_List); oit.current(); ++oit) {
        UMLObject* obj = oit.current();
        /**** For reference, here is the non-reentrant iteration scheme -
              DO NOT USE THIS !
        for (UMLObject *obj = m_List.first(); obj; obj = m_List.next())
         {  ....  }
         ****/
        if (obj->resolveRef()) {
            UMLClassifierListItem *cli = static_cast<UMLClassifierListItem*>(obj);
            switch (cli->getBaseType()) {
                case Uml::ot_Attribute:
                    emit attributeAdded(cli);
                    break;
                case Uml::ot_Operation:
                    emit operationAdded(cli);
                    break;
                case Uml::ot_Template:
                    emit templateAdded(cli);
                    break;
                default:
                    break;
            }
        }
    }
    return success;
}

bool UMLClassifier::acceptAssociationType(Uml::Association_Type type)
{
    switch(type)
    {
    case at_Generalization:
    case at_Aggregation:
    case at_Relationship:
    case at_Dependency:
    case at_Association:
    case at_Association_Self:
    case at_Containment:
    case at_Composition:
    case at_Realization:
    case at_UniAssociation:
        return true;
    default:
        return false;
    }
    return false; //shutup compiler warning
}

UMLAttribute* UMLClassifier::createAttribute(const QString &name,
                                             UMLObject *type,
                                             Uml::Visibility vis,
                                             const QString &init) {
    Uml::IDType id = UniqueID::gen();
    QString currentName;
    if (name.isNull())  {
        currentName = uniqChildName(Uml::ot_Attribute);
    } else {
        currentName = name;
    }
    UMLAttribute* newAttribute = new UMLAttribute(this, currentName, id, vis, type, init);

    int button = QDialog::Accepted;
    bool goodName = false;

    //check for name.isNull() stops dialog being shown
    //when creating attribute via list view
    while (button == QDialog::Accepted && !goodName && name.isNull()) {
        UMLAttributeDialog attributeDialogue(0, newAttribute);
        button = attributeDialogue.exec();
        QString name = newAttribute->getName();

        if(name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        } else if ( findChildObject(name) != NULL ) {
            KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
        } else {
            goodName = true;
        }
    }

    if (button != QDialog::Accepted) {
        delete newAttribute;
        return NULL;
    }

    addAttribute(newAttribute);

    UMLDoc *umldoc = UMLApp::app()->getDocument();
    umldoc->signalUMLObjectCreated(newAttribute);
    return newAttribute;
}

UMLAttribute* UMLClassifier::addAttribute(const QString &name, Uml::IDType id /* = Uml::id_None */) {
    for (UMLObjectListIt lit(m_List); lit.current(); ++lit) {
        UMLObject *obj = lit.current();
        if (obj->getBaseType() == Uml::ot_Attribute && obj->getName() == name)
            return static_cast<UMLAttribute*>(obj);
    }
    Uml::Visibility scope = Settings::getOptionState().classState.defaultAttributeScope;
    UMLAttribute *a = new UMLAttribute(this, name, id, scope);
    m_List.append(a);
    emit attributeAdded(a);
    UMLObject::emitModified();
    connect(a,SIGNAL(modified()),this,SIGNAL(modified()));
    return a;
}

UMLAttribute* UMLClassifier::addAttribute(const QString &name, UMLObject *type, Uml::Visibility scope) {
    UMLAttribute *a = new UMLAttribute(this);
    a->setName(name);
    a->setVisibility(scope);
    a->setID(UniqueID::gen());
    if (type)
        a->setType(type);
    m_List.append(a);
    emit attributeAdded(a);
    UMLObject::emitModified();
    connect(a,SIGNAL(modified()),this,SIGNAL(modified()));
    return a;
}

bool UMLClassifier::addAttribute(UMLAttribute* att, IDChangeLog* Log /* = 0 */,
                                 int position /* = -1 */) {
    if (findChildObject(att->getName()) == NULL) {
        att->parent()->removeChild( att );
        this->insertChild( att );
        if (position >= 0 && position < (int)m_List.count())
            m_List.insert(position, att);
        else
            m_List.append(att);
        emit attributeAdded(att);
        UMLObject::emitModified();
        connect(att, SIGNAL(modified()), this, SIGNAL(modified()));
        return true;
    } else if (Log) {
        Log->removeChangeByNewID(att->getID());
        delete att;
    }
    return false;
}

int UMLClassifier::removeAttribute(UMLAttribute* a) {
    if (!m_List.remove(a)) {
        kDebug() << "can't find att given in list" << endl;
        return -1;
    }
    emit attributeRemoved(a);
    UMLObject::emitModified();
    // If we are deleting the object, then we don't need to disconnect..this is done auto-magically
    // for us by QObject. -b.t.
    // disconnect(a,SIGNAL(modified()),this,SIGNAL(modified()));
    delete a;
    return m_List.count();
}


void UMLClassifier::setClassAssoc(UMLAssociation *assoc) {
    m_pClassAssoc = assoc;
}

UMLAssociation *UMLClassifier::getClassAssoc() const{
    return m_pClassAssoc;
}

bool UMLClassifier::hasAbstractOps () {
    UMLOperationList opl( getOpList() );
    for(UMLOperation *op = opl.first(); op ; op = opl.next())
        if(op->getAbstract())
            return true;
    return false;
}

int UMLClassifier::operations() {
    return getOpList().count();
}

UMLOperationList UMLClassifier::getOpList(bool includeInherited) {
    UMLOperationList ops;
    for (UMLObjectListIt lit(m_List); lit.current(); ++lit) {
        UMLObject *li = lit.current();
        if (li->getBaseType() == ot_Operation)
            ops.append(static_cast<UMLOperation*>(li));
    }
    if (includeInherited) {
        UMLClassifierList parents = findSuperClassConcepts();
        UMLClassifier *c;
        for (UMLClassifierListIt pit(parents); (c = pit.current()) != NULL; ++pit) {
            if (c == this) {
                kError() << "UMLClassifier::getOpList: class " << c->getName()
                    << " is parent of itself ?!?" << endl;
                continue;
            }
            // get operations for each parent by recursive call
            UMLOperationList pops = c->getOpList(true);
            // add these operations to operation list, but only if unique.
            for (UMLOperation *po = pops.first(); po; po = pops.next()) {
                QString po_as_string(po->toString(Uml::st_SigNoVis));
                UMLOperation *o = NULL;
                for (o = ops.first(); o; o = ops.next())
                    if (o->toString(Uml::st_SigNoVis) == po_as_string)
                        break;
                if (!o)
                    ops.append(po);
            }
        }
    }
    return ops;
}

UMLClassifierListItemList UMLClassifier::getFilteredList(Uml::Object_Type ot) const {
    UMLClassifierListItemList resultList;
    UMLObject *o;
    for (UMLObjectListIt lit(m_List); (o = lit.current()) != NULL; ++lit) {
        if (o->getBaseType() == Uml::ot_Association)
            continue;
        UMLClassifierListItem *listItem = static_cast<UMLClassifierListItem*>(o);
        if (ot == Uml::ot_UMLObject || listItem->getBaseType() == ot)
            resultList.append(listItem);
    }
    return resultList;
}

UMLTemplate* UMLClassifier::addTemplate(const QString &name, Uml::IDType id) {
    UMLTemplate *t = findTemplate(name);
    if (t)
        return t;
    t = new UMLTemplate(this, name, id);
    m_List.append(t);
    emit templateAdded(t);
    UMLObject::emitModified();
    connect(t, SIGNAL(modified()), this, SIGNAL(modified()));
    return t;
}

bool UMLClassifier::addTemplate(UMLTemplate* newTemplate, IDChangeLog* log /* = 0*/) {
    QString name = newTemplate->getName();
    if (findChildObject(name) == NULL) {
        newTemplate->parent()->removeChild(newTemplate);
        this->insertChild(newTemplate);
        m_List.append(newTemplate);
        emit templateAdded(newTemplate);
        UMLObject::emitModified();
        connect(newTemplate,SIGNAL(modified()),this,SIGNAL(modified()));
        return true;
    } else if (log) {
        log->removeChangeByNewID( newTemplate->getID() );
        delete newTemplate;
    }
    return false;
}

bool UMLClassifier::addTemplate(UMLTemplate* Template, int position)
{
    QString name = Template->getName();
    if (findChildObject(name) == NULL) {
        Template->parent()->removeChild(Template);
        this->insertChild(Template);
        if( position >= 0 && position <= (int)m_List.count() )
            m_List.insert(position,Template);
        else
            m_List.append(Template);
        emit templateAdded(Template);
        UMLObject::emitModified();
        connect(Template,SIGNAL(modified()),this,SIGNAL(modified()));
        return true;
    }
    //else
    return false;
}

int UMLClassifier::removeTemplate(UMLTemplate* umltemplate) {
    if ( !m_List.remove(umltemplate) ) {
        kWarning() << "can't find att given in list" << endl;
        return -1;
    }
    emit templateRemoved(umltemplate);
    UMLObject::emitModified();
    disconnect(umltemplate,SIGNAL(modified()),this,SIGNAL(modified()));
    return m_List.count();
}


UMLTemplate *UMLClassifier::findTemplate(const QString& name) {
    UMLTemplateList templParams = getTemplateList();
    for (UMLTemplate *t = templParams.first(); t; t = templParams.next()) {
        if (t->getName() == name)
            return t;
    }
    return NULL;
}

int UMLClassifier::templates() {
    UMLClassifierListItemList tempList = getFilteredList(Uml::ot_Template);
    return tempList.count();
}

UMLTemplateList UMLClassifier::getTemplateList() const {
    UMLTemplateList templateList;
    for (UMLObjectListIt lit(m_List); lit.current(); ++lit) {
        UMLObject *listItem = lit.current();
        if (listItem->getBaseType() == Uml::ot_Template) {
            templateList.append(static_cast<UMLTemplate*>(listItem));
        }
    }
    return templateList;
}

int UMLClassifier::takeItem(UMLClassifierListItem *item) {
        UMLObject* currentAtt;
        QString buf;
        for (UMLObjectListIt it0(m_List);
             (currentAtt = it0.current()); ++it0) {
            QString txt = currentAtt->getName();
            if (txt.isEmpty())
              txt = "Type-" + QString::number((int) currentAtt->getBaseType());
            buf.append(' ' + currentAtt->getName());
        }
        kDebug() << "  UMLClassifier::takeItem (before): m_List is " << buf << endl;
    int index = m_List.findRef(item);
    if (index == -1)
        return -1;
    switch (item->getBaseType()) {
        case Uml::ot_Operation: {
            if (removeOperation(dynamic_cast<UMLOperation*>(item)) < 0)
                index = -1;
            break;
        }
        case Uml::ot_Attribute: {
            UMLAttribute *retval = dynamic_cast<UMLAttribute*>(m_List.take());
            if (retval) {
                emit attributeRemoved(retval);
                emit modified();
            } else {
                index = -1;
            }
            break;
        }
        case Uml::ot_Template: {
            UMLTemplate *t = dynamic_cast<UMLTemplate*>(m_List.take());
            if (t) {
                emit templateRemoved(t);
                emit modified();
            } else {
                index = -1;
            }
            break;
        }
        case Uml::ot_EnumLiteral: {
            UMLEnumLiteral *el = dynamic_cast<UMLEnumLiteral*>(m_List.take());
            if (el) {
                UMLEnum *e = static_cast<UMLEnum*>(this);
                e->signalEnumLiteralRemoved(el);
                emit modified();
            } else {
                index = -1;
            }
            break;
        }
        case Uml::ot_EntityAttribute: {
            UMLEntityAttribute* el = dynamic_cast<UMLEntityAttribute*>(m_List.take());
            if (el) {
                UMLEntity *e = static_cast<UMLEntity*>(this);
                e->signalEntityAttributeRemoved(el);
                emit modified();
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

void UMLClassifier::setOriginType(UMLClassifier *origType) {
    m_pSecondary = origType;
}

UMLClassifier * UMLClassifier::originType() const{
    return static_cast<UMLClassifier*>(m_pSecondary);
}

void UMLClassifier::setIsReference(bool isRef) {
    m_isRef = isRef;
}

bool UMLClassifier::isReference() const{
    return m_isRef;
}

UMLAssociationList  UMLClassifier::getUniAssociationToBeImplemented() {
    UMLAssociationList associations = getSpecificAssocs(Uml::at_UniAssociation);
    UMLAssociationList uniAssocListToBeImplemented;

    for(UMLAssociation *a = associations.first(); a; a = associations.next()) {
        if (a->getObjectId(Uml::B) == getID())
            continue;  // we need to be at the A side

        QString roleNameB = a->getRoleName(Uml::B);
        if (!roleNameB.isEmpty()) {
            UMLAttributeList atl = getAttributeList();
            bool found = false;
            //make sure that an attribute with the same name doesn't already exist
            for (UMLAttribute *at = atl.first(); at ; at = atl.next()) {
                if (at->getName() == roleNameB) {
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

void UMLClassifier::saveToXMI(QDomDocument & qDoc, QDomElement & qElement) {
    QString tag;
    switch (m_BaseType) {
        case Uml::ot_Class:
            tag = "UML:Class";
            break;
        case Uml::ot_Interface:
            tag = "UML:Interface";
            break;
        case Uml::ot_Datatype:
            tag = "UML:DataType";
            break;
        default:
            kError() << "UMLClassifier::saveToXMI() internal error: basetype is "
                << m_BaseType << endl;
            return;
    }
    QDomElement classifierElement = UMLObject::save(tag, qDoc);
    if (m_BaseType == Uml::ot_Datatype && m_pSecondary != NULL)
        classifierElement.setAttribute( "elementReference",
                                        ID2STR(m_pSecondary->getID()) );

    //save templates
    UMLClassifierListItemList list = getFilteredList(Uml::ot_Template);
    if (list.count()) {
        QDomElement tmplElement = qDoc.createElement( "UML:ModelElement.templateParameter" );
        for (UMLClassifierListItem *tmpl = list.first(); tmpl; tmpl = list.next() ) {
            tmpl->saveToXMI(qDoc, tmplElement);
        }
        classifierElement.appendChild( tmplElement );
    }

    //save generalizations (we are the subclass, the other end is the superclass)
    UMLAssociationList generalizations = getSpecificAssocs(Uml::at_Generalization);
    if (generalizations.count()) {
        QDomElement genElement = qDoc.createElement("UML:GeneralizableElement.generalization");
        for (UMLAssociation *a = generalizations.first(); a; a = generalizations.next()) {
            // We are the subclass if we are at the role A end.
            if (m_nId != a->getObjectId(Uml::A))
                continue;
            QDomElement gElem = qDoc.createElement("UML:Generalization");
            gElem.setAttribute( "xmi.idref", ID2STR(a->getID()) );
            genElement.appendChild(gElem);
        }
        if (genElement.hasChildNodes())
            classifierElement.appendChild( genElement );
    }

    // save attributes
    QDomElement featureElement = qDoc.createElement( "UML:Classifier.feature" );
    UMLClassifierListItemList attList = getFilteredList(Uml::ot_Attribute);
    for (UMLClassifierListItem *pAtt = attList.first(); pAtt; pAtt = attList.next() )
        pAtt -> saveToXMI( qDoc, featureElement );

    // save operations
    UMLOperationList opList = getOpList();
    for (UMLOperation *pOp = opList.first(); pOp; pOp = opList.next() )
        pOp -> saveToXMI( qDoc, featureElement );
    if (featureElement.hasChildNodes())
        classifierElement.appendChild( featureElement );

    // save contained objects
    if (m_objects.count()) {
        QDomElement ownedElement = qDoc.createElement( "UML:Namespace.ownedElement" );
        for (UMLObjectListIt oit(m_objects); oit.current(); ++oit) {
            UMLObject *obj = oit.current();
            obj->saveToXMI (qDoc, ownedElement);
        }
        classifierElement.appendChild( ownedElement );
    }
    qElement.appendChild( classifierElement );
}

UMLClassifierListItem* UMLClassifier::makeChildObject(const QString& xmiTag) {
    UMLClassifierListItem* pObject = NULL;
    if (tagEq(xmiTag, "Operation")) {
        pObject = new UMLOperation(this);
    } else if (tagEq(xmiTag, "Attribute")) {
        if (getBaseType() != Uml::ot_Class)
            return NULL;
        pObject = new UMLAttribute(this);
    } else if (tagEq(xmiTag, "TemplateParameter")) {
        pObject = new UMLTemplate(this);
    }
    return pObject;
}

bool UMLClassifier::load(QDomElement& element) {
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
        if (tagEq(tag, "ModelElement.templateParameter") ||
                tagEq(tag, "Classifier.feature") ||
                tagEq(tag, "Namespace.ownedElement") ||
                tagEq(tag, "Namespace.contents")) {
            load(element);
            // Not evaluating the return value from load()
            // because we want a best effort.

        } else if ((child = makeChildObject(tag)) != NULL) {
            if (child->loadFromXMI(element)) {
                switch (child->getBaseType()) {
                    case Uml::ot_Template:
                        addTemplate( static_cast<UMLTemplate*>(child) );
                        break;
                    case Uml::ot_Operation:
                        if (! addOperation(static_cast<UMLOperation*>(child)) ) {
                            kError() << "UMLClassifier::load: error from addOperation(op)"
                                      << endl;
                            delete child;
                            totalSuccess = false;
                        }
                        break;
                    case Uml::ot_Attribute:
                        addAttribute( static_cast<UMLAttribute*>(child) );
                        break;
                    default:
                        break;
                }
            } else {
                kWarning() << "UMLClassifier::load: failed to load " << tag << endl;
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



#include "classifier.moc"
