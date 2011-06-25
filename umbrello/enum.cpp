/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "enum.h"

// app includes
#include "debug_utils.h"
#include "enumliteral.h"
#include "umldoc.h"
#include "uml.h"
#include "uniqueid.h"
#include "idchangelog.h"

// kde includes
#include <klocale.h>
#include <kmessagebox.h>

/**
 * Sets up an enum.
 * @param name  The name of the Enum.
 * @param id  The unique id of the Enum.
 */
UMLEnum::UMLEnum(const QString& name, Uml::IDType id) : UMLClassifier(name, id)
{
    init();
}

/**
 * Standard destructor.
 */
UMLEnum::~UMLEnum()
{
    m_List.clear();
}

/**
 * Overloaded '==' operator.
 */
bool UMLEnum::operator==(const UMLEnum & rhs) const
{
    return UMLClassifier::operator==(rhs);
}

/**
 * Copy the internal presentation of this object into the new
 * object.
 */
void UMLEnum::copyInto(UMLObject *lhs) const
{
    UMLClassifier::copyInto(lhs);
}

/**
 * Make a clone of this object.
 */
UMLObject* UMLEnum::clone() const
{
    UMLEnum *clone = new UMLEnum();
    copyInto(clone);

    return clone;
}

/**
 * Initializes key variables of the class.
 */
void UMLEnum::init()
{
    m_BaseType = UMLObject::ot_Enum;
    setStereotype( "enum" );
}

/**
 * Creates a literal for the enum.
 * @return  The UMLEnum created
 */
UMLObject* UMLEnum::createEnumLiteral(const QString& name)
{
    Uml::IDType id = UniqueID::gen();
    QString currentName;
    if (name.isNull())  {
        currentName = uniqChildName(UMLObject::ot_EnumLiteral);
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
        QString name = newEnumLiteral->name();

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

    UMLDoc *umldoc = UMLApp::app()->document();
    umldoc->signalUMLObjectCreated(newEnumLiteral);
    return newEnumLiteral;
}

/**
 * Adds an enumliteral to the enum.
 * @param name  The name of the enumliteral.
 * @param id  The id of the enumliteral (optional.)
 *            If omitted a new ID is assigned internally.
 * @return  Pointer to the UMLEnumliteral created.
 */
UMLObject* UMLEnum::addEnumLiteral(const QString &name, Uml::IDType id)
{
    UMLObject *el = UMLCanvasObject::findChildObject(name);
    if (el != NULL) {
        uDebug() << name << " is already present";
        return el;
    }
    UMLEnumLiteral* literal = new UMLEnumLiteral(this, name, id);
    m_List.append(literal);
    UMLObject::emitModified();
    emit enumLiteralAdded(literal);
    connect(literal,SIGNAL(modified()),this,SIGNAL(modified()));
    return literal;
}

/**
 * Adds an already created enumliteral.
 * The enumliteral object must not belong to any other concept.
 * @param literal  Pointer to the UMLEnumLiteral.
 * @param Log      Pointer to the IDChangeLog.
 * @return  True if the enumliteral was successfully added.
 */
bool UMLEnum::addEnumLiteral(UMLEnumLiteral* literal, IDChangeLog* Log /* = 0*/)
{
    QString name = (QString)literal->name();
    if (findChildObject(name) == NULL) {
        literal->setParent(this);
        m_List.append(literal);
        UMLObject::emitModified();
        emit enumLiteralAdded(literal);
        connect(literal,SIGNAL(modified()),this,SIGNAL(modified()));
        return true;
    } else if (Log) {
        Log->removeChangeByNewID( literal->id() );
        delete literal;
    }
    return false;
}

/**
 * Adds an enumliteral to the enum, at the given position.
 * If position is negative or too large, the enumliteral is added
 * to the end of the list.
 * TODO:  give default value -1 to position (append) - now it conflicts with the method above..
 * @param literal   Pointer to the UMLEnumLiteral.
 * @param position  Position index for the insertion.
 * @return  True if the enumliteral was successfully added.
 */
bool UMLEnum::addEnumLiteral(UMLEnumLiteral* literal, int position)
{
    QString name = (QString)literal->name();
    if (findChildObject(name) == NULL) {
        literal->setParent(this);
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

/**
 * Removes an enumliteral from the class.
 * @param literal  The enumliteral to remove.
 * @return  Count of the remaining enumliterals after removal.
 *          Returns -1 if the given enumliteral was not found.
 */
int UMLEnum::removeEnumLiteral(UMLEnumLiteral* literal)
{
    if (!m_List.removeAll(literal)) {
        uDebug() << "can not find att given in list";
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

/**
 * Returns the number of enumliterals for the class.
 * @return  The number of enumliterals for the class.
 */
int UMLEnum::enumLiterals()
{
    return m_List.count();
}

/**
 * Emit the enumLiteralRemoved signal.
 */
void UMLEnum::signalEnumLiteralRemoved(UMLClassifierListItem *elit)
{
    emit enumLiteralRemoved(elit);
}

/**
 * Creates the <UML:Enum> element including its enumliterals.
 */
void UMLEnum::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement enumElement = UMLObject::save("UML:Enumeration", qDoc);
    // save enum literals
    UMLClassifierListItemList enumLiterals = getFilteredList(UMLObject::ot_EnumLiteral);
    foreach (UMLClassifierListItem* pEnumLiteral , enumLiterals ) {
        pEnumLiteral->saveToXMI(qDoc, enumElement);
    }
    qElement.appendChild(enumElement);
}

/**
 * Loads the <UML:Enum> element including its enumliterals.
 */
bool UMLEnum::load(QDomElement& element)
{
    QDomNode node = element.firstChild();
    while( !node.isNull() ) {
        if (node.isComment()) {
            node = node.nextSibling();
            continue;
        }
        QDomElement tempElement = node.toElement();
        QString tag = tempElement.tagName();
        if (UMLDoc::tagEq(tag, "EnumerationLiteral") ||
                UMLDoc::tagEq(tag, "EnumLiteral")) {   // for backward compatibility
            UMLEnumLiteral* pEnumLiteral = new UMLEnumLiteral(this);
            if( !pEnumLiteral->loadFromXMI(tempElement) ) {
                return false;
            }
            m_List.append(pEnumLiteral);
        } else if (tag == "stereotype") {
            uDebug() << name() << ": losing old-format stereotype.";
        } else {
            uWarning() << "unknown child type in UMLEnum::load";
        }
        node = node.nextSibling();
    }//end while
    return true;
}

/**
 * Create a new ClassifierListObject (enumLiteral)
 * according to the given XMI tag.
 * Returns NULL if the string given does not contain one of the tags
 * <UML:EnumLiteral>
 * Used by the clipboard for paste operation.
 * Reimplemented from UMLClassifier for UMLEnum
 */
UMLClassifierListItem* UMLEnum::makeChildObject(const QString& xmiTag)
{
    UMLClassifierListItem* pObject = NULL;
    if (UMLDoc::tagEq(xmiTag, "EnumerationLiteral") || UMLDoc::tagEq( xmiTag, "EnumLiteral")) {
        pObject = new UMLEnumLiteral(this);
    }
    return pObject;
}

#include "enum.moc"
