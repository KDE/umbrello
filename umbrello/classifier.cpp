/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
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
#include "stereotype.h"
#include "umldoc.h"
#include "uml.h"
#include "umllistview.h"
#include "model_utils.h"
#include "clipboard/idchangelog.h"
#include "dialogs/umloperationdialog.h"
#include "dialogs/umlattributedialog.h"
#include "dialogs/umltemplatedialog.h"
#include "optionstate.h"

using namespace Uml;

UMLClassifier::UMLClassifier(const QString & name, Uml::IDType id, bool bIsInterface)
        : UMLPackage(name, id)
{
    init(bIsInterface);
}

UMLClassifier::~UMLClassifier() {
}

void UMLClassifier::init(bool bIsInterface /* = false */) {
    setInterface(bIsInterface);
    m_pClassAssoc = NULL;
}

void UMLClassifier::setInterface(bool b /* = true */) {
    // @todo get rid of direct dependencies to UMLListView
    //  (e.g. move utility methods to Model_Utils and/or use signals)
    UMLListView::Icon_Type newIcon;
    if (b) {
        m_BaseType = ot_Interface;
        UMLObject::setStereotype("interface");
        newIcon = UMLListView::it_Interface;
    } else {
        m_BaseType = ot_Class;
        UMLObject::setStereotype(QString::null);
        newIcon = UMLListView::it_Class;
    }
    UMLListView *listView = UMLApp::app()->getListView();
    listView->changeIconOf(this, newIcon);
}

bool UMLClassifier::isInterface() const {
    return (m_BaseType == ot_Interface);
}

UMLOperation * UMLClassifier::checkOperationSignature( QString name,
        UMLAttributeList *opParams,
        UMLOperation *exemptOp)
{
    UMLOperationList list = findOperations(name);
    if( list.count() == 0 )
        return NULL;
    int inputParmCount = (opParams ? opParams->count() : 0);

    // there is at least one operation with the same name... compare the parameter list
    for (UMLOperationListIt oit(list); oit.current(); ++oit)
    {
        UMLOperation* test = oit.current();
        if (test == exemptOp)
            continue;
        UMLAttributeList *testParams = test->getParmList( );
        if (!opParams) {
            if (0 == testParams->count())
                return test;
            continue;
        }
        int pCount = testParams->count();
        if( pCount != inputParmCount )
            continue;
        int i = 0;
        for( ; i < pCount; ++i )
        {
            // The only criterion for equivalence is the parameter types.
            // (Default values are not considered.)
            if( testParams->at(i)->getTypeName() != opParams->at(i)->getTypeName() )
                break;
        }
        if( i == pCount )
        {//all parameters matched -> the signature is not unique
            return test;
        }
    }
    // we did not find an exact match, so the signature is unique ( acceptable )
    return NULL;
}

UMLOperation* UMLClassifier::findOperation(QString name, Model_Utils::NameAndType_List params) {
    UMLOperationList list = findOperations(name);
    if (list.count() == 0)
        return NULL;
    // If there are operation(s) with the same name then compare the parameter list
    const int inputParmCount = params.count();
    UMLOperation* test = NULL;
    for (UMLOperationListIt oit(list); (test = oit.current()) != NULL; ++oit) {
        UMLAttributeList *testParams = test->getParmList();
        const int pCount = testParams->count();
        if (inputParmCount == 0 && pCount == 0)
            break;
        if (inputParmCount != pCount)
            continue;
        int i = 0;
        for (; i < pCount; ++i) {
            Model_Utils::NameAndType_ListIt nt(params.at(i));
            UMLClassifier *c = dynamic_cast<UMLClassifier*>((*nt).m_type);
            UMLClassifier *testType = testParams->at(i)->getType();
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
            UMLAttribute *par = new UMLAttribute(op, nt.m_name);
            par->setType(nt.m_type);
            par->setParmKind(nt.m_direction);
            par->setInitialValue(nt.m_initialValue);
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
    addOperation( op );

    UMLDoc *umldoc = UMLApp::app()->getDocument();
    umldoc->signalUMLObjectCreated(op);
    return op;
}

bool UMLClassifier::addOperation(UMLOperation* op, int position )
{
    if (m_List.findRef(op) != -1) {
        kdDebug() << "UMLClassifier::addOperation: findRef("
        << op->getName() << ") finds op (bad)"
        << endl;
        return false;
    }
    if (checkOperationSignature(op->getName(), op->getParmList()) ) {
        kdDebug() << "UMLClassifier::addOperation: checkOperationSignature("
        << op->getName() << ") op is non-unique" << endl;
        return false;
    }

    if( position >= 0 && position <= (int)m_List.count() )
        m_List.insert(position,op);
    else
        m_List.append( op );
    emit operationAdded(op);
    emit modified();
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
        kdDebug() << "UMLClassifier::removeOperation called on NULL op"
        << endl;
        return -1;
    }
    if(!m_List.remove(op)) {
        kdDebug() << "UMLClassifier::removeOperation: can't find op "
        << op->getName() << " in list" << endl;
        return -1;
    }
    // disconnection needed.
    // note that we dont delete the operation, just remove it from the Classifier
    disconnect(op,SIGNAL(modified()),this,SIGNAL(modified()));
    emit operationRemoved(op);
    emit modified();
    return m_List.count();
}

UMLOperation* UMLClassifier::takeOperation(UMLOperation* o, int *wasAtIndex) {
    int index = m_List.findRef(o);
    if (wasAtIndex)
        *wasAtIndex = index;
    if (removeOperation(o) >= 0) {
        return o;
    }
    return 0;
}

UMLObject* UMLClassifier::createTemplate(QString currentName /*= QString::null*/) {
    bool goodName = !currentName.isEmpty();
    if (!goodName)
        currentName = uniqChildName(Uml::ot_Template);
    UMLTemplate* newTemplate = new UMLTemplate(this, currentName);

    int button = QDialog::Accepted;

    while (button==QDialog::Accepted && !goodName) {
        UMLTemplateDialog templateDialogue(0, newTemplate);
        button = templateDialogue.exec();
        QString name = newTemplate->getName();

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

UMLAttributeList UMLClassifier::getAttributeList() {
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
                            || (concept->isInterface() && type == INTERFACE)))
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
                            || (concept->isInterface() && type == INTERFACE)))
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
    rhs->setInterface(isInterface());
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

