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
#include "operation.h"

// qt/kde includes
#include <qregexp.h>
#include <kdebug.h>
#include <klocale.h>

// app includes
#include "attribute.h"
#include "classifier.h"
#include "uml.h"
#include "umldoc.h"
#include "uniqueid.h"
#include "dialogs/umloperationdialog.h"

UMLOperation::UMLOperation(const UMLClassifier *parent, const QString& name,
                           Uml::IDType id, Uml::Visibility s, UMLObject *rt)
        : UMLClassifierListItem(parent, name, id)
{
    if (rt)
        m_returnId = UniqueID::gen();
    else
        m_returnId = Uml::id_None;
    m_pSecondary = rt;
    m_Vis = s;
    m_BaseType = Uml::ot_Operation;
    m_bConst = false;
}

UMLOperation::UMLOperation(const UMLClassifier * parent)
        : UMLClassifierListItem (parent)
{
    m_BaseType = Uml::ot_Operation;
    m_bConst = false;
}

UMLOperation::~UMLOperation() {
}

void UMLOperation::setType(UMLObject *type) {
    UMLClassifierListItem::setType(type);
    if (m_returnId == Uml::id_None)
        m_returnId = UniqueID::gen();
}

void UMLOperation::moveParmLeft(UMLAttribute * a) {
    if (a == NULL) {
        kDebug() << "UMLOperation::moveParmLeft called on NULL attribute"
        << endl;
        return;
    }
    kDebug() << "UMLOperation::moveParmLeft(" << a->getName() << ") called"
    << endl;
    disconnect(a,SIGNAL(modified()),this,SIGNAL(modified()));
    int idx;
    if ( (idx=m_List.find( a )) == -1 ) {
        kDebug() << "Error move parm left " << a->getName() << endl;
        return;
    }
    if ( idx == 0 )
        return;
    m_List.remove( a );
    m_List.insert( idx-1, a );
}

void UMLOperation::moveParmRight(UMLAttribute * a) {
    if (a == NULL) {
        kDebug() << "UMLOperation::moveParmRight called on NULL attribute"
        << endl;
        return;
    }
    kDebug() << "UMLOperation::moveParmRight(" << a->getName() << ") called"
    << endl;
    disconnect(a,SIGNAL(modified()),this,SIGNAL(modified()));
    int idx;
    if ( (idx=m_List.find( a )) == -1 ) {
        kDebug() << "Error move parm right " << a->getName() << endl;
        return;
    }
    int count = m_List.count();
    if ( idx == count-1 )
        return;
    m_List.remove( a );
    m_List.insert( idx+1, a );
}

void UMLOperation::removeParm(UMLAttribute * a, bool emitModifiedSignal /* =true */) {
    if (a == NULL) {
        kDebug() << "UMLOperation::removeParm called on NULL attribute"
        << endl;
        return;
    }
    kDebug() << "UMLOperation::removeParm(" << a->getName() << ") called"
    << endl;
    disconnect(a,SIGNAL(modified()),this,SIGNAL(modified()));
    if(!m_List.remove(a))
        kDebug() << "Error removing parm " << a->getName() << endl;

    if (emitModifiedSignal)
        emit modified();
}

UMLAttribute* UMLOperation::findParm(const QString &name) {
    UMLAttribute * obj=0;
    for (obj = m_List.first(); obj; obj = m_List.next()) {
        if (obj->getName() == name)
            return obj;
    }
    return 0;
}

