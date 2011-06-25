/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "operation.h"

// app includes
#include "attribute.h"
#include "classifier.h"
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "uniqueid.h"
#include "umloperationdialog.h"
#include "codegenerator.h"
#include "codedocument.h"
#include "codeblock.h"

// kde includes
#include <klocale.h>

// qt includes
#include <QtCore/QRegExp>

/**
 * Constructs an UMLOperation.
 * Not intended for general use: The operation is not tied in with
 * umbrello's Qt signalling for object creation.
 * If you want to create an Operation use the method in UMLDoc instead.
 *
 * @param parent    the parent to this operation
 * @param name      the name of the operation
 * @param id        the id of the operation
 * @param s         the visibility of the operation
 * @param rt        the return type of the operation
 */
UMLOperation::UMLOperation(UMLClassifier *parent, const QString& name,
                           Uml::IDType id, Uml::Visibility s, UMLObject *rt)
  : UMLClassifierListItem(parent, name, id)
{
    if (rt)
        m_returnId = UniqueID::gen();
    else
        m_returnId = Uml::id_None;
    m_pSecondary = rt;
    m_Vis = s;
    m_BaseType = UMLObject::ot_Operation;
    m_bConst = false;
    m_Code.clear();
}

/**
 * Constructs an UMLOperation.
 * Not intended for general use: The operation is not tied in with
 * umbrello's Qt signalling for object creation.
 * If you want to create an Operation use the method in UMLDoc instead.
 *
 * @param parent    the parent to this operation
 */
UMLOperation::UMLOperation(UMLClassifier * parent)
  : UMLClassifierListItem (parent)
{
    m_BaseType = UMLObject::ot_Operation;
    m_bConst = false;
    m_Code.clear();
}

/**
 * Destructor.
 */
UMLOperation::~UMLOperation()
{
}

/**
 * Reimplement method from UMLClassifierListItem.
 *
 * @param type      pointer to the type object
 */
void UMLOperation::setType(UMLObject* type)
{
    UMLClassifierListItem::setType(type);
    if (m_returnId == Uml::id_None)
        m_returnId = UniqueID::gen();
}

/**
 * Move a parameter one position to the left.
 *
 * @param a   the parameter to move
 */
void UMLOperation::moveParmLeft(UMLAttribute * a)
{
    if (a == NULL) {
        uDebug() << "called on NULL attribute";
        return;
    }
    uDebug() << "called for " << a->name();
    disconnect(a,SIGNAL(modified()),this,SIGNAL(modified()));
    int idx;
    if ( (idx=m_List.indexOf( a )) == -1 ) {
        uDebug() << "Error move parm left " << a->name();
        return;
    }
    if ( idx == 0 )
        return;
    m_List.removeAll( a );
    m_List.insert( idx-1, a );
}

/**
 * Move a parameter one position to the right.
 *
 * @param a   the parameter to move
 */
void UMLOperation::moveParmRight(UMLAttribute * a)
{
    if (a == NULL) {
        uDebug() << "called on NULL attribute";
        return;
    }
    uDebug() << "called for " << a->name();
    disconnect(a,SIGNAL(modified()),this,SIGNAL(modified()));
    int idx;
    if ( (idx=m_List.indexOf( a )) == -1 ) {
        uDebug() << "Error move parm right " << a->name();
        return;
    }
    int count = m_List.count();
    if ( idx == count-1 )
        return;
    m_List.removeAll( a );
    m_List.insert( idx+1, a );
}

/**
 * Remove a parameter from the operation.
 *
 * @param a         the parameter to remove
 * @param emitModifiedSignal  whether to emit the "modified" signal
 *                  which creates an entry in the Undo stack for the
 *                  removal, default: true
 */
void UMLOperation::removeParm(UMLAttribute * a, bool emitModifiedSignal /* =true */)
{
    if (a == NULL) {
        uDebug() << "called on NULL attribute";
        return;
    }
    uDebug() << "called for " << a->name();
    disconnect(a,SIGNAL(modified()),this,SIGNAL(modified()));
    if(!m_List.removeAll(a))
        uDebug() << "Error removing parm " << a->name();

    if (emitModifiedSignal)
        emit modified();
}

/**
 * Returns a list of parameters.
 *
 * @return a list of the parameters in the operation
 */
UMLAttributeList UMLOperation::getParmList() const
{
    return m_List;
}

