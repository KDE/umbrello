/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "codeclassfield.h"

// app includes
#include "attribute.h"
#include "association.h"
#include "classifiercodedocument.h"
#include "codegenerator.h"
#include "codegenerators/codegenfactory.h"
#include "debug_utils.h"
#include "umlobject.h"
#include "umlrole.h"
#include "uml.h"

// qt includes
#include <QtCore/QRegExp>

CodeClassField::CodeClassField ( ClassifierCodeDocument * doc , UMLRole * role)
        : CodeParameter ( doc , (UMLObject*) role)
{
    setParentUMLObject(role);
    initFields(true);
}

CodeClassField::CodeClassField ( ClassifierCodeDocument * doc , UMLAttribute * attrib)
        : CodeParameter ( doc , (UMLObject*) attrib )
{
    setParentUMLObject(attrib);
    initFields(true);
}

CodeClassField::~CodeClassField ( )
{
    // remove methods from parent document
    Q_FOREACH( CodeAccessorMethod *m, m_methodVector )
    {
        getParentDocument()->removeTextBlock(m);
        m->forceRelease();
    }

    // clear the decl block from parent text block list too
    if(m_declCodeBlock)
    {
        getParentDocument()->removeTextBlock(m_declCodeBlock);
        m_declCodeBlock->forceRelease();
        delete m_declCodeBlock;
    }

}

/**
 * Set the parent UMLobject appropriately.
 */
void CodeClassField::setParentUMLObject (UMLObject * obj)
{
    UMLRole *role = dynamic_cast<UMLRole*>(obj);
    if(role) {
        UMLAssociation * parentAssoc = role->parentAssociation();
        Uml::AssociationType atype = parentAssoc->getAssocType();
        m_parentIsAttribute = false;

        if ( atype == Uml::AssociationType::Association || atype == Uml::AssociationType::Association_Self)
            m_classFieldType = PlainAssociation; // Plain == Self + untyped associations
        else if (atype == Uml::AssociationType::Aggregation)
            m_classFieldType = Aggregation;
        else if (atype == Uml::AssociationType::Composition)
            m_classFieldType = Composition;
    } else {
        m_classFieldType = Attribute;
        m_parentIsAttribute = true;
    }
}

QString CodeClassField::getTypeName ( )
{
    if (parentIsAttribute())
    {
        UMLAttribute * at = (UMLAttribute*) getParentObject();
        return at->getTypeName();
    } else {
        UMLRole * role = (UMLRole*) getParentObject();
        if(fieldIsSingleValue()) {
            return getUMLObjectName(role->object());
        } else {
            return role->name();
        }
    }
}

// get the type of object that will be added/removed from lists
// of objects (as per specification of associations)
QString CodeClassField::getListObjectType()
{
    if (!parentIsAttribute())
    {
        UMLRole * role = dynamic_cast<UMLRole*>(getParentObject());
        return getUMLObjectName(role->object());
    }
    return QString();
}

/**
 * Get the value of m_isAbstract.
 * @return the value of m_isAbstract
 */
bool CodeClassField::parentIsAttribute ( ) const
{
    return m_parentIsAttribute;
    //  return (m_classFieldType == Attribute) ? true : false;
}

/**
 * Get the type of classfield this is.
 */
CodeClassField::ClassFieldType CodeClassField::getClassFieldType() const
{
    return m_classFieldType;
}

/*
CodeClassFieldDialog * CodeClassField::getDialog ( )
{
    return m_dialog;
}
*/

// methods like this _shouldn't_ be needed IF we properly did things thruought the code.
QString CodeClassField::getUMLObjectName(UMLObject *obj)
{
    return (obj!=0)?obj->name():QString("NULL");
}

/**
 * Add a Method object to the m_methodVector List.
 */