UMLObject* UMLClassifier::createAttribute(const QString &name /*=null*/) {
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    Uml::IDType id = umldoc->getUniqueID();
    QString currentName;
    if (name.isNull())  {
        currentName = uniqChildName(Uml::ot_Attribute);
    } else {
        currentName = name;
    }
    const Settings::OptionState optionState = UMLApp::app()->getOptionState();
    Uml::Visibility scope = optionState.classState.defaultAttributeScope;
    UMLAttribute* newAttribute = new UMLAttribute(this, currentName, id, scope);

    int button = QDialog::Accepted;
    bool goodName = false;

    //check for name.isNull() stops dialogue being shown
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
        return NULL;
    }

    addAttribute(newAttribute);

    umldoc->signalUMLObjectCreated(newAttribute);
    return newAttribute;
}

UMLAttribute* UMLClassifier::addAttribute(const QString &name, Uml::IDType id /* = Uml::id_None */) {
    for (UMLObjectListIt lit(m_List); lit.current(); ++lit) {
        UMLObject *obj = lit.current();
        if (obj->getBaseType() == Uml::ot_Attribute && obj->getName() == name)
            return static_cast<UMLAttribute*>(obj);
    }
    UMLApp *app = UMLApp::app();
    Uml::Visibility scope = app->getOptionState().classState.defaultAttributeScope;
    UMLAttribute *a = new UMLAttribute(this, name, id, scope);
    m_List.append(a);
    emit modified();
    connect(a,SIGNAL(modified()),this,SIGNAL(modified()));
    emit attributeAdded(a);
    return a;
}

UMLAttribute* UMLClassifier::addAttribute(const QString &name, UMLObject *type, Uml::Visibility scope) {
    UMLAttribute *a = new UMLAttribute(this, name, Uml::id_None, scope);
    if (type)
        a->setType(type);
    m_List.append(a);
    emit modified();
    connect(a,SIGNAL(modified()),this,SIGNAL(modified()));
    emit attributeAdded(a);
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
        emit modified();
        connect(att, SIGNAL(modified()), this, SIGNAL(modified()));
        emit attributeAdded(att);
        return true;
    } else if (Log) {
        Log->removeChangeByNewID(att->getID());
        delete att;
    }
    return false;
}

int UMLClassifier::removeAttribute(UMLAttribute* a) {
    if (!m_List.remove(a)) {
        kdDebug() << "can't find att given in list" << endl;
        return -1;
    }
    emit attributeRemoved(a);
    emit modified();
    // If we are deleteing the object, then we dont need to disconnect..this is done auto-magically
    // for us by QObject. -b.t.
    // disconnect(a,SIGNAL(modified()),this,SIGNAL(modified()));
    delete a;
    return m_List.count();
}

UMLAttribute* UMLClassifier::takeAttribute(UMLAttribute* a, int *wasAtIndex) {
    int index = m_List.findRef( a );
    if (wasAtIndex)
        *wasAtIndex = index;
    UMLAttribute *retval = NULL;
    if (index != -1)
        retval = dynamic_cast<UMLAttribute*>(m_List.take( ));
    if (retval) {
        emit attributeRemoved(retval);
        emit modified();
    }
    kdDebug() << "UMLClassifier::takeAttribute: findRef returns " << index
    << ", a=" << a << ", retval=" << retval << endl;
    return retval;
}