/**
 * Finds a parameter of the operation.
 *
 * @param name      the parameter name to search for
 * @return          the found parameter, 0 if not found
 */
UMLAttribute* UMLOperation::findParm(const QString &name)
{
    UMLAttribute * obj=0;
    foreach (obj , m_List ) {
        if (obj->name() == name)
            return obj;
    }
    return 0;
}

/**
 * Returns a string representation of the operation.
 *
 * @param sig       what type of operation string to show
 * @return          the string representation of the operation
 */
QString UMLOperation::toString(Uml::SignatureType sig)
{
    QString s;

    if (sig == Uml::SignatureType::ShowSig || sig == Uml::SignatureType::NoSig)
          s = m_Vis.toString(true) + ' ';

    s += name();
    Uml::ProgrammingLanguage pl = UMLApp::app()->activeLanguage();
    bool parameterlessOpNeedsParentheses =
        (pl != Uml::ProgrammingLanguage::Pascal && pl != Uml::ProgrammingLanguage::Ada);

    if (sig == Uml::SignatureType::NoSig || sig == Uml::SignatureType::NoSigNoVis) {
        if (parameterlessOpNeedsParentheses)
            s.append("()");
        return s;
    }
    int last = m_List.count();
    if (last) {
        s.append("(");
        int i = 0;
        foreach (UMLAttribute *param , m_List ) {
            i++;
            s.append(param->toString(Uml::SignatureType::SigNoVis));
            if (i < last)
                s.append(", ");
        }
        s.append(")");
    } else if (parameterlessOpNeedsParentheses) {
        s.append("()");
    }
    UMLClassifier *ownParent = static_cast<UMLClassifier*>(parent());
    QString returnType;
    UMLClassifier *retType = UMLClassifierListItem::getType();
    if (retType) {
        UMLPackage *retVisibility = retType->umlPackage();
        if (retVisibility != ownParent && retVisibility != ownParent->umlPackage())
            returnType = retType->fullyQualifiedName();
        else
            returnType = retType->name();
    }
    if (returnType.length() > 0 && returnType != "void") {
        s.append(" : ");

        if (returnType.startsWith(QLatin1String("virtual "))) {
            s += returnType.mid(8);
        } else {
            s += returnType;
        }
    }
    return s;
}

/**
 * Add a parameter to the operation.
 *
 * @param parameter the parameter to add
 * @param position  the position in the parameter list.
 *                  If position = -1 the parameter will be
 *                  appended to the list.
 */
void UMLOperation::addParm(UMLAttribute *parameter, int position)
{
    if( position >= 0 && position <= (int)m_List.count() )
        m_List.insert(position,parameter);
    else
        m_List.append( parameter );
    UMLObject::emitModified();
    connect(parameter,SIGNAL(modified()),this,SIGNAL(modified()));
}

/**
 * Returns an unused parameter name for a new parameter.
 */
QString UMLOperation::getUniqueParameterName()
{
    QString currentName = i18n("new_parameter");
    QString name = currentName;
    for (int number = 1; findParm(name); ++number) {
        name = currentName + '_' + QString::number(number);
    }
    return name;
}

/**
 * Overloaded '==' operator.
 */
bool UMLOperation::operator==(const UMLOperation & rhs) const
{
    if ( this == &rhs )
        return true;

    if ( !UMLObject::operator==( rhs ) )
        return false;

    if ( getTypeName() != rhs.getTypeName() )
        return false;

    if ( m_List.count() != rhs.m_List.count() )
        return false;

    if (!(m_List == rhs.m_List))
        return false;

    return true;
}

/**
 * Copy the internal presentation of this object into the new
 * object.
 */
void UMLOperation::copyInto(UMLObject *lhs) const
{
    UMLOperation *target = static_cast<UMLOperation*>(lhs);

    UMLClassifierListItem::copyInto(target);

    m_List.copyInto(&(target->m_List));
}

/**
 * Make a clone of this object.
 */
UMLObject* UMLOperation::clone() const
{
    //FIXME: The new operation should be slaved to the NEW parent not the old.
    UMLOperation *clone = new UMLOperation( static_cast<UMLClassifier*>(parent()) );
    copyInto(clone);

    return clone;
}

/**
 * Calls resolveRef() on all parameters.
 * Needs to be called after all UML objects are loaded from file.
 *
 * @return  true for success
 */
