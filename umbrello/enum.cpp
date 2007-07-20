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
#include "enum.h"
// qt/kde includes
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
// app includes
#include "enumliteral.h"
#include "umldoc.h"
#include "uml.h"
#include "uniqueid.h"
#include "clipboard/idchangelog.h"

UMLEnum::UMLEnum(const QString& name, Uml::IDType id) : UMLClassifier(name, id) {
    init();
}

UMLEnum::~UMLEnum() {
    m_List.clear();
}

bool UMLEnum::operator==( UMLEnum & rhs ) {
    return UMLClassifier::operator==(rhs);
}

void UMLEnum::copyInto(UMLEnum *rhs) const
{
    UMLClassifier::copyInto(rhs);
}

UMLObject* UMLEnum::clone() const
{
    UMLEnum *clone = new UMLEnum();
    copyInto(clone);

    return clone;
}

void UMLEnum::init() {
    m_BaseType = Uml::ot_Enum;
    setStereotype( "enum" );
}

UMLObject* UMLEnum::createEnumLiteral(const QString& name) {
    Uml::IDType id = UniqueID::gen();
    QString currentName;
    if (name.isNull())  {
        currentName = uniqChildName(Uml::ot_EnumLiteral);
    } else {
        currentName = name;
    }

    UMLEnumLiteral* newEnumLiteral = new UMLEnumLiteral(this, currentName);

    bool ok = true;
    bool goodName = false;

    //check for name.isNull() stops dialog being shown
    //when creating enum literal via list view
    while (ok && !goodName && name.isNull()) {
        ok = newEnumLiteral->showPropertiesDialog( UMLApp::app() );
        QString name = newEnumLiteral->getName();

        if(name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        } else {
            goodName = true;
        }
    }

    if (!ok) {
        delete newEnumLiteral;
        return NULL;
    }

    addEnumLiteral(newEnumLiteral);

    UMLDoc *umldoc = UMLApp::app()->getDocument();
    umldoc->signalUMLObjectCreated(newEnumLiteral);
    return newEnumLiteral;
}

UMLObject* UMLEnum::addEnumLiteral(const QString &name, Uml::IDType id) {
    UMLObject *el = UMLCanvasObject::findChildObject(name);
    if (el != NULL) {
        kDebug() << "UMLEnum::addEnumLiteral: " << name
                  << " is already present" << endl;
        return el;
    }
    UMLEnumLiteral* literal = new UMLEnumLiteral(this, name, id);
    m_List.append(literal);
    UMLObject::emitModified();
    emit enumLiteralAdded(literal);
    connect(literal,SIGNAL(modified()),this,SIGNAL(modified()));
    return literal;
}

bool UMLEnum::addEnumLiteral(UMLEnumLiteral* literal, IDChangeLog* Log /* = 0*/) {
    QString name = (QString)literal->getName();
    if (findChildObject(name) == NULL) {
        literal->parent()->removeChild(literal);
        this->insertChild(literal);
        m_List.append(literal);
        UMLObject::emitModified();
        emit enumLiteralAdded(literal);
        connect(literal,SIGNAL(modified()),this,SIGNAL(modified()));
        return true;
    } else if (Log) {
        Log->removeChangeByNewID( literal->getID() );
        delete literal;
    }
    return false;
}

bool UMLEnum::addEnumLiteral(UMLEnumLiteral* literal, int position) {
    QString name = (QString)literal->getName();
    if (findChildObject(name) == NULL) {
        literal->parent()->removeChild(literal);
        this->insertChild(literal);
        if ( position >= 0 && position <= (int)m_List.count() )  {
            m_List.insert(position,literal);
        } else {
            m_List.append(literal);
        }
        UMLObject::emitModified();
        emit enumLiteralAdded(literal);
        connect(literal,SIGNAL(modified()),this,SIGNAL(modified()));
        return true;
    }
    return false;
}

int UMLEnum::removeEnumLiteral(UMLEnumLiteral* literal) {
    if (!m_List.remove(literal)) {
        kDebug() << "can't find att given in list" << endl;
        return -1;
    }
    emit enumLiteralRemoved(literal);
    UMLObject::emitModified();
    // If we are deleting the object, then we don't need to disconnect..this is done auto-magically
    // for us by QObject. -b.t.
    // disconnect(a,SIGNAL(modified()),this,SIGNAL(modified()));
    delete literal;
    return m_List.count();
}

int UMLEnum::enumLiterals() {
    return m_List.count();
}

void UMLEnum::signalEnumLiteralRemoved(UMLClassifierListItem *elit) {
    emit enumLiteralRemoved(elit);
}

void UMLEnum::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement enumElement = UMLObject::save("UML:Enumeration", qDoc);
    // save enum literals
    UMLClassifierListItemList enumLiterals = getFilteredList(Uml::ot_EnumLiteral);
    UMLClassifierListItem* pEnumLiteral = 0;
    for (UMLClassifierListItemListIt it(enumLiterals);
         (pEnumLiteral = it.current()) != NULL; ++it) {
        pEnumLiteral->saveToXMI(qDoc, enumElement);
    }
    qElement.appendChild(enumElement);
}

bool UMLEnum::load(QDomElement& element) {
    QDomNode node = element.firstChild();
    while( !node.isNull() ) {
        if (node.isComment()) {
            node = node.nextSibling();
            continue;
        }
        QDomElement tempElement = node.toElement();
        QString tag = tempElement.tagName();
        if (Uml::tagEq(tag, "EnumerationLiteral") ||
                Uml::tagEq(tag, "EnumLiteral")) {   // for backward compatibility
            UMLEnumLiteral* pEnumLiteral = new UMLEnumLiteral(this);
            if( !pEnumLiteral->loadFromXMI(tempElement) ) {
                return false;
            }
            m_List.append(pEnumLiteral);
        } else if (tag == "stereotype") {
            kDebug() << "UMLEnum::load(" << m_Name
            << "): losing old-format stereotype." << endl;
        } else {
            kWarning() << "unknown child type in UMLEnum::load" << endl;
        }
        node = node.nextSibling();
    }//end while
    return true;
}


#include "enum.moc"