bool CodeClassField::addMethod ( CodeAccessorMethod * add_object )
{
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
 * Remove a Method object from m_methodVector List.
 */
bool CodeClassField::removeMethod ( CodeAccessorMethod * remove_object )
{
    // m_methodMap->erase(remove_object->getType());
    m_methodVector.removeAll(remove_object);
    getParentDocument()->removeTextBlock(remove_object);
    return true;
}

/**
 * Get the list of Method objects held by m_methodVector.
 * @return QPtrList<CodeMethodBlock *> list of Method objects held by
 * m_methodVector
 */
CodeAccessorMethodList CodeClassField::getMethodList() const
{
    return m_methodVector;
}

/**
 * Determine if we will *allow* methods to be viewable.
 * this flag is often used to toggle autogeneration of accessor
 * methods in the code class field.
 */
bool CodeClassField::getWriteOutMethods () const
{
    return m_writeOutMethods;
}

/**
 * Determine if we will *allow* methods to be viewable.
 * this flag is often used to toggle autogeneration of accessor
 * methods in the code class field.
 */
void CodeClassField::setWriteOutMethods ( bool val )
{
    m_writeOutMethods = val;
    updateContent();
}

/**
 * Return the declaration statement for this class field object.
 * will be empty until this (abstract) class is inherited in elsewhere.
 */
CodeClassFieldDeclarationBlock * CodeClassField::getDeclarationCodeBlock( )
{
    return m_declCodeBlock;
}

/**
 * Load params from the appropriate XMI element node.
 */
void CodeClassField::loadFromXMI ( QDomElement & root )
{
    setAttributesFromNode(root);
}

/**
 * Set attributes of the node that represents this class
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
    Q_FOREACH( CodeAccessorMethod *method, m_methodVector )
    {
        method->saveToXMI(doc,cfElem);
    }
}

/**
 * Set the class attributes of this object from
 * the passed element node.
 */
void CodeClassField::setAttributesFromNode ( QDomElement & root)
{
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
                    uError()<<"Can not load code accessor method for type:"<<type<<" which does not exist in this codeclassfield. Is XMI out-dated or corrupt?";

            } else
                if( tag == "header" ) {
                    // this is treated in parent.. skip over here
                } else
                    uWarning()<<"ERROR: bad savefile? code classfield loadFromXMI got child element with unknown tag:"<<tag<<" ignoring node.";

        node = element.nextSibling();
        element = node.toElement();
    }
}

/**
 * Save the XMI representation of this object.
 */
void CodeClassField::saveToXMI ( QDomDocument & doc, QDomElement & root )
{
    QDomElement docElement = doc.createElement( "codeclassfield" );

    setAttributesOnNode(doc, docElement);

    root.appendChild( docElement );
}

/**
 * Find the minimum number of things that can occur in an association
 * If mistakenly called on attribute CF's the default value of is "0"
 * is returned. Similarly, if the association (role) CF doesn't have a multiplicty
 * 0 is returned.
 */