bool UMLOperation::resolveRef()
{
    bool overallSuccess = UMLObject::resolveRef();
    // See remark on iteration style in UMLClassifier::resolveRef()
    foreach (UMLAttribute* pAtt, m_List ) {
        if (! pAtt->resolveRef())
            overallSuccess = false;
    }
    return overallSuccess;
}

/**
 * Returns whether this operation is a constructor.
 *
 * @return  true if this operation is a constructor
 */
bool UMLOperation::isConstructorOperation()
{
    // if an operation has the stereotype constructor
    // return true
    QString strConstructor ("constructor");
    if (stereotype() == strConstructor)
        return true;

    UMLClassifier * c = static_cast<UMLClassifier*>(this->parent());
    QString cName = c->name();
    QString opName = name();
    // It's a constructor operation if the operation name
    // matches that of the parent classifier.
    return (cName == opName);
}

/**
 * Returns whether this operation is a destructor.
 *
 * @return  true if this operation is a destructor
 */
bool UMLOperation::isDestructorOperation()
{
    if (stereotype() == "destructor")
        return true;
    UMLClassifier * c = static_cast<UMLClassifier*>(this->parent());

    QString cName = c->name();
    QString opName = name();
    // Special support for C++ syntax:
    // It's a destructor operation if the operation name begins
    // with "~" followed by the name of the parent classifier.
    if (! opName.startsWith('~'))
        return false;
    opName.remove( QRegExp("^~\\s*") );
    return (cName == opName);
}

/**
 * Shortcut for (isConstructorOperation() || isDestructorOperation()).
 *
 * @return  true if this operation is a constructor or destructor
 */
bool UMLOperation::isLifeOperation()
{
    return (isConstructorOperation() || isDestructorOperation());
}

/**
 * Sets whether this operation is a query (C++ "const").
 */
void UMLOperation::setConst(bool b)
{
    m_bConst = b;
}

/**
 * Returns whether this operation is a query (C++ "const").
 */
bool UMLOperation::getConst() const
{
    return m_bConst;
}

/**
 * Display the properties configuration dialog for the template.
 *
 * @param parent   the parent for the dialog
 */
bool UMLOperation::showPropertiesDialog(QWidget* parent)
{
    UMLOperationDialog dialog(parent, this);
    return dialog.exec();
}

/**
 * Sets the source code for this operation.
 *
 * @param code  the body of this operation
 */
void UMLOperation::setSourceCode(const QString& code)
{
    m_Code = code;
}

/**
 * Returns the source code for this operation.
 */
QString UMLOperation::getSourceCode() const
{
    return m_Code;
}

/**
 * Saves to the <UML:Operation> XMI element.
 */
void UMLOperation::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement operationElement = UMLObject::save("UML:Operation", qDoc);
    operationElement.setAttribute( "isQuery", m_bConst ? "true" : "false" );
    QDomElement featureElement = qDoc.createElement( "UML:BehavioralFeature.parameter" );
    if (m_pSecondary) {
        QDomElement retElement = qDoc.createElement("UML:Parameter");
        if (m_returnId == Uml::id_None) {
            uDebug() << name() << ": m_returnId is not set, setting it now.";
            m_returnId = UniqueID::gen();
        }
        retElement.setAttribute( "xmi.id", ID2STR(m_returnId) );
        retElement.setAttribute( "type", ID2STR(m_pSecondary->id()) );
        retElement.setAttribute( "kind", "return" );
        featureElement.appendChild( retElement );
    } else {
        uDebug() << "m_SecondaryId is " << m_SecondaryId;
    }

    //save each attribute here, type different
    foreach( UMLAttribute* pAtt , m_List ) {
        QDomElement attElement = pAtt->UMLObject::save("UML:Parameter", qDoc);
        UMLClassifier *attrType = pAtt->getType();
        if (attrType) {
            attElement.setAttribute( "type", ID2STR(attrType->id()) );
        } else {
            attElement.setAttribute( "type", pAtt->getTypeName() );
        }
        attElement.setAttribute( "value", pAtt->getInitialValue() );

        Uml::Parameter_Direction kind = pAtt->getParmKind();
        if (kind == Uml::pd_Out)
            attElement.setAttribute("kind", "out");
        else if (kind == Uml::pd_InOut)
            attElement.setAttribute("kind", "inout");
        // The default for the parameter kind is "in".

        featureElement.appendChild( attElement );
    }
    if (featureElement.hasChildNodes()) {
        operationElement.appendChild( featureElement );
    }
    qElement.appendChild( operationElement );
}

