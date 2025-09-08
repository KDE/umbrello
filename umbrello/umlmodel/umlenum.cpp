/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umlenum.h"

// app includes
#include "debug_utils.h"
#include "enumliteral.h"
#include "optionstate.h"
#include "umldoc.h"
#include "uml.h"
#include "uniqueid.h"
#include "idchangelog.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

DEBUG_REGISTER(UMLEnum)

/**
 * Sets up an enum.
 * @param name  The name of the Enum.
 * @param id  The unique id of the Enum.
 */
UMLEnum::UMLEnum(const QString& name, Uml::ID::Type id) : UMLClassifier(name, id)
{
    init();
}

/**
 * Standard destructor.
 */
UMLEnum::~UMLEnum()
{
    subordinates().clear();
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
    setStereotypeCmd(QStringLiteral("enum"));
}

/**
 * Creates a literal for the enum.
 * @return  The UMLEnum created
 */
UMLObject* UMLEnum::createEnumLiteral(const QString& name)
{
    Uml::ID::Type id = UniqueID::gen();
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
        ok = newEnumLiteral->showPropertiesDialog(UMLApp::app());
        QString name = newEnumLiteral->name();

        if(name.length() == 0) {
            KMessageBox::error(nullptr, i18n("That is an invalid name."), i18n("Invalid Name"));
        } else {
            goodName = true;
        }
    }

    if (!ok) {
        delete newEnumLiteral;
        return nullptr;
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
 * @param value  Optional numeric representation of the enumliteral.
 * @return  Pointer to the UMLEnumliteral created.
 */
UMLObject* UMLEnum::addEnumLiteral(const QString &name, Uml::ID::Type id, const QString& value)
{
    UMLObject *el = UMLCanvasObject::findChildObject(name);
    if (el != nullptr) {
        logDebug1("UMLEnum::addEnumLiteral: %1 is already present", name);
        return el;
    }
    UMLEnumLiteral* literal = new UMLEnumLiteral(this, name, id, value);
    subordinates().append(literal);
    UMLObject::emitModified();
    Q_EMIT enumLiteralAdded(literal);
    connect(literal, SIGNAL(modified()), this, SIGNAL(modified()));
    return literal;
}

/**
 * Adds an already created enumliteral.
 * The enumliteral object must not belong to any other classifier.
 * @param literal  Pointer to the UMLEnumLiteral.
 * @param Log      Pointer to the IDChangeLog.
 * @return  True if the enumliteral was successfully added.
 */
bool UMLEnum::addEnumLiteral(UMLEnumLiteral* literal, IDChangeLog* Log /* = nullptr*/)
{
    QString name = (QString)literal->name();
    if (findChildObject(name) == nullptr) {
        literal->setParent(this);
        subordinates().append(literal);
        UMLObject::emitModified();
        Q_EMIT enumLiteralAdded(literal);
        connect(literal, SIGNAL(modified()), this, SIGNAL(modified()));
        return true;
    } else if (Log) {
        Log->removeChangeByNewID(literal->id());
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
    Q_ASSERT(literal);
    QString name = (QString)literal->name();
    if (findChildObject(name) == nullptr) {
        literal->setParent(this);
        if (position >= 0 && position <= (int)subordinates().count())  {
            subordinates().insert(position, literal);
        } else {
            subordinates().append(literal);
        }
        UMLObject::emitModified();
        Q_EMIT enumLiteralAdded(literal);
        connect(literal, SIGNAL(modified()), this, SIGNAL(modified()));
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
    if (!subordinates().removeAll(literal)) {
        logDebug0("UMLEnum::removeEnumLiteral: cannot find att given in list");
        return -1;
    }
    Q_EMIT enumLiteralRemoved(literal);
    UMLObject::emitModified();
    // If we are deleting the object, then we don't need to disconnect..this is done auto-magically
    // for us by QObject. -b.t.
    // disconnect(a, SIGNAL(modified()), this, SIGNAL(modified()));
    delete literal;
    return subordinates().count();
}

/**
 * Returns the number of enumliterals for the class.
 * @return  The number of enumliterals for the class.
 */
int UMLEnum::enumLiterals() const
{
    return subordinates().count();
}

/**
 * Emit the enumLiteralRemoved signal.
 */
void UMLEnum::signalEnumLiteralRemoved(UMLClassifierListItem *elit)
{
    Q_EMIT enumLiteralRemoved(elit);
}

/**
 * Creates the <UML:Enum> element including its enumliterals.
 */
void UMLEnum::saveToXMI(QXmlStreamWriter& writer)
{
    UMLObject::save1(writer, QStringLiteral("Enumeration"));
    // save enum literals
    if (! Settings::optionState().generalState.uml2) {
        writer.writeStartElement(QStringLiteral("UML:Enumeration.literal"));
    }
    UMLClassifierListItemList enumLiterals = getFilteredList(UMLObject::ot_EnumLiteral);
    for(UMLClassifierListItem *pEnumLiteral : enumLiterals) {
        pEnumLiteral->saveToXMI(writer);
    }
    if (! Settings::optionState().generalState.uml2) {
        writer.writeEndElement();            // UML:Enumeration.literal
    }
    UMLObject::save1end(writer); // UML:Enumeration
}

/**
 * Loads the <UML:Enum> element including its enumliterals.
 */
bool UMLEnum::load1(QDomElement& element)
{
    QDomNode node = element.firstChild();
    while(!node.isNull()) {
        if (node.isComment()) {
            node = node.nextSibling();
            continue;
        }
        QDomElement tempElement = node.toElement();
        QString tag = tempElement.tagName();
        if (UMLDoc::tagEq(tag, QStringLiteral("EnumerationLiteral")) ||
            UMLDoc::tagEq(tag, QStringLiteral("ownedLiteral")) ||
                UMLDoc::tagEq(tag, QStringLiteral("EnumLiteral"))) {   // for backward compatibility
            UMLEnumLiteral* pEnumLiteral = new UMLEnumLiteral(this);
            if(!pEnumLiteral->loadFromXMI(tempElement)) {
                return false;
            }
            subordinates().append(pEnumLiteral);
        } else if (UMLDoc::tagEq(tag, QStringLiteral("Enumeration.literal"))) {  // UML 1.4
            if (! load1(tempElement))
                return false;
        } else if (tag == QStringLiteral("stereotype")) {
            logDebug1("UMLEnum::load1 %1: losing old-format stereotype.", name());
        } else {
            logWarn1("UMLEnum::load1: unknown child type %1", tag);
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
    UMLClassifierListItem *pObject = nullptr;
    if (UMLDoc::tagEq(xmiTag, QStringLiteral("EnumerationLiteral")) ||
               UMLDoc::tagEq(xmiTag, QStringLiteral("EnumLiteral"))) {
        pObject = new UMLEnumLiteral(this);
    }
    return pObject;
}