void UMLClassifier::setClassAssoc(UMLAssociation *assoc) {
    m_pClassAssoc = assoc;
}

UMLAssociation *UMLClassifier::getClassAssoc() {
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
        UMLClassifierList parents(findSuperClassConcepts());
        for (UMLClassifierListIt pit(parents); pit.current(); ++pit) {
            // get operations for each parent by recursive call
            UMLOperationList pops = pit.current()->getOpList(true);
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

UMLClassifierListItemList UMLClassifier::getFilteredList(Object_Type ot) {
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
    emit modified();
    connect(t, SIGNAL(modified()), this, SIGNAL(modified()));
    emit templateAdded(t);
    return t;
}

bool UMLClassifier::addTemplate(UMLTemplate* newTemplate, IDChangeLog* log /* = 0*/) {
    QString name = newTemplate->getName();
    if (findChildObject(name) == NULL) {
        newTemplate->parent()->removeChild(newTemplate);
        this->insertChild(newTemplate);
        m_List.append(newTemplate);
        emit modified();
        connect(newTemplate,SIGNAL(modified()),this,SIGNAL(modified()));
        emit templateAdded(newTemplate);
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
        emit modified();
        connect(Template,SIGNAL(modified()),this,SIGNAL(modified()));
        emit templateAdded(Template);
        return true;
    }
    //else
    return false;
}

int UMLClassifier::removeTemplate(UMLTemplate* umltemplate) {
    if ( !m_List.remove(umltemplate) ) {
        kdWarning() << "can't find att given in list" << endl;
        return -1;
    }
    emit templateRemoved(umltemplate);
    emit modified();
    disconnect(umltemplate,SIGNAL(modified()),this,SIGNAL(modified()));
    return m_List.count();
}

UMLTemplate* UMLClassifier::takeTemplate(UMLTemplate* t, int *wasAtIndex) {
    int index = m_List.findRef( t );
    if (wasAtIndex)
        *wasAtIndex = index;
    t = (index == -1 ? 0 : dynamic_cast<UMLTemplate*>(m_List.take( )));
    if (t) {
        emit templateRemoved(t);
        emit modified();
    }
    return t;
}

UMLTemplate *UMLClassifier::findTemplate(QString name) {
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

UMLTemplateList UMLClassifier::getTemplateList() {
    UMLTemplateList templateList;
    for (UMLObjectListIt lit(m_List); lit.current(); ++lit) {
        UMLObject *listItem = lit.current();
        if (listItem->getBaseType() == Uml::ot_Template) {
            templateList.append(static_cast<UMLTemplate*>(listItem));
        }
    }
    return templateList;
}


void UMLClassifier::saveToXMI(QDomDocument & qDoc, QDomElement & qElement) {
    QDomElement classifierElement;
    if (this->isInterface())
        classifierElement = UMLObject::save("UML:Interface", qDoc);
    else
        classifierElement = UMLObject::save("UML:Class", qDoc);

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

bool UMLClassifier::load(QDomElement& element) {

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
            if (! load(element))
                return false;
        } else if (tagEq(tag, "TemplateParameter")) {
            UMLTemplate* tmplParm = new UMLTemplate(this);
            if (!tmplParm->loadFromXMI(element)) {
                delete tmplParm;
            } else {
                addTemplate(tmplParm);
            }
        } else if (tagEq(tag, "Operation")) {
            UMLOperation* op = new UMLOperation(this);
            if (!op->loadFromXMI(element)) {
                kdError() << "UMLClassifier::load: error from op->loadFromXMI()"
                << endl;
                delete op;
                return true;
                // Returning false here will spoil the entire load.
                // At this point the user has been warned that
                // something went wrong so let's still try to go on.
            }
            if (!addOperation(op) ) {
                kdError() << "UMLClassifier::load: error from addOperation(op)"
                << endl;
                delete op;
                //return false;
                // Returning false here will spoil the entire
                // load. At this point the user has been warned
                // that something went wrong so let's still try
                // our best effort.
            }
        } else if (Uml::tagEq(tag, "Attribute")) {
            UMLAttribute * pAtt = new UMLAttribute(this);
            if (!pAtt->loadFromXMI(element)) {
                delete pAtt;
                return false;
            }
            addAttribute(pAtt);
        } else if (!Model_Utils::isCommonXMIAttribute(tag)) {
            UMLDoc *umldoc = UMLApp::app()->getDocument();
            UMLObject *pObject = UMLDoc::makeNewUMLObject(tag);
            if( !pObject )
                continue;
            pObject->setUMLPackage(this);
            if (pObject->loadFromXMI(element)) {
                addObject(pObject);
                if (tagEq(tag, "Generalization"))
                    umldoc->addAssocToConcepts((UMLAssociation *) pObject);
            } else {
                delete pObject;
            }
        }
    }
    return true;
}

#include "classifier.moc"
