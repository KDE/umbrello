
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*  This code generated by:
 *      Author : thomas
 *      Date   : Fri Jun 20 2003
 */

#include <qregexp.h>
#include <kdebug.h>

#include "codeclassfield.h"

#include "association.h"
#include "classifiercodedocument.h"
#include "codegenerator.h"
#include "attribute.h"
#include "umlobject.h"
#include "umlrole.h"

// Constructors/Destructors
//

CodeClassField::CodeClassField ( ClassifierCodeDocument * doc , UMLRole * role)
        : CodeParameter ( doc , (UMLObject*) role)
{

    setParentUMLObject(role);
    initFields( );

}

CodeClassField::CodeClassField ( ClassifierCodeDocument * doc , UMLAttribute * attrib)
        : CodeParameter ( doc , (UMLObject*) attrib )
{

    setParentUMLObject(attrib);
    initFields( );

}

CodeClassField::~CodeClassField ( ) {

    // remove methods from parent document
    CodeAccessorMethodList list = m_methodVector;
    for(CodeAccessorMethod * m = list.first(); m ; m=list.next())
    {
        getParentDocument()->removeTextBlock(m);
        m->forceRelease();
    }
    list.clear();

    // clear the decl block from parent text block list too
    if(m_declCodeBlock)
    {
        getParentDocument()->removeTextBlock(m_declCodeBlock);
        m_declCodeBlock->forceRelease();
        delete m_declCodeBlock;
    }

}

//
// Methods
//


// Accessor methods
//

void CodeClassField::setParentUMLObject (UMLObject * obj) {
    UMLRole *role = dynamic_cast<UMLRole*>(obj);
    if(role) {
        UMLAssociation * parentAssoc = role->getParentAssociation();
        Uml::Association_Type atype = parentAssoc->getAssocType();
        m_parentIsAttribute = false;

        if ( atype == Uml::at_Association || atype == Uml::at_Association_Self)
            m_classFieldType = PlainAssociation; // Plain == Self + untyped associations
        else if (atype == Uml::at_Aggregation)
            m_classFieldType = Aggregation;
        else if (atype == Uml::at_Composition)
            m_classFieldType = Composition;
    } else {
        m_classFieldType = Attribute;
        m_parentIsAttribute = true;
    }
}

QString CodeClassField::getListFieldClassName () {
    return getParentGenerator()->getListFieldClassName();
}

// Public attribute accessor methods
//

QString CodeClassField::getTypeName ( ) {

    if (parentIsAttribute())
    {
        UMLAttribute * at = (UMLAttribute*) getParentObject();
        return at->getTypeName();
    } else {
        UMLRole * role = (UMLRole*) getParentObject();
        if(fieldIsSingleValue()) {
            return getUMLObjectName(role->getObject());
        } else {
            return role->getName();
        }
    }
}

// get the type of object that will be added/removed from lists
// of objects (as per specification of associations)
QString CodeClassField::getListObjectType() {
    QString type = QString ("");
    if (!parentIsAttribute())
    {
        UMLRole * role = dynamic_cast<UMLRole*>(getParentObject());
        type = getUMLObjectName(role->getObject());
    }
    return type;
}

/**
 * Get the value of m_isAbstract
 * @return the value of m_isAbstract
 */
bool CodeClassField::parentIsAttribute ( ) {
    return m_parentIsAttribute;
    //  return (m_classFieldType == Attribute) ? true : false;
}

/**
 * Get the type of classfield this is.
 */
CodeClassField::ClassFieldType CodeClassField::getClassFieldType() {
    return m_classFieldType;
}

/**
 * Get the value of m_dialog
 * @return the value of m_dialog
 */
/*
CodeClassFieldDialog * CodeClassField::getDialog ( ) {
    return m_dialog;
}
*/

// methods like this _shouldnt_ be needed IF we properly did things thruought the code.
QString CodeClassField::getUMLObjectName(UMLObject *obj)
{
    return (obj!=0)?obj->getName():QString("NULL");
}

/**
 * Add a Method object to the m_methodVector List
 */
bool CodeClassField::addMethod ( CodeAccessorMethod * add_object ) {

    CodeAccessorMethod::AccessorType type = add_object->getType();

    if(findMethodByType(type))
        return false;
    /*
        // this wont work as the key for QMap needs to inherit from QObject
        if(m_methodMap->contains(type))
                return false; // return false, we already have some object with this tag in the list
        else
                m_methodMap->insert(type, add_object);
    */

    m_methodVector.append(add_object);
    return true;
}

/**
 * Remove a Method object from m_methodVector List
 */
