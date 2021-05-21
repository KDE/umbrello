/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "classifiercodedocument.h"

// local includes
#include "association.h"
#include "attribute.h"
#include "debug_utils.h"
#include "operation.h"
#include "classifierlistitem.h"
#include "classifier.h"
#include "codegenerator.h"
#include "uml.h"
#include "umldoc.h"
#include "umlrole.h"
#include "umlattributelist.h"
#include "umloperationlist.h"
#include "codegenfactory.h"

// qt includes
#include <QList>
#include <QRegExp>
#include <QXmlStreamWriter>

/**
 * Constructor.
 */
ClassifierCodeDocument::ClassifierCodeDocument(UMLClassifier * parent)
{
    init (parent);
}

/**
 * Destructor.
 */
ClassifierCodeDocument::~ClassifierCodeDocument()
{
    qDeleteAll(m_classfieldVector);
    m_classfieldVector.clear();
}

/**
 * Get a list of codeclassifier objects held by this classifiercodedocument that meet the passed criteria.
 */
CodeClassFieldList ClassifierCodeDocument::getSpecificClassFields(CodeClassField::ClassFieldType cfType)
{
    CodeClassFieldList list;
    CodeClassFieldList::ConstIterator it = m_classfieldVector.constBegin();
    CodeClassFieldList::ConstIterator end = m_classfieldVector.constEnd();
    for (; it != end; ++it)
    {
        if ((*it)->getClassFieldType() == cfType)
            list.append(*it);
    }
    return list;
}

/**
 * Get a list of codeclassifier objects held by this classifiercodedocument that meet the passed criteria.
 */
CodeClassFieldList ClassifierCodeDocument::getSpecificClassFields(CodeClassField::ClassFieldType cfType, bool isStatic)
{
    CodeClassFieldList list;
    CodeClassFieldList::ConstIterator it = m_classfieldVector.constBegin();
    CodeClassFieldList::ConstIterator end = m_classfieldVector.constEnd();
    for (; it != end; ++it)
    {
        CodeClassField *cf = *it;
        if (cf->getClassFieldType() == cfType && cf->getStatic() == isStatic)
            list.append(cf);
    }
    return list;
}

/**
 * Get a list of codeclassifier objects held by this classifiercodedocument that meet the passed criteria.
 */
CodeClassFieldList ClassifierCodeDocument::getSpecificClassFields (CodeClassField::ClassFieldType cfType, Uml::Visibility::Enum visibility)
{
    CodeClassFieldList list;
    CodeClassFieldList::ConstIterator it = m_classfieldVector.constBegin();
    CodeClassFieldList::ConstIterator end = m_classfieldVector.constEnd();
    for (; it != end; ++it)
    {
        CodeClassField * cf = *it;
        if (cf->getClassFieldType() == cfType && cf->getVisibility() == visibility)
            list.append(cf);
    }
    return list;
}

/**
 * Get a list of codeclassifier objects held by this classifiercodedocument that meet the passed criteria.
 */
CodeClassFieldList ClassifierCodeDocument::getSpecificClassFields (CodeClassField::ClassFieldType cfType, bool isStatic, Uml::Visibility::Enum visibility)
{
    CodeClassFieldList list;
    CodeClassFieldList::ConstIterator it = m_classfieldVector.constBegin();
    CodeClassFieldList::ConstIterator end = m_classfieldVector.constEnd();
    for (; it != end; ++it)
    {
        CodeClassField *cf = *it;
        if (cf->getClassFieldType() == cfType && cf->getVisibility() == visibility && cf->getStatic() == isStatic)
            list.append(cf);
    }
    return list;
}

// do we have accessor methods for lists of objects?
// (as opposed to lists of primitive types like 'int' or 'float', etc)

/**
 * Tell if any of the accessor classfields will be of lists of objects.
 */
bool ClassifierCodeDocument::hasObjectVectorClassFields()
{
    CodeClassFieldList::Iterator it = m_classfieldVector.begin();
    CodeClassFieldList::Iterator end = m_classfieldVector.end();
    for (; it != end; ++it)
    {
        if((*it)->getClassFieldType() != CodeClassField::Attribute)
        {
            UMLRole * role = (*it)->getParentObject()->asUMLRole();
            if (!role) {
                uError() << "invalid parent object type";
                return false;
            }
            QString multi = role->multiplicity();
            if (
                multi.contains(QRegExp(QLatin1String("[23456789\\*]"))) ||
                multi.contains(QRegExp(QLatin1String("1\\d")))
           )
                return true;
        }
    }
    return false;
}