/**
 * Loads a <UML:Operation> XMI element.
 */
bool UMLOperation::load( QDomElement & element )
{
    m_SecondaryId = element.attribute( "type", "" );
    QString isQuery = element.attribute( "isQuery", "" );
    if (!isQuery.isEmpty()) {
        // We need this extra test for isEmpty() because load() might have been
        // called again by the processing for BehavioralFeature.parameter (see below)
        m_bConst = (isQuery == "true");
    }
    QDomNode node = element.firstChild();
    if (node.isComment())
        node = node.nextSibling();
    QDomElement attElement = node.toElement();
    while ( !attElement.isNull() ) {
        QString tag = attElement.tagName();
        if (UMLDoc::tagEq(tag, "BehavioralFeature.parameter")) {
            if (! load(attElement))
                return false;
        } else if (UMLDoc::tagEq(tag, "Parameter")) {
            QString kind = attElement.attribute("kind", "");
            if (kind.isEmpty()) {
                // Perhaps the kind is stored in a child node:
                for (QDomNode n = attElement.firstChild(); !n.isNull(); n = n.nextSibling()) {
                    if (n.isComment())
                        continue;
                    QDomElement tempElement = n.toElement();
                    QString tag = tempElement.tagName();
                    if (!UMLDoc::tagEq(tag, "kind"))
                        continue;
                    kind = tempElement.attribute( "xmi.value", "" );
                    break;
                }
                if (kind.isEmpty()) {
                    kind = "in";
                }
            }
            if (kind == "return") {
                QString returnId = attElement.attribute("xmi.id", "");
                if (!returnId.isEmpty())
                    m_returnId = STR2ID(returnId);
                m_SecondaryId = attElement.attribute( "type", "" );
                if (m_SecondaryId.isEmpty()) {
                    // Perhaps the type is stored in a child node:
                    QDomNode node = attElement.firstChild();
                    while (!node.isNull()) {
                        if (node.isComment()) {
                            node = node.nextSibling();
                            continue;
                        }
                        QDomElement tempElement = node.toElement();
                        QString tag = tempElement.tagName();
                        if (!UMLDoc::tagEq(tag, "type")) {
                            node = node.nextSibling();
                            continue;
                        }
                        m_SecondaryId = tempElement.attribute( "xmi.id", "" );
                        if (m_SecondaryId.isEmpty())
                            m_SecondaryId = tempElement.attribute( "xmi.idref", "" );
                        if (m_SecondaryId.isEmpty()) {
                            QDomNode inner = node.firstChild();
                            QDomElement tmpElem = inner.toElement();
                            m_SecondaryId = tmpElem.attribute( "xmi.id", "" );
                            if (m_SecondaryId.isEmpty())
                                m_SecondaryId = tmpElem.attribute( "xmi.idref", "" );
                        }
                        break;
                    }
                    if (m_SecondaryId.isEmpty()) {
                        uError() << name() << ": cannot find return type.";
                    }
                }
                // Use deferred xmi.id resolution.
                m_pSecondary = NULL;
            } else {
                UMLAttribute * pAtt = new UMLAttribute( this );
                if( !pAtt->loadFromXMI(attElement) ) {
                    delete pAtt;
                    return false;
                }
                if (kind == "out")
                    pAtt->setParmKind(Uml::pd_Out);
                else if (kind == "inout")
                    pAtt->setParmKind(Uml::pd_InOut);
                else
                    pAtt->setParmKind(Uml::pd_In);
                m_List.append( pAtt );
            }
        }
        node = node.nextSibling();
        if (node.isComment())
            node = node.nextSibling();
        attElement = node.toElement();

        // loading the source code which was entered in the 'classpropdlg' dialog is not done
        // with the following code, because there is no CodeDocument.
        /*
        CodeGenerator* codegen = UMLApp::app()->getGenerator();
        if (codegen) {
            uDebug() << "CodeDocument searching with id=" << ID2STR(UMLObject::getID());
            CodeDocument* codeDoc = codegen->findCodeDocumentByID(ID2STR(UMLObject::getID()));
            if (codeDoc) {
                uDebug() << "CodeDocument found:\n" << codeDoc;
            }
        }
        */
        // it is done in the code generators by calling CodeGenerator::loadFromXMI(...).

    }//end while
    return true;
}


#include "operation.moc"