bool CodeClassField::removeMethod ( CodeAccessorMethod * remove_object ) {
    // m_methodMap->erase(remove_object->getType());
    m_methodVector.removeRef(remove_object);
    getParentDocument()->removeTextBlock(remove_object);
    return true;
}

/**
 * Get the list of Method objects held by m_methodVector
 * @return QPtrList<CodeMethodBlock *> list of Method objects held by
 * m_methodVector
 */
CodeAccessorMethodList * CodeClassField::getMethodList ( ) {
    return &m_methodVector;
}

/** determine if we will *allow* methods to be viewable.
 * this flag is often used to toggle autogeneration of accessor
 * methods in the code class field.
 */
bool CodeClassField::getWriteOutMethods ()
{
    return m_writeOutMethods;
}

void CodeClassField::setWriteOutMethods ( bool val )
{
    m_writeOutMethods = val;
    updateContent();
}

/**
 * return the declaration statement for this class field object.
 * will be empty until this (abstract) class is inherited in elsewhere.
 */
CodeClassFieldDeclarationBlock * CodeClassField::getDeclarationCodeBlock( )
{
    return m_declCodeBlock;
}

CodeAccessorMethod * CodeClassField::newCodeAccessorMethod ( CodeAccessorMethod::AccessorType type ) {
    return getParentDocument()->newCodeAccessorMethod(this, type);
}

// Other methods
//

/**
 * load params from the appropriate XMI element node.
 */
void CodeClassField::loadFromXMI ( QDomElement & root ) {
    setAttributesFromNode(root);
}

/** set attributes of the node that represents this class
 * in the XMI document.
 */
void CodeClassField::setAttributesOnNode ( QDomDocument & doc, QDomElement & cfElem)
{

    // super class
    CodeParameter::setAttributesOnNode(doc,cfElem);

    // now set local attributes/fields
    cfElem.setAttribute("field_type",m_classFieldType);
    cfElem.setAttribute("listClassName",m_listClassName);
    cfElem.setAttribute("writeOutMethods",getWriteOutMethods()?"true":"false");

    // record tag on declaration codeblock
    // which we will store in its own separate child node block
    m_declCodeBlock->saveToXMI(doc, cfElem);

    // now record the tags on our accessormethods
    CodeAccessorMethodList * list = getMethodList ( );
    for(CodeAccessorMethod * method=list->first(); method; method=list->next())
    {
        method->saveToXMI(doc,cfElem);
    }

}

/** set the class attributes of this object from
 * the passed element node.
 */
void CodeClassField::setAttributesFromNode ( QDomElement & root) {

    // always disconnect
    getParentObject()->disconnect(this);

    // superclass call.. may reset the parent object
    CodeParameter::setAttributesFromNode(root);

    // make AFTER super-class call. This will reconnect to the parent
    // and re-check we have all needed child accessor methods and decl blocks
    initFields( );

    setWriteOutMethods(root.attribute("writeOutMethods","true") == "true" ? true : false);
    m_listClassName = root.attribute("listClassName","");
    m_classFieldType = (ClassFieldType) root.attribute("field_type","0").toInt();

    // load accessor methods now
    // by looking for our particular child element
    QDomNode node = root.firstChild();
    QDomElement element = node.toElement();
    while( !element.isNull() ) {
        QString tag = element.tagName();
        if( tag == "ccfdeclarationcodeblock" ) {
            m_declCodeBlock->loadFromXMI(element);
        } else
            if( tag == "codeaccessormethod" ) {
                int type = element.attribute("accessType","0").toInt();
                int role_id = element.attribute("role_id","-1").toInt();
                CodeAccessorMethod * method = findMethodByType((CodeAccessorMethod::AccessorType) type, role_id);
                if(method)
                    method->loadFromXMI(element);
                else
                    kError()<<"Cant load code accessor method for type:"<<type<<" which doesnt exist in this codeclassfield. Is XMI out-dated or corrupt?"<<endl;

            } else
                if( tag == "header" ) {
                    // this is treated in parent.. skip over here
                } else
                    kWarning()<<"ERROR: bad savefile? code classfield loadFromXMI got child element with unknown tag:"<<tag<<" ignoring node."<<endl;

        node = element.nextSibling();
        element = node.toElement();
    }

}

/**
 * Save the XMI representation of this object
 */
void CodeClassField::saveToXMI ( QDomDocument & doc, QDomElement & root ) {
    QDomElement docElement = doc.createElement( "codeclassfield" );

    setAttributesOnNode(doc, docElement);

    root.appendChild( docElement );
}