/**
 * Does this object have any classfields declared?
 */
bool ClassifierCodeDocument::hasClassFields()
{
    if(m_classfieldVector.count() > 0)
        return true;
    return false;
}

/**
 * Tell if one or more codeclassfields are derived from associations.
 */
bool ClassifierCodeDocument::hasAssociationClassFields()
{
    CodeClassFieldList list = getSpecificClassFields(CodeClassField::Attribute);
    return (m_classfieldVector.count() - list.count()) > 0 ? true : false;
}

/**
 * Tell if one or more codeclassfields are derived from attributes.
 */
bool ClassifierCodeDocument::hasAttributeClassFields()
{
    CodeClassFieldList list = getSpecificClassFields(CodeClassField::Attribute);
    return list.count() > 0 ? true : false;
}

/**
 * Add a CodeClassField object to the m_classfieldVector List
 * We DON'T add methods of the code classfield here because we need to allow
 * the codegenerator writer the liberty to organize their document as they desire.
 * @return boolean true if successful in adding
 */
bool ClassifierCodeDocument::addCodeClassField (CodeClassField * add_object)
{
    UMLObject * umlobj = add_object->getParentObject();
    if(!(m_classFieldMap.contains(umlobj)))
    {
        m_classfieldVector.append(add_object);
        m_classFieldMap.insert(umlobj, add_object);

        return true;
    }
    return false;
}

/**
 * Synchronize this document to the attributes/associations of the parent classifier.
 * This is a slot..should only be called from a signal.
 */
void ClassifierCodeDocument::addAttributeClassField (UMLClassifierListItem *obj, bool syncToParentIfAdded)
{
    UMLAttribute *at = (UMLAttribute*)obj;

    // This can be signalled multiple times: after creation and after calling resolveRef,
    // skip creation if the attribute is already there.
    if (m_classFieldMap.contains(at)) {
        return;
    }

    CodeClassField * cf = CodeGenFactory::newCodeClassField(this, at);
    if (cf) {
        if (!addCodeClassField(cf)) {
            // If cf was not added to m_classFieldMap, it must be deleted to
            // ensure correct cleanup when closing umbrello.
            delete cf;
        } else if (syncToParentIfAdded) {
            updateContent();
        }
    }
}

/**
 * Remove a CodeClassField object from m_classfieldVector List
 */
bool ClassifierCodeDocument::removeCodeClassField (CodeClassField * remove_object)
{
    UMLObject * umlobj = remove_object->getParentObject();
    if(m_classFieldMap.contains(umlobj))
    {
        if (m_classfieldVector.removeAll(remove_object))
        {
            // remove from our classfield map
            m_classFieldMap.remove(umlobj);
            delete remove_object;
            return true;
        }
    }
    return false;
}

void ClassifierCodeDocument::removeAttributeClassField(UMLClassifierListItem *obj)
{
    CodeClassField * remove_object = m_classFieldMap[obj];
    if(remove_object)
        removeCodeClassField(remove_object);
}

void ClassifierCodeDocument::removeAssociationClassField (UMLAssociation *assoc)
{
    // the object could be either (or both!) role a or b. We should check
    // both parts of the association.
    CodeClassField * remove_object = m_classFieldMap[assoc->getUMLRole(Uml::RoleType::A)];
    if(remove_object)
        removeCodeClassField(remove_object);

    // check role b
    remove_object = m_classFieldMap[assoc->getUMLRole(Uml::RoleType::B)];
    if(remove_object)
        removeCodeClassField(remove_object);
}

/**
 * Get the list of CodeClassField objects held by m_classfieldVector
 * @return CodeClassFieldList list of CodeClassField objects held by
 * m_classfieldVector
 */
CodeClassFieldList * ClassifierCodeDocument::getCodeClassFieldList ()
{
    return &m_classfieldVector;
}

/**
 * Get the value of m_parentclassifier
 * @return the value of m_parentclassifier
 */
UMLClassifier * ClassifierCodeDocument::getParentClassifier ()
{
    return m_parentclassifier;
}

/**
 * Get a list of codeoperation objects held by this classifiercodedocument.
 * @return      QList<CodeOperation>
 */