int CodeClassField::minimumListOccurances( )
{
    if (!parentIsAttribute())
    {
        UMLRole * role = dynamic_cast<UMLRole*>(getParentObject());
        QString multi = role->multiplicity();
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

/**
 * Find the maximum number of things that can occur in an association
 * If mistakenly called on attribute CF's the default value of is "1"
 * is returned. If the association (role) CF doesn't have a multiplicty
 * or has a "*" specified then '-1' (unbounded) is returned.
 */
int CodeClassField::maximumListOccurances( )
{
    if (!parentIsAttribute())
    {
        UMLRole * role = dynamic_cast<UMLRole*>(getParentObject());
        QString multi = role->multiplicity();
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

/**
 * A little utility method to make life easier for code document programmers
 */
QString CodeClassField::cleanName ( const QString &name )
{
    return getParentDocument()->cleanName(name);
}

/**
 * Another utility method to make life easier for code document programmers
 * this one fixes the initial declared value of string attributes so that if
 * it is empty or lacking quotations, it comes out as ""
 */
QString CodeClassField::fixInitialStringDeclValue(const QString& val, const QString &type)
{
    QString value = val;
    // check for strings only<F2>String value = val;
    if (!value.isEmpty() && type == "String") {
        if (!value.startsWith('"'))
            value.prepend('"');
        if (!value.endsWith('"'))
            value.append('"');
    }
    return value;
}

/**
 * Force the synchronization of the content (methods and declarations)
 * of this class field.
 */
void CodeClassField::synchronize ()
{
    updateContent();
    Q_FOREACH( CodeAccessorMethod *method, m_methodVector )
        method->syncToParent();

    if(m_declCodeBlock)
        m_declCodeBlock->syncToParent();
}

/**
 * Utility method to allow finding particular accessor method of this
 * code class field by its type identifier.
 */
CodeAccessorMethod * CodeClassField::findMethodByType ( CodeAccessorMethod::AccessorType type, int role_id)
{
    //if we already know to which file this class was written/should be written, just return it.
    /*
        // argh. this wont work because "accessorType' doesn't inherit from QObject.
        if(m_methodMap->contains(type))
                return ((*m_methodMap)[type]);
        CodeAccessorMethod * obj = NULL;
    */
    if(role_id > 1 || role_id < 0)
    {
        Q_FOREACH( CodeAccessorMethod *m, m_methodVector )
            if( m->getType() == type)
                return m;
    } else {
        // ugh. forced into this underperforming algorithm because of bad association
        // design.
        Q_FOREACH( CodeAccessorMethod *m, m_methodVector )
        {
            UMLRole * role = dynamic_cast<UMLRole*>(m->getParentObject());
            if(!role)
                uError()<<"    FindMethodByType()  cant create role for method type:"<<m->getType()<<endl;
            if( role && m->getType() == type && role->role() == role_id)
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
        CodeAccessorMethod * method = CodeGenFactory::newCodeAccessorMethod (getParentDocument(), this, CodeAccessorMethod::GET);
        if(method)
        {
            method->setType(CodeAccessorMethod::GET);
            addMethod(method);
        }
    }

    if(!findMethodByType(CodeAccessorMethod::SET))
    {
        CodeAccessorMethod * method = CodeGenFactory::newCodeAccessorMethod (getParentDocument(), this, CodeAccessorMethod::SET);
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
            CodeAccessorMethod * method = CodeGenFactory::newCodeAccessorMethod (getParentDocument(), this, CodeAccessorMethod::ADD);
            if(method) {
                method->setType(CodeAccessorMethod::ADD);
                addMethod(method);
            }
        }

        if(!findMethodByType(CodeAccessorMethod::REMOVE))
        {
            CodeAccessorMethod * method = CodeGenFactory::newCodeAccessorMethod (getParentDocument(), this, CodeAccessorMethod::REMOVE);
            if(method) {
                method->setType(CodeAccessorMethod::REMOVE);
                addMethod(method);
            }
        }

        if(!findMethodByType(CodeAccessorMethod::LIST))
        {
            CodeAccessorMethod * method = CodeGenFactory::newCodeAccessorMethod (getParentDocument(), this, CodeAccessorMethod::LIST);
            if(method) {
                method->setType(CodeAccessorMethod::LIST);
                addMethod(method);
            }
        }

    }
}

/**
 * Updates the status of the accessor methods
 * as to whether or not they should be written out.
 */
void CodeClassField::updateContent()
{
    // Set properties for writing out the various methods derived from UMLRoles.
    // I suppose this could be supported under individual accessor method synctoparent
    // calls, but it is going to happen again and again for many languages. Why not a catch
    // all here? -b.t.
    if (parentIsAttribute())
    {
        Q_FOREACH( CodeAccessorMethod *method, m_methodVector )
            method->setWriteOutText( m_writeOutMethods );
        return;
    }
    UMLRole * role = dynamic_cast<UMLRole*>(getParentObject());
    Uml::Changeability changeType = role->changeability();
    bool isSingleValue = fieldIsSingleValue();
    bool isEmptyRole = role->name().isEmpty() ? true : false;

    Q_FOREACH( CodeAccessorMethod *method, m_methodVector )
    {

        CodeAccessorMethod::AccessorType type = method->getType();

        // for role-based accessors, we DON'T write ourselves out when
        // the name of the role is not defined OR when the global flag
        // to not show ANY methods is set.
        if(!m_writeOutMethods || isEmptyRole)
        {
            method->setWriteOutText(false);
            continue;
        }

        // not to change if no tag (don't know what it is, OR it is not an AutoGenerated method
        if(method->contentType() != CodeBlock::AutoGenerated)
            continue;

        // first off, some accessor methods wont appear if it is a singleValue
        // role and vice-versa
        if(isSingleValue)
        {
            switch(type) {
            case CodeAccessorMethod::SET:
                // SET method true ONLY IF changeability is NOT Frozen
                if (changeType != Uml::Changeability::Frozen)
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
                if (changeType != Uml::Changeability::Frozen)
                    method->setWriteOutText(true);
                else
                    method->setWriteOutText(false);
                break;
            case CodeAccessorMethod::REMOVE:
                // Remove methods ONLY IF changeability is Changeable
                if (changeType == Uml::Changeability::Changeable)
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
// multiplicity object that we don't have to figure out what it means via regex.

/**
 * Determine whether the parent object in this classfield indicates that it is
 * a single variable or a List (Vector). One day this will be done correctly with special
 * multiplicity object.
 */
bool CodeClassField::fieldIsSingleValue ( )
{
    // For the time being, all attributes ARE single values (yes,
    // I know this isnt always true, but we have to start somewhere.)
    if(parentIsAttribute())
        return true;

    UMLRole * role = dynamic_cast<UMLRole*>(getParentObject());
    if(!role)
        return true; // it is really an attribute

    QString multi = role->multiplicity();

    if(multi.isEmpty() || multi.contains(QRegExp("^(0|1)$"))
            || multi.contains(QRegExp("^0\\.\\.1$")))
        return true;

    return false;
}

/**
 * Init class fields.
 */
void CodeClassField::initFields(bool inConstructor)
{
    m_writeOutMethods = false;
    m_listClassName.clear();
    m_declCodeBlock = NULL;

    // m_methodMap = new QMap<CodeAccessorMethod::AccessorType, CodeAccessorMethod *>;

    if (!inConstructor)
        finishInitialization();
}

/**
 * Finish off initializations of the object.
 * This is necessary as a separate method because we cannot call
 * virtual methods that are reimplemented in a language specific class
 * during our own construction (the own object is not finished being
 * constructed and therefore the C++ dispatch mechanism does not yet
 * work as expected.)
 */
void CodeClassField::finishInitialization()
{
    m_declCodeBlock = CodeGenFactory::newDeclarationCodeBlock(getParentDocument(), this);
    initAccessorMethods();
    updateContent();

    connect(getParentObject(),SIGNAL(modified()),this,SIGNAL(modified())); // child objects will trigger off this signal
}

#include "codeclassfield.moc"