int CodeClassField::minimumListOccurances( ) {
    if (!parentIsAttribute())
    {
        UMLRole * role = dynamic_cast<UMLRole*>(getParentObject());
        QString multi = role->getMultiplicity();
        // ush. IF we had a multiplicty object, this would be much easier.
        if(!multi.isEmpty())
        {
            QString lowerBoundString = multi.remove(QRegExp("\\.\\.\\d+$"));
            if(!lowerBoundString.isEmpty() &&lowerBoundString.contains(QRegExp("^\\d+$")))
                return lowerBoundString.toInt();
        }

    }
    return 0;
}

int CodeClassField::maximumListOccurances( ) {
    if (!parentIsAttribute())
    {
        UMLRole * role = dynamic_cast<UMLRole*>(getParentObject());
        QString multi = role->getMultiplicity();
        // ush. IF we had a multiplicty object, this would be much easier.
        if(!multi.isEmpty())
        {
            QString upperBoundString = multi.section(QRegExp("(\\.\\.)"),1);
            if(!upperBoundString.isEmpty() && upperBoundString.contains(QRegExp("^\\d+$")))
                return upperBoundString.toInt();
            else
                return -1; // unbounded
        } else
            return -1; // unbounded

    }
    return 1;
}

QString CodeClassField::cleanName ( const QString &name ) {
    return getParentDocument()->cleanName(name);
}

QString CodeClassField::fixInitialStringDeclValue(QString value, const QString &type)
{
    // check for strings only
    if (!value.isEmpty() && type == "String") {
        if (!value.startsWith("\""))
            value.prepend("\"");
        if (!value.endsWith("\""))
            value.append("\"");
    }
    return value;
}

void CodeClassField::synchronize ()
{
    updateContent();
    CodeAccessorMethodList * list = getMethodList();
    for(CodeAccessorMethod * method=list->first(); method; method=list->next())
        method->syncToParent();

    if(m_declCodeBlock)
        m_declCodeBlock->syncToParent();
}

CodeAccessorMethod * CodeClassField::findMethodByType ( CodeAccessorMethod::AccessorType type, int role_id)
{
    //if we already know to which file this class was written/should be written, just return it.
    /*
        // argh. this wont work because "accessorType' doesnt inherit from QObject.
        if(m_methodMap->contains(type))
                return ((*m_methodMap)[type]);
        CodeAccessorMethod * obj = NULL;
    */
    if(role_id > 1 || role_id < 0)
    {
        for (CodeAccessorMethod * m = m_methodVector.first(); m ; m= m_methodVector.next())
            if( m->getType() == type)
                return m;
    } else {
        // ugh. forced into this underperforming algorithm because of bad association
        // design.
        for (CodeAccessorMethod * m = m_methodVector.first(); m ; m= m_methodVector.next())
        {
            UMLRole * role = dynamic_cast<UMLRole*>(m->getParentObject());
            if(!role)
                kError()<<"    FindMethodByType()  cant create role for method type:"<<m->getType()<<endl;
            if( role && m->getType() == type && role->getRole() == role_id)
                return m;
        }

    }

    return (CodeAccessorMethod *) NULL;
}

void CodeClassField::initAccessorMethods()
{

    // everything gets potential get/set method
    //if(!m_methodMap->contains(CodeAccessorMethod::GET))
    if(!findMethodByType(CodeAccessorMethod::GET))
    {
        CodeAccessorMethod * method = newCodeAccessorMethod (CodeAccessorMethod::GET);
        if(method)
        {
            method->setType(CodeAccessorMethod::GET);
            addMethod(method);
        }
    }

    if(!findMethodByType(CodeAccessorMethod::SET))
    {
        CodeAccessorMethod * method = newCodeAccessorMethod (CodeAccessorMethod::SET);
        if(method) {
            method->setType(CodeAccessorMethod::SET);
            addMethod(method);
        }
    }

    // add in the add,remove and list methods for things which are role based.
    // (and only used if the role specifies a 'list' type object
    if (!parentIsAttribute()) {

        if(!findMethodByType(CodeAccessorMethod::ADD))
        {
            CodeAccessorMethod * method = newCodeAccessorMethod (CodeAccessorMethod::ADD);
            if(method) {
                method->setType(CodeAccessorMethod::ADD);
                addMethod(method);
            }
        }

        if(!findMethodByType(CodeAccessorMethod::REMOVE))
        {
            CodeAccessorMethod * method = newCodeAccessorMethod (CodeAccessorMethod::REMOVE);
            if(method) {
                method->setType(CodeAccessorMethod::REMOVE);
                addMethod(method);
            }
        }

        if(!findMethodByType(CodeAccessorMethod::LIST))
        {
            CodeAccessorMethod * method = newCodeAccessorMethod (CodeAccessorMethod::LIST);
            if(method) {
                method->setType(CodeAccessorMethod::LIST);
                addMethod(method);
            }
        }

    }


}