QList<CodeOperation*> ClassifierCodeDocument::getCodeOperations ()
{
    QList<CodeOperation*> list;

    TextBlockList * tlist = getTextBlockList();
    foreach (TextBlock* tb, *tlist)
    {
        CodeOperation * cop = dynamic_cast<CodeOperation*>(tb);
        if (cop) {
            list.append(cop);
        }
    }
    return list;
}

/**
 * @param  o The Operation to add
 */
void ClassifierCodeDocument::addOperation (UMLClassifierListItem * o)
{
    UMLOperation *op = o->asUMLOperation();
    if (op == 0) {
        uError() << "arg is not a UMLOperation";
        return;
    }
    QString tag = CodeOperation::findTag(op);
    CodeOperation * codeOp = dynamic_cast<CodeOperation*>(findTextBlockByTag(tag, true));
    bool createdNew = false;

    // create the block, if it doesn't already exist
    if(!codeOp)
    {
        codeOp = CodeGenFactory::newCodeOperation(this, op);
        createdNew = true;
    }

    // now try to add it. This may fail because it (or a block with
    // the same tag) is already in the document somewhere. IF we
    // created this new, then we need to delete our object.
    if(!addCodeOperation(codeOp)) // wont add if already present
        if(createdNew)
            delete codeOp;
}

/**
 * @param   op
 */
void ClassifierCodeDocument::removeOperation (UMLClassifierListItem * op)
{
    QString tag = CodeOperation::findTag((UMLOperation*)op);
    TextBlock *tb = findTextBlockByTag(tag, true);
    if(tb)
    {
        if(removeTextBlock(tb)) // wont add if already present
            delete tb; // delete unused operations
        else
            uError() << "Cant remove CodeOperation from ClassCodeDocument!";

    }
    else
        uError() << "Cant Find codeOperation for deleted operation!";
}

// Other methods
//

/**
 * A utility method that allows user to easily add classfield methods to this document.
 */
void ClassifierCodeDocument::addCodeClassFieldMethods(CodeClassFieldList &list)
{
    CodeClassFieldList::Iterator it = list.begin();
    CodeClassFieldList::Iterator end = list.end();
    for (; it!= end; ++it)
    {
        CodeClassField * field = *it;
        CodeAccessorMethodList list = field->getMethodList();
        Q_FOREACH(CodeAccessorMethod *method, list)
        {
            /*
                QString tag = method->getTag();
                if(tag.isEmpty()) {
                        tag = getUniqueTag();
                        method->setTag(tag);
                }
            */
            addTextBlock(method); // wont add if already exists in document, will add a tag if missing;
        }
    }
}

/**
 * Add declaration blocks for the passed classfields.
 */
void ClassifierCodeDocument::declareClassFields (CodeClassFieldList & list,
        CodeGenObjectWithTextBlocks * parent)
{
    CodeClassFieldList::Iterator it = list.begin();
    CodeClassFieldList::Iterator end = list.end();
    for (; it!= end; ++it)
    {
        CodeClassField * field = *it;
        CodeClassFieldDeclarationBlock * declBlock = field->getDeclarationCodeBlock();

        /*
              // if it has a tag, check
              if(!declBlock->getTag().isEmpty())
              {
                      // In C++, because we may shift the declaration to a different parent
                      // block for a change in scope, we need to track down any pre-existing
                      // location, and remove FIRST before adding to new parent
                      CodeGenObjectWithTextBlocks * oldParent = findParentObjectForTaggedTextBlock (declBlock->getTag());
                      if(oldParent) {
                              if(oldParent != parent)
                                      oldParent->removeTextBlock(declBlock);
                      }
              }
        */

        parent->addTextBlock(declBlock); // wont add it IF its already present. Will give it a tag if missing
    }
}

/**
 * Return if the parent classifier is a class
 */
bool ClassifierCodeDocument::parentIsClass()
{
    return (m_parentclassifier->baseType() == UMLObject::ot_Class);
}

/**
 * Return if the parent classifier is an interface
 */
bool ClassifierCodeDocument::parentIsInterface()
{
    return (m_parentclassifier->baseType() == UMLObject::ot_Interface);
}

/**
 * Init from a UMLClassifier object.
 * @param       classifier
 * @param       package
 */
