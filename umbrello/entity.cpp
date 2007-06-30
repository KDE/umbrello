/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
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
#include "uniqueid.h"
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

UMLAttribute* UMLEntity::createAttribute(const QString &name /*=null*/, UMLObject *type /*=NULL*/) {
    Uml::IDType id = UniqueID::gen();
    QString currentName;
    if (name.isNull())  {
        currentName = uniqChildName(Uml::ot_EntityAttribute);
    } else {
        currentName = name;
    }
    const Settings::OptionState optionState = Settings::getOptionState();
    Uml::Visibility scope = optionState.classState.defaultAttributeScope;
    UMLEntityAttribute* newAttribute = new UMLEntityAttribute(this, currentName, id, scope, type);

    int button = QDialog::Accepted;
    bool goodName = false;

    //check for name.isNull() stops dialog being shown
    //when creating attribute via list view
    while (button==QDialog::Accepted && !goodName && name.isNull()) {
        UMLEntityAttributeDialog attributedialog(0, newAttribute);
        button = attributedialog.exec();
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

    addEntityAttribute(newAttribute);

    UMLDoc *umldoc = UMLApp::app()->getDocument();
    umldoc->signalUMLObjectCreated(newAttribute);
    return newAttribute;
}

UMLObject* UMLEntity::addEntityAttribute(const QString& name, Uml::IDType id) {
    UMLEntityAttribute* literal = new UMLEntityAttribute(this, name, id);
    m_List.append(literal);
    emit entityAttributeAdded(literal);
    UMLObject::emitModified();
    connect(literal,SIGNAL(modified()),this,SIGNAL(modified()));
    return literal;
}

bool UMLEntity::addEntityAttribute(UMLEntityAttribute* attribute, IDChangeLog* Log /* = 0*/) {
    QString name = (QString)attribute->getName();
    if (findChildObject(name) == NULL) {
        attribute->parent()->removeChild(attribute);
        this->insertChild(attribute);
        m_List.append(attribute);
        emit entityAttributeAdded(attribute);
        UMLObject::emitModified();
        connect(attribute,SIGNAL(modified()),this,SIGNAL(modified()));
        return true;
    } else if (Log) {
        Log->removeChangeByNewID( attribute->getID() );
        delete attribute;
    }
    return false;
}

bool UMLEntity::addEntityAttribute(UMLEntityAttribute* attribute, int position) {
    QString name = (QString)attribute->getName();
    if (findChildObject(name) == NULL) {
        attribute->parent()->removeChild(attribute);
        this->insertChild(attribute);
        if ( position >= 0 && position <= (int)m_List.count() )  {
            m_List.insert(position,attribute);
        } else {
            m_List.append(attribute);
        }
        emit entityAttributeAdded(attribute);
        UMLObject::emitModified();
        connect(attribute,SIGNAL(modified()),this,SIGNAL(modified()));
        return true;
    }
    return false;
}

int UMLEntity::removeEntityAttribute(UMLClassifierListItem* literal) {
    if (!m_List.remove((UMLEntityAttribute*)literal)) {
        kDebug() << "can't find att given in list" << endl;
        return -1;
    }
    emit entityAttributeRemoved(literal);
    UMLObject::emitModified();
    // If we are deleting the object, then we don't need to disconnect..this is done auto-magically
    // for us by QObject. -b.t.
    // disconnect(a,SIGNAL(modified()),this,SIGNAL(modified()));
    delete literal;
    return m_List.count();
}

int UMLEntity::entityAttributes() {
    UMLClassifierListItemList entityAttributes = getFilteredList(Uml::ot_EntityAttribute);
    return entityAttributes.count();
}

void UMLEntity::signalEntityAttributeRemoved(UMLClassifierListItem *eattr) {
    emit entityAttributeRemoved(eattr);
}

bool UMLEntity::resolveRef() {
    bool success = UMLClassifier::resolveRef();
    for (UMLObjectListIt oit(m_List); oit.current(); ++oit) {
        UMLObject* obj = oit.current();
        if (obj->resolveRef()) {
            UMLClassifierListItem *cli = static_cast<UMLClassifierListItem*>(obj);
            if (cli->getBaseType() == Uml::ot_EntityAttribute)
                emit entityAttributeAdded(cli);
        }
    }
    return success;
}

void UMLEntity::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement entityElement = UMLObject::save("UML:Entity", qDoc);
    //save operations
    UMLClassifierListItemList entityAttributes = getFilteredList(Uml::ot_EntityAttribute);
    UMLClassifierListItem* pEntityAttribute = 0;
    for (UMLClassifierListItemListIt it(entityAttributes);
         (pEntityAttribute = it.current()) != NULL; ++it) {
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
            kDebug() << "UMLEntity::load(" << m_Name
            << "): losing old-format stereotype." << endl;
        } else {
            kWarning() << "unknown child type in UMLEntity::load" << endl;
        }
        node = node.nextSibling();
    }//end while
    return true;
}


#include "entity.moc"
