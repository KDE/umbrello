/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
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
#include "dialogs/umloperationdialog.h"

UMLOperation::UMLOperation(const UMLClassifier *parent, QString Name, int id,
			   Uml::Scope s, QString rt)
    : UMLClassifierListItem(parent, Name, id)
{
	setTypeName( rt );
	m_Scope = s;
	m_BaseType = Uml::ot_Operation;
	m_List.setAutoDelete(false);
}

UMLOperation::UMLOperation(const UMLClassifier * parent)
    : UMLClassifierListItem (parent)
{
	m_BaseType = Uml::ot_Operation;
	m_List.setAutoDelete(true);
}

UMLOperation::~UMLOperation() {
}

UMLAttribute * UMLOperation::addParm(QString type, QString name, QString initialValue,
				     QString doc, Uml::Parameter_Direction kind) {
	// make the new parameter (attribute) public, just to be safe
	UMLDoc *umldoc = UMLApp::app()->getDocument();
	UMLAttribute * a = new UMLAttribute(this, name, umldoc->getUniqueID(), Uml::Public, type);
	a -> setDoc(doc);
	a -> setInitialValue(initialValue);
	a -> setParmKind(kind);
	addParm(a);
	return a;
}

void UMLOperation::removeParm(UMLAttribute * a) {
	if (a == NULL) {
		kdDebug() << "UMLOperation::removeParm called on NULL attribute"
			  << endl;
		return;
	}
	kdDebug() << "UMLOperation::removeParm(" << a->getName() << ") called"
		  << endl;
	disconnect(a,SIGNAL(modified()),this,SIGNAL(modified()));
	if(!m_List.remove(a))
		kdDebug() << "Error removing parm " << a->getName() << endl;

	emit modified();
}

UMLAttribute* UMLOperation::findParm(QString name) {
	UMLAttribute * obj=0;
	for (obj = m_List.first(); obj; obj = m_List.next()) {
		if (obj->getName() == name)
			return obj;
	}
	return 0;
}

QString UMLOperation::toString(Uml::Signature_Type sig) {
	QString s = "";

	if(sig == Uml::st_ShowSig || sig == Uml::st_NoSig) {
		if(m_Scope == Uml::Public)
			s = "+ ";
		else if(m_Scope == Uml::Private)
			s = "- ";
		else if(m_Scope == Uml::Protected)
			s = "# ";
	}

	s += getName();
	s.append("(");

	if(sig == Uml::st_NoSig || sig == Uml::st_NoSigNoScope) {
		s.append(")");
		return s;
	}
	UMLAttribute * obj=0;
	int last = m_List.count(), i = 0;
	for(obj=m_List.first();obj != 0;obj=m_List.next()) {
		i++;
		s.append(obj -> toString(Uml::st_SigNoScope));
		if(i < last)
			s.append(", ");
	}
	s.append(")");
	QString returnType = UMLClassifierListItem::getTypeName();
	if (returnType.length() > 0 ) {
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
	emit modified();
	connect(parameter,SIGNAL(modified()),this,SIGNAL(modified()));
}

QString UMLOperation::getUniqueParameterName() {
	QString currentName = i18n("new_parameter");
	QString name = currentName;
	for (int number = 1; findParm(name); number++) {
	        name = currentName + "_" + QString::number(number);
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
	// TODO Why is this a UMLClassifier?
	// -- Huh? I don't understand this TODO   --okellogg
	UMLOperation *clone = new UMLOperation( (UMLClassifier *) parent());
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

void UMLOperation::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement operationElement = UMLObject::save("UML:Operation", qDoc);
	if (m_pSecondary) {
		operationElement.setAttribute( "type", m_pSecondary->getID() );
	} else {
		operationElement.setAttribute( "type", m_SecondaryId );
	}
	//save each attribute here, type different
	UMLAttribute* pAtt = 0;
	for( pAtt = m_List.first(); pAtt != 0; pAtt = m_List.next() ) {
		QDomElement attElement = pAtt->UMLObject::save("UML:Parameter", qDoc);
		UMLClassifier *attrType = pAtt->getType();
		if (attrType) {
			attElement.setAttribute( "type", attrType->getID() );
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

		operationElement.appendChild( attElement );
	}
	qElement.appendChild( operationElement );
}

bool UMLOperation::load( QDomElement & element ) {
	QString type = element.attribute( "type", "" );
	if (!type.isEmpty()) {
		if (type.contains( QRegExp("\\D") )) {
			m_SecondaryId = type;  // defer type resolution
		} else {
			UMLDoc *pDoc = UMLApp::app()->getDocument();
			m_pSecondary = pDoc->findUMLObject( type.toInt() );
			if (m_pSecondary == NULL) {
				kdError() << "UMLOperation::load: Cannot find UML object"
					  << " for type " << type << endl;
				UMLClassifierListItem::setTypeName( "void" );
			}
		}
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
			UMLAttribute * pAtt = new UMLAttribute( this );
			if( !pAtt->loadFromXMI(attElement) ) {
				delete pAtt;
				return false;
			}
			QString kind = attElement.attribute("kind", "in");
			if (kind == "return") {
				// TEMPORARY: For the time being we are sure
				// this is a foreign XMI file.
				// Use deferred xmi.id resolution.
				m_pSecondary = NULL;
				m_SecondaryId = pAtt->getSecondaryId();
				delete pAtt;
			} else {
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

bool UMLOperation::isConstructorOperation() {
	UMLClassifier * c = dynamic_cast<UMLClassifier*>(this->parent());

	QString cName = c->getName();
	QString opName = getName();
	QString opReturn = getTypeName();

	// its a constructor operation if the operation name and return type
	// match that of the parent classifier
	return (cName == opName && cName == opReturn) ? true : false;
}

bool UMLOperation::showPropertiesDialogue(QWidget* parent) {
	UMLOperationDialog dialogue(parent, this);
	return dialogue.exec();
}


#include "operation.moc"