void ClassifierCodeDocument::init (UMLClassifier * c)
{
    m_parentclassifier = c;

    updateHeader();
    syncNamesToParent();
    // initCodeClassFields(); // cant call here?..newCodeClassField is pure virtual

    // slots
    if (parentIsClass())  {
        connect(c, SIGNAL(attributeAdded(UMLClassifierListItem*)), this, SLOT(addAttributeClassField(UMLClassifierListItem*)));
        connect(c, SIGNAL(attributeRemoved(UMLClassifierListItem*)), this, SLOT(removeAttributeClassField(UMLClassifierListItem*)));
    }

    connect(c, SIGNAL(sigAssociationEndAdded(UMLAssociation*)), this, SLOT(addAssociationClassField(UMLAssociation*)));
    connect(c, SIGNAL(sigAssociationEndRemoved(UMLAssociation*)), this, SLOT(removeAssociationClassField(UMLAssociation*)));
    connect(c, SIGNAL(operationAdded(UMLClassifierListItem*)), this, SLOT(addOperation(UMLClassifierListItem*)));
    connect(c, SIGNAL(operationRemoved(UMLClassifierListItem*)), this, SLOT(removeOperation(UMLClassifierListItem*)));
    connect(c, SIGNAL(modified()), this, SLOT(syncToParent()));

}

/**
 * IF the classifier object is modified, this will get called.
 * @todo we cannot make this virtual as long as the
 *       ClassifierCodeDocument constructor calls it because that gives
 *       a call-before-construction error.
 * Example of the problem: CPPSourceCodeDocument reimplementing syncNamesToParent()
 *  CPPCodeGenerator::initFromParentDocument()
 *    CodeDocument * codeDoc = new CPPSourceCodeDocument(c);
 *      CPPSourceCodeDocument::CPPSourceCodeDocument(UMLClassifier * concept)
 *       : ClassifierCodeDocument(concept)
 *        ClassifierCodeDocument::ClassifierCodeDocument(concept)
 *         init(concept);
 *          syncNamesToParent();
 *            dispatches to CPPSourceCodeDocument::syncNamesToParent()
 *            but that object is not yet constructed.
 */
void ClassifierCodeDocument::syncNamesToParent()
{
    QString fileName = CodeGenerator::cleanName(getParentClassifier()->name());
    if (!UMLApp::app()->activeLanguageIsCaseSensitive()) {
        // @todo let the user decide about mixed case file names (codegen setup menu)
        fileName = fileName.toLower();
    }
    setFileName(fileName);
    setPackage(m_parentclassifier->umlPackage());
}

/**
 * Cause this classifier code document to synchronize to current policy.
 */
void ClassifierCodeDocument::synchronize()
{
    updateHeader(); // doing this insures time/date stamp is at the time of this call
    syncNamesToParent();
    updateContent();
    syncClassFields();
    updateOperations();

}

/**
 * Force synchronization of child classfields to their parent objects.
 */
void ClassifierCodeDocument::syncClassFields()
{
    CodeClassFieldList::Iterator it = m_classfieldVector.begin();
    CodeClassFieldList::Iterator end = m_classfieldVector.end();
    for (; it!= end; ++it)
        (*it)->synchronize();
}

/**
 * Update code operations in this document using the parent classifier.
 */
void ClassifierCodeDocument::updateOperations()
{
    UMLOperationList opList(getParentClassifier()->getOpList());
    foreach (UMLOperation *op, opList) {
        QString tag = CodeOperation::findTag(op);
        CodeOperation * codeOp = dynamic_cast<CodeOperation*>(findTextBlockByTag(tag, true));
        bool createdNew = false;

        if(!codeOp)
        {
            codeOp = CodeGenFactory::newCodeOperation(this, op);
            createdNew = true;
        }

        // now try to add it. This may fail because it (or a block with
        // the same tag) is already in the document somewhere. IF we
        // created this new, then we need to delete our object.
        if(!addCodeOperation(codeOp)) // wont add if already present
            if(createdNew)
                delete codeOp;

        // synchronize all non-new operations
        if(!createdNew)
            codeOp->syncToParent();
    }
}

void ClassifierCodeDocument::syncToParent()
{
    synchronize();
}

/**
 * Add codeclassfields to this classifiercodedocument. If a codeclassfield
 * already exists, it is not added.
 */
