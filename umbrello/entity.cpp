 /*
  *  copyright (C) 2003-2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// own header
#include "entity.h"
// qt/kde includes
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
// app includes
#include "entityattribute.h"
#include "umldoc.h"
#include "uml.h"
#include "clipboard/idchangelog.h"
#include "dialogs/umlentityattributedialog.h"

UMLEntity::UMLEntity(const QString& name, Uml::IDType id) : UMLClassifier(name, id) {
	init();
}

UMLEntity::~UMLEntity() {
	m_List.clear();
}

bool UMLEntity::operator==( UMLEntity& rhs ) {
	return UMLClassifier::operator==(rhs);
}

void UMLEntity::copyInto(UMLEntity *rhs) const
{
	UMLClassifier::copyInto(rhs);
}

UMLObject* UMLEntity::clone() const
{
	UMLEntity* clone = new UMLEntity();
	copyInto(clone);

	return clone;
}

void UMLEntity::init() {
	m_BaseType = Uml::ot_Entity;
}

UMLObject* UMLEntity::createEntityAttribute(const QString &name /*=null*/) {
	UMLDoc *umldoc = UMLApp::app()->getDocument();
	Uml::IDType id = umldoc->getUniqueID();
	QString currentName;
	if (name.isNull())  {
		currentName = uniqChildName(Uml::ot_EntityAttribute);
	} else {
		currentName = name;
	}
	const Settings::OptionState optionState = UMLApp::app()->getOptionState();
	Uml::Scope scope = optionState.classState.defaultAttributeScope;
	UMLEntityAttribute* newAttribute = new UMLEntityAttribute(this, currentName, id, scope);

	int button = QDialog::Accepted;
	bool goodName = false;

	//check for name.isNull() stops dialogue being shown
	//when creating attribute via list view
	while (button==QDialog::Accepted && !goodName && name.isNull()) {
		UMLEntityAttributeDialog attributeDialogue(0, newAttribute);
		button = attributeDialogue.exec();
		QString name = newAttribute->getName();

		if(name.length() == 0) {
			KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
		} else if ( findChildObject(Uml::ot_EntityAttribute, name).count() > 0 ) {
			KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
		} else {
			goodName = true;
		}
	}

	if (button != QDialog::Accepted) {
		return NULL;
	}

	addEntityAttribute(newAttribute);

	umldoc->signalUMLObjectCreated(newAttribute);
	return newAttribute;
}

UMLObject* UMLEntity::addEntityAttribute(const QString& name, Uml::IDType id) {
	UMLEntityAttribute* literal = new UMLEntityAttribute(this, name, id);
	m_List.append(literal);
	emit modified();
	connect(literal,SIGNAL(modified()),this,SIGNAL(modified()));
	emit entityAttributeAdded(literal);
	return literal;
}

bool UMLEntity::addEntityAttribute(UMLEntityAttribute* attribute, IDChangeLog* Log /* = 0*/) {
	QString name = (QString)attribute->getName();
	if (findChildObject(Uml::ot_EntityAttribute, name).count() == 0) {
		attribute->parent()->removeChild(attribute);
		this->insertChild(attribute);
		m_List.append(attribute);
		emit modified();
		connect(attribute,SIGNAL(modified()),this,SIGNAL(modified()));
		emit entityAttributeAdded(attribute);
		return true;
	} else if (Log) {
		Log->removeChangeByNewID( attribute->getID() );
		delete attribute;
	}
	return false;
}

bool UMLEntity::addEntityAttribute(UMLEntityAttribute* attribute, int position) {
	QString name = (QString)attribute->getName();
	if (findChildObject( Uml::ot_EntityAttribute, name).count() == 0) {
		attribute->parent()->removeChild(attribute);
		this->insertChild(attribute);
		if ( position >= 0 && position <= (int)m_List.count() )  {
			m_List.insert(position,attribute);
		} else {
			m_List.append(attribute);
		}
		emit modified();
		connect(attribute,SIGNAL(modified()),this,SIGNAL(modified()));
		emit entityAttributeAdded(attribute);
		return true;
	}
	return false;
}

int UMLEntity::removeEntityAttribute(UMLObject* literal) {
	if (!m_List.remove((UMLEntityAttribute*)literal)) {
		kdDebug() << "can't find att given in list" << endl;
		return -1;
	}
	emit entityAttributeRemoved(literal);
	emit modified();
	// If we are deleteing the object, then we dont need to disconnect..this is done auto-magically
	// for us by QObject. -b.t.
	// disconnect(a,SIGNAL(modified()),this,SIGNAL(modified()));
	delete literal;
	return m_List.count();
}

UMLEntityAttribute* UMLEntity::takeEntityAttribute(UMLEntityAttribute* el) {
	int index = m_List.findRef( el );
	el = (index == -1 ? 0 : dynamic_cast<UMLEntityAttribute*>(m_List.take( )));
	if (el) {
		emit entityAttributeRemoved(el);
		emit modified();
	}
	return el;
}

UMLObjectList UMLEntity::findChildObject(Uml::Object_Type t, const QString &n) {
	UMLObjectList list;
	if (t == Uml::ot_Association) {
		return UMLClassifier::findChildObject(t, n);
	} else if (t == Uml::ot_EntityAttribute) {
		UMLClassifierListItem * obj=0;
		for (obj = m_List.first(); obj != 0; obj = m_List.next()) {
			if (obj->getBaseType() != t)
				continue;
			if (obj->getName() == n)
				list.append( obj );
		}
	} else {
		kdWarning() << "finding child object of unknown type" <<t<<" (requested name = "<<n<<")"<<endl;
	}

	return list;
}

int UMLEntity::entityAttributes() {
	return m_List.count();
}


void UMLEntity::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement entityElement = UMLObject::save("UML:Entity", qDoc);
	//save operations
	UMLClassifierListItem* pEntityAttribute = 0;
	for ( pEntityAttribute = m_List.first(); pEntityAttribute != 0;
	      pEntityAttribute = m_List.next() ) {
		pEntityAttribute->saveToXMI(qDoc, entityElement);
	}
	qElement.appendChild(entityElement);
}

bool UMLEntity::load(QDomElement& element) {
	QDomNode node = element.firstChild();
	while( !node.isNull() ) {
		if (node.isComment()) {
			node = node.nextSibling();
			continue;
		}
		QDomElement tempElement = node.toElement();
		QString tag = tempElement.tagName();
		if (Uml::tagEq(tag, "EntityAttribute")) {   // for backward compatibility
			UMLEntityAttribute* pEntityAttribute = new UMLEntityAttribute(this);
			if( !pEntityAttribute->loadFromXMI(tempElement) ) {
				return false;
			}
			m_List.append(pEntityAttribute);
		} else if (tag == "stereotype") {
			kdDebug() << "UMLEntity::load(" << m_Name
				  << "): losing old-format stereotype." << endl;
		} else {
			kdWarning() << "unknown child type in UMLEntity::load" << endl;
		}
		node = node.nextSibling();
	}//end while
	return true;
}


#include "entity.moc"