void CodeClassField::updateContent()
{

    // Set properties for writing out the various methods derived from UMLRoles.
    // I suppose this could be supported under individual accessor method synctoparent
    // calls, but its going to happen again and again for many languages. Why not a catch
    // all here? -b.t.
    if (parentIsAttribute())
    {
        for ( CodeAccessorMethod *method = m_methodVector.first(); method;
                method = m_methodVector.next() )
            method->setWriteOutText( m_writeOutMethods );
        return;
    }
    UMLRole * role = dynamic_cast<UMLRole*>(getParentObject());
    Uml::Changeability_Type changeType = role->getChangeability();
    bool isSingleValue = fieldIsSingleValue();
    bool isEmptyRole = role->getName().isEmpty() ? true : false;

    for (CodeAccessorMethod * method = m_methodVector.first(); method; method=m_methodVector.next())
    {

        CodeAccessorMethod::AccessorType type = method->getType();

        // for role-based accessors, we DONT write ourselves out when
        // the name of the role is not defined OR when the global flag
        // to not show ANY methods is set.
        if(!m_writeOutMethods || isEmptyRole)
        {
            method->setWriteOutText(false);
            continue;
        }

        // not to change if no tag (dont know what it is, OR its not an AutoGenerated method
        if(method->getContentType() != CodeBlock::AutoGenerated)
            continue;

        // first off, some accessor methods wont appear if its a singleValue
        // role and vice-versa
        if(isSingleValue)
        {
            switch(type) {
            case CodeAccessorMethod::SET:
                // SET method true ONLY IF changeability is NOT Frozen
                if (changeType != Uml::chg_Frozen)
                    method->setWriteOutText(true);
                else
                    method->setWriteOutText(false);
                break;
            case CodeAccessorMethod::GET:
                method->setWriteOutText(true);
                break;
            case CodeAccessorMethod::ADD:
            case CodeAccessorMethod::REMOVE:
            case CodeAccessorMethod::LIST:
            default: // list/add/remove always false
                method->setWriteOutText(false);
                break;
            }
        }
        else
        {
            switch(type) {
                // get/set always false
            case CodeAccessorMethod::GET:
            case CodeAccessorMethod::SET:
                method->setWriteOutText(false);
                break;
            case CodeAccessorMethod::ADD:
                // ADD method true ONLY IF changeability is NOT Frozen
                if (changeType != Uml::chg_Frozen)
                    method->setWriteOutText(true);
                else
                    method->setWriteOutText(false);
                break;
            case CodeAccessorMethod::REMOVE:
                // Remove methods ONLY IF changeability is Changeable
                if (changeType == Uml::chg_Changeable)
                    method->setWriteOutText(true);
                else
                    method->setWriteOutText(false);
                break;
            case CodeAccessorMethod::LIST:
            default:
                method->setWriteOutText(true);
                break;
            }
        }
    }
}

// determine whether the parent object in this classfield indicates that it is
// a single variable or a List (Vector). One day this will be done correctly with special
// multiplicity object that we dont have to figure out what it means via regex.
bool CodeClassField::fieldIsSingleValue ( )
{
    // For the time being, all attributes ARE single values (yes,
    // I know this isnt always true, but we have to start somewhere.)
    if(parentIsAttribute())
        return true;

    UMLRole * role = dynamic_cast<UMLRole*>(getParentObject());
    if(!role)
        return true; // its really an attribute

    QString multi = role->getMultiplicity();

    if(multi.isEmpty() || multi.contains(QRegExp("^(0|1)$"))
            || multi.contains(QRegExp("^0\\.\\.1$")))
        return true;

    return false;
}

void CodeClassField::initFields ( ) {

    m_writeOutMethods = false;
    m_listClassName = QString ("");
    m_declCodeBlock = getParentDocument()->newDeclarationCodeBlock(this);

    m_methodVector.setAutoDelete(false);
    // m_methodMap = new QMap<CodeAccessorMethod::AccessorType, CodeAccessorMethod *>;

    initAccessorMethods();
    updateContent();

    connect(getParentObject(),SIGNAL(modified()),this,SIGNAL(modified())); // child objects will trigger off this signal

}

#include "codeclassfield.moc"