void ClassifierCodeDocument::initCodeClassFields()
{
    UMLClassifier * c = getParentClassifier();
    // first, do the code classifields that arise from attributes
    if (parentIsClass()) {
        UMLAttributeList alist = c->getAttributeList();
        foreach(UMLAttribute * at, alist) {
            CodeClassField * field = CodeGenFactory::newCodeClassField(this, at);
            addCodeClassField(field);
        }

    }

    // now, do the code classifields that arise from associations
    UMLAssociationList ap = c->getSpecificAssocs(Uml::AssociationType::Association);
    UMLAssociationList ag = c->getAggregations();
    UMLAssociationList ac = c->getCompositions();
    UMLAssociationList selfAssoc = c->getSpecificAssocs(Uml::AssociationType::Association_Self);

    updateAssociationClassFields(ap);
    updateAssociationClassFields(ag);
    updateAssociationClassFields(ac);
    updateAssociationClassFields(selfAssoc);
}

/**
 * Using the passed list, update our inventory of CodeClassFields which are
 * based on UMLRoles (e.g. derived from associations with other classifiers).
 */
void ClassifierCodeDocument::updateAssociationClassFields (UMLAssociationList &assocList)
{
    foreach(UMLAssociation * a, assocList)
        addAssociationClassField(a, false); // syncToParent later
}

void ClassifierCodeDocument::addAssociationClassField (UMLAssociation * a, bool syncToParentIfAdded)
{
    Uml::ID::Type cid = getParentClassifier()->id(); // so we know who 'we' are
    bool printRoleA = false, printRoleB = false, shouldSync = false;
    // it may seem counter intuitive, but you want to insert the role of the
    // *other* class into *this* class.
    if (a->getObjectId(Uml::RoleType::A) == cid)
        printRoleB = true;

    if (a->getObjectId(Uml::RoleType::B) == cid)
        printRoleA = true;

    // grab RoleB decl
    if (printRoleB)
    {
        UMLRole * role = a->getUMLRole(Uml::RoleType::B);
        if(!m_classFieldMap.contains((UMLObject*)role))
        {
            CodeClassField * classfield = CodeGenFactory::newCodeClassField(this, role);
            if(addCodeClassField(classfield))
                shouldSync = true;
        }
    }

    // print RoleA decl
    if (printRoleA)
    {
        UMLRole * role = a->getUMLRole(Uml::RoleType::A);
        if(!m_classFieldMap.contains((UMLObject*)role))
        {
            CodeClassField * classfield = CodeGenFactory::newCodeClassField(this, role);
            if(addCodeClassField(classfield))
                shouldSync = true;
        }
    }

    if (shouldSync && syncToParentIfAdded)
        syncToParent(); // needed for a slot add

}

/**
 * Set the class attributes of this object from
 * the passed element node.
 */
void ClassifierCodeDocument::setAttributesFromNode (QDomElement & elem)
{
    // NOTE: we DON'T set the parent here as we ONLY get to this point
    // IF the parent codegenerator could find a matching parent classifier
    // that already has a code document.

    // We FIRST set code class field stuff..check re-linnking with
    // accessor methods by looking for our particular child element
    QDomNode node = elem.firstChild();
    QDomElement childElem = node.toElement();
    while(!childElem.isNull()) {
        QString tag = childElem.tagName();
        if(tag == QLatin1String("classfields")) {
            // load classfields
            loadClassFieldsFromXMI(childElem);
            break;
        }
        node = childElem.nextSibling();
        childElem= node.toElement();
    }

    // call super-class after. THis will populate the text blocks (incl
    // the code accessor methods above) as is appropriate
    CodeDocument::setAttributesFromNode(elem);
}

// look at all classfields currently in document.. match up
// by parent object ID and Role ID (needed for self-association CF's)
CodeClassField *
ClassifierCodeDocument::findCodeClassFieldFromParentID (Uml::ID::Type id,
        int role_id)
{
    CodeClassFieldList::Iterator it = m_classfieldVector.begin();
    CodeClassFieldList::Iterator end = m_classfieldVector.end();
    for (; it != end; ++it)
    {
        CodeClassField * cf = *it;
        if(role_id == -1) { // attribute-based
            if (Uml::ID::fromString(cf->ID()) == id)
                return cf;
        } else { // association(role)-based
            const Uml::RoleType::Enum r = Uml::RoleType::fromInt(role_id);
            UMLRole * role = cf->getParentObject()->asUMLRole();
            if(role && Uml::ID::fromString(cf->ID()) == id && role->role() == r)
                return cf;
        }
    }

    // shouldn't happen..
    uError() << "Failed to find codeclassfield for parent uml id:"
             << Uml::ID::toString(id) << " (role id:" << role_id
             << ") Do you have a corrupt classifier code document?";

    return (CodeClassField*) 0; // not found
}