QString UMLOperation::toString(Uml::Signature_Type sig) {
    QString s = "";

    if(sig == Uml::st_ShowSig || sig == Uml::st_NoSig)
          s = m_Vis.toString(true) + ' ';

    s += getName();
    Uml::Programming_Language pl = UMLApp::app()->getActiveLanguage();
    bool parameterlessOpNeedsParentheses = (pl != Uml::pl_Pascal && pl != Uml::pl_Ada);

    if (sig == Uml::st_NoSig || sig == Uml::st_NoSigNoVis) {
        if (parameterlessOpNeedsParentheses)
            s.append("()");
        return s;
    }
    int last = m_List.count();
    if (last) {
        s.append("(");
        int i = 0;
        for (UMLAttribute *param = m_List.first(); param; param = m_List.next()) {
            i++;
            s.append(param->toString(Uml::st_SigNoVis));
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
        UMLPackage *retVisibility = retType->getUMLPackage();
        if (retVisibility != ownParent && retVisibility != ownParent->getUMLPackage())
            returnType = retType->getFullyQualifiedName();
        else
            returnType = retType->getName();
    }
    if (returnType.length() > 0 && returnType != "void") {
        s.append(" : ");

        if (returnType.startsWith("virtual ")) {
            s += returnType.mid(8);
        } else {
            s += returnType;
        }
    }
    return s;
}

void UMLOperation::addParm(UMLAttribute *parameter, int position) {
    if( position >= 0 && position <= (int)m_List.count() )
        m_List.insert(position,parameter);
    else
        m_List.append( parameter );
    UMLObject::emitModified();
    connect(parameter,SIGNAL(modified()),this,SIGNAL(modified()));
}

QString UMLOperation::getUniqueParameterName() {
    QString currentName = i18n("new_parameter");
    QString name = currentName;
    for (int number = 1; findParm(name); number++) {
        name = currentName + '_' + QString::number(number);
    }
    return name;
}

bool UMLOperation::operator==( UMLOperation & rhs ) {
    if( this == &rhs )
        return true;

    if( !UMLObject::operator==( rhs ) )
        return false;

    if( getTypeName() != rhs.getTypeName() )
        return false;

    if( m_List.count() != rhs.m_List.count() )
        return false;

    if(!(m_List == rhs.m_List))
        return false;

    return true;
}

void UMLOperation::copyInto(UMLOperation *rhs) const
{
    UMLClassifierListItem::copyInto(rhs);

    m_List.copyInto(&(rhs->m_List));
}

UMLObject* UMLOperation::clone() const
{
    //FIXME: The new operation should be slaved to the NEW parent not the old.
    UMLOperation *clone = new UMLOperation( static_cast<UMLClassifier*>(parent()) );
    copyInto(clone);

    return clone;
}

bool UMLOperation::resolveRef() {
    bool overallSuccess = UMLObject::resolveRef();
    // See remark on iteration style in UMLClassifier::resolveRef()
    for (UMLAttributeListIt ait(m_List); ait.current(); ++ait) {
        UMLAttribute *pAtt = ait.current();
        if (! pAtt->resolveRef())
            overallSuccess = false;
    }
    return overallSuccess;
}

bool UMLOperation::isConstructorOperation() {
    // if an operation has the stereotype constructor
    // return true
    QString strConstructor ("constructor");
    if (getStereotype() == strConstructor)
        return true;

    UMLClassifier * c = static_cast<UMLClassifier*>(this->parent());
    QString cName = c->getName();
    QString opName = getName();
    // It's a constructor operation if the operation name
    // matches that of the parent classifier.
    return (cName == opName);
}

bool UMLOperation::isDestructorOperation() {
    if (getStereotype() == "destructor")
        return true;
    UMLClassifier * c = static_cast<UMLClassifier*>(this->parent());

    QString cName = c->getName();
    QString opName = getName();
    // Special support for C++ syntax:
    // It's a destructor operation if the operation name begins
    // with "~" followed by the name of the parent classifier.
    if (! opName.startsWith("~"))
        return false;
    opName.remove( QRegExp("^~\\s*") );
    return (cName == opName);
}

bool UMLOperation::isLifeOperation() {
    return (isConstructorOperation() || isDestructorOperation());
}

void UMLOperation::setConst(bool b) {
    m_bConst = b;
}

bool UMLOperation::getConst() const {
    return m_bConst;
}

bool UMLOperation::showPropertiesDialog(QWidget* parent) {
    UMLOperationDialog dialog(parent, this);
    return dialog.exec();
}

void UMLOperation::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement operationElement = UMLObject::save("UML:Operation", qDoc);
    operationElement.setAttribute( "isQuery", m_bConst ? "true" : "false" );
    QDomElement featureElement = qDoc.createElement( "UML:BehavioralFeature.parameter" );
    if (m_pSecondary) {
        QDomElement retElement = qDoc.createElement("UML:Parameter");
        if (m_returnId == Uml::id_None) {
            kDebug() << "UMLOperation::saveToXMI(" << m_Name
                << "): m_returnId is not set, setting it now." << endl;
            m_returnId = UniqueID::gen();
        }
        retElement.setAttribute( "xmi.id", ID2STR(m_returnId) );
        retElement.setAttribute( "type", ID2STR(m_pSecondary->getID()) );
        retElement.setAttribute( "kind", "return" );
        featureElement.appendChild( retElement );
    } else {
        kDebug() << "UMLOperation::saveToXMI: m_SecondaryId is "
        << m_SecondaryId << endl;
    }
    //save each attribute here, type different
    UMLAttribute* pAtt = 0;
    for( pAtt = m_List.first(); pAtt != 0; pAtt = m_List.next() ) {
        QDomElement attElement = pAtt->UMLObject::save("UML:Parameter", qDoc);
        UMLClassifier *attrType = pAtt->getType();
        if (attrType) {
            attElement.setAttribute( "type", ID2STR(attrType->getID()) );
        } else {
            attElement.setAttribute( "type", pAtt -> getTypeName() );
        }
        attElement.setAttribute( "value", pAtt -> getInitialValue() );

        Uml::Parameter_Direction kind = pAtt->getParmKind();
        if (kind == Uml::pd_Out)
            attElement.setAttribute("kind", "out");
        else if (kind == Uml::pd_InOut)
            attElement.setAttribute("kind", "inout");
        // The default for the parameter kind is "in".

        featureElement.appendChild( attElement );
    }
    if (featureElement.hasChildNodes())
        operationElement.appendChild( featureElement );
    qElement.appendChild( operationElement );
}

bool UMLOperation::load( QDomElement & element ) {
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
    while( !attElement.isNull() ) {
        QString tag = attElement.tagName();
        if (Uml::tagEq(tag, "BehavioralFeature.parameter")) {
            if (! load(attElement))
                return false;
        } else if (Uml::tagEq(tag, "Parameter")) {
            QString kind = attElement.attribute("kind", "");
            if (kind.isEmpty()) {
                // Perhaps the kind is stored in a child node:
                for (QDomNode n = attElement.firstChild(); !n.isNull(); n = n.nextSibling()) {
                    if (n.isComment())
                        continue;
                    QDomElement tempElement = n.toElement();
                    QString tag = tempElement.tagName();
                    if (!Uml::tagEq(tag, "kind"))
                        continue;
                    kind = tempElement.attribute( "xmi.value", "" );
                    break;
                }
                if (kind.isEmpty()) {
                    // kDebug() << "UMLOperation::load(" << m_Name << "): "
                    //  << "cannot find kind, using default \"in\"." << endl;
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
                        if (!Uml::tagEq(tag, "type")) {
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
                        kError() << "UMLOperation::load(" << m_Name << "): "
                        << "cannot find return type." << endl;
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
    }//end while
    return true;
}


#include "operation.moc"