/**
 * Load CodeClassFields from XMI element node.
 */
void ClassifierCodeDocument::loadClassFieldsFromXMI(QDomElement & elem)
{
    QDomNode node = elem.firstChild();
    QDomElement childElem = node.toElement();
    while(!childElem.isNull()) {
        QString nodeName = childElem.tagName();
        if(nodeName == QLatin1String("codeclassfield"))
        {
            QString id = childElem.attribute(QLatin1String("parent_id"), QLatin1String("-1"));
            int role_id = childElem.attribute(QLatin1String("role_id"), QLatin1String("-1")).toInt();
            CodeClassField * cf = findCodeClassFieldFromParentID(Uml::ID::fromString(id), role_id);
            if(cf)
            {
                // Because we just may change the parent object here,
                // we need to yank it from the map of umlobjects
                m_classFieldMap.remove(cf->getParentObject());

                // configure from XMI
                cf->loadFromXMI1(childElem);

                // now add back in
                m_classFieldMap.insert(cf->getParentObject(), cf);

            } else
                uError()<<" LoadFromXMI: cannot load classfield parent_id:"<<id<<" do you have a corrupt savefile?";
        }
        node = childElem.nextSibling();
        childElem= node.toElement();
    }
}

/**
 * Save the XMI representation of this object.
 */
void ClassifierCodeDocument::saveToXMI1(QXmlStreamWriter& writer)
{
#if 0
    // avoid the creation of primitive data type
    QString strType;
    if (getParentClassifier()->getBaseType() == Uml::ot_Datatype) {
        strType = getParentClassifier()->getName();
        // lets get the default code generator to check if it is a primitive data type
        // there's a reason to create files for int/boolean and so ?
        if (getParentGenerator()->isReservedKeyword(strType))
           return;
    }
#endif
    writer.writeStartElement(QLatin1String("classifiercodedocument"));

    setAttributesOnNode(writer);

    writer.writeEndElement();
}

/**
 * Load params from the appropriate XMI element node.
 */
void ClassifierCodeDocument::loadFromXMI1 (QDomElement & root)
{
    // set attributes/fields
    setAttributesFromNode(root);

    // now sync our doc, needed?
    // synchronize();
}

/**
 * Set attributes of the node that represents this class
 * in the XMI document.
 */
void ClassifierCodeDocument::setAttributesOnNode (QXmlStreamWriter& writer)
{
    // do super-class first
    CodeDocument::setAttributesOnNode(writer);

    // cache local attributes/fields
    writer.writeAttribute(QLatin1String("parent_class"), Uml::ID::toString(getParentClassifier()->id()));

    // (code) class fields
    // which we will store in its own separate child node block
    writer.writeStartElement(QLatin1String("classfields"));
    CodeClassFieldList::Iterator it = m_classfieldVector.begin();
    CodeClassFieldList::Iterator end = m_classfieldVector.end();
    for (; it!= end; ++it)
        (*it)->saveToXMI1(writer);
    writer.writeEndElement();
}

/**
 * Find a specific textblock held by any code class field in this document
 * by its tag.
 */
TextBlock * ClassifierCodeDocument::findCodeClassFieldTextBlockByTag (const QString &tag)
{
    CodeClassFieldList::Iterator it = m_classfieldVector.begin();
    CodeClassFieldList::Iterator end = m_classfieldVector.end();
    for (; it!= end; ++it)
    {
        CodeClassField * cf = *it;
        CodeClassFieldDeclarationBlock * decl = cf->getDeclarationCodeBlock();
        if(decl && decl->getTag() == tag)
            return decl;
        // well, if not in the decl block, then in the methods perhaps?
        CodeAccessorMethodList mlist = cf->getMethodList();
        Q_FOREACH(CodeAccessorMethod *m, mlist)
            if(m->getTag() == tag)
                return m;
    }
    // if we get here, we failed.
    return (TextBlock*) 0;
}


