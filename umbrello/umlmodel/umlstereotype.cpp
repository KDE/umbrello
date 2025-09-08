/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umlstereotype.h"

// local includes
#include "debug_utils.h"
#include "dialog_utils.h"
#include "optionstate.h"
#include "umldoc.h"
#include "umlapp.h"

// kde includes
#include <KLocalizedString>

DEBUG_REGISTER(UMLStereotype)

/**
 * Sets up a stereotype.
 *
 * @param name   The name of this UMLStereotype.
 * @param id     The unique id given to this UMLStereotype.
 */
UMLStereotype::UMLStereotype(const QString &name, Uml::ID::Type id /* = Uml::id_None */)
  : UMLObject(name, id)
{
    m_BaseType = UMLObject::ot_Stereotype;
    UMLStereotype * existing = UMLApp::app()->document()->findStereotype(name);
    if (existing) {
        logError1("UMLStereotype constructor: %1 already exists", name);
    }
    m_refCount = 0;
}

/**
 * Sets up a stereotype.
 */
UMLStereotype::UMLStereotype()
  : UMLObject()
{
    m_BaseType = UMLObject::ot_Stereotype;
    m_refCount = 0;
}

/**
 * Destructor.
 */
UMLStereotype::~UMLStereotype()
{
    //Q_ASSERT(m_refCount == 0);
}

/**
 * Overloaded '==' operator.
 */
bool UMLStereotype::operator==(const UMLStereotype &rhs) const
{
    if (this == &rhs) {
        return true;
    }

    if (!UMLObject::operator==(rhs)) {
        return false;
    }

    return true;
}

/**
 * Copy the internal presentation of this object into the new
 * object.
 */
void UMLStereotype::copyInto(UMLObject *lhs) const
{
    UMLObject::copyInto(lhs);
}

/**
 * Make a clone of this object.
 */
UMLObject* UMLStereotype::clone() const
{
    UMLStereotype *clone = new UMLStereotype();
    copyInto(clone);

    return clone;
}

/**
 * Reset stereotype attribute definitions to none.
 */
void UMLStereotype::clearAttributeDefs()
{
    m_attrDefs.clear();
}

/**
 * Setter for stereotype attribute definitions.
 */
void UMLStereotype::setAttributeDefs(const AttributeDefs& adefs)
{
    m_attrDefs = adefs;
}

/**
 * Const getter for stereotype attribute definitions.
 */
const UMLStereotype::AttributeDefs& UMLStereotype::getAttributeDefs() const
{
    return m_attrDefs;
}

/**
 * Getter for stereotype attribute definitions returning writable data.
 */
UMLStereotype::AttributeDefs& UMLStereotype::getAttributeDefs()
{
    return m_attrDefs;
}

/**
 * Saves to the <UML:StereoType> XMI element.
 */
void UMLStereotype::saveToXMI(QXmlStreamWriter& writer)
{
    //FIXME: uml13.dtd compliance
    UMLObject::save1(writer, QStringLiteral("Stereotype"));
    if (!m_attrDefs.isEmpty()) {
        if (! Settings::optionState().generalState.uml2) {
            writer.writeStartElement(QStringLiteral("UML:Stereotype.feature"));
        }
        for(AttributeDef ad : m_attrDefs) {
            const QString tag = (Settings::optionState().generalState.uml2 ?
                                 QStringLiteral("ownedAttribute") : QStringLiteral("UML:Attribute"));
            writer.writeStartElement(tag);
            writer.writeAttribute(QStringLiteral("name"), ad.name);
            writer.writeAttribute(QStringLiteral("type"), Uml::PrimitiveTypes::toString(ad.type));
            if (!ad.defaultVal.isEmpty())
                writer.writeAttribute(QStringLiteral("initialValue"), ad.defaultVal);
            writer.writeEndElement();            // UML:Attribute
        }
        if (! Settings::optionState().generalState.uml2) {
            writer.writeEndElement();            // UML:Stereotype.feature
        }
    }
    writer.writeEndElement();
}

/**
 * Auxiliary to loadFromXMI:
 * The loading of stereotype attributes is implemented here.
 */
bool UMLStereotype::load1(QDomElement& element)
{
    if (!element.hasChildNodes()) {
        return true;
    }
    for (QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isComment())
            continue;
        element = node.toElement();
        QString tag = element.tagName();
        if (UMLDoc::tagEq(tag, QStringLiteral("Stereotype.feature"))) {
            QDomNode attNode = element.firstChild();
            QDomElement attElem = attNode.toElement();
            while (!attElem.isNull()) {
                tag = attElem.tagName();
                if (UMLDoc::tagEq(tag, QStringLiteral("Attribute"))) {
                    QString name = attElem.attribute(QStringLiteral("name"));
                    QString typeStr = attElem.attribute(QStringLiteral("type"));
                    Uml::PrimitiveTypes::Enum type = Uml::PrimitiveTypes::fromString(typeStr);
                    QString dfltVal = attElem.attribute(QStringLiteral("initialValue"));
                    AttributeDef ad(name, type, dfltVal);
                    m_attrDefs.append(ad);
                } else {
                    logDebug2("UMLStereotype::load1(%1): Unknown Stereotype.feature child %2",
                              m_name, tag);
                }
                attNode = attNode.nextSibling();
                attElem = attNode.toElement();
            }
        }
    }
    return true;
}

/**
 * Display the properties configuration dialog for the stereotype
 * (just a line edit).
 */
bool UMLStereotype::showPropertiesDialog(QWidget* parent)
{
    Q_UNUSED(parent);
    QString stereoTypeName = name();
    bool ok = Dialog_Utils::askRenameName(baseType(), stereoTypeName);
    if (ok) {
        setName(stereoTypeName);
    }
    return ok;
}

/**
 * Increments the reference count for this stereotype.
 */
void UMLStereotype::incrRefCount()
{
    m_refCount++;
}

/**
 * Decrements the reference count for this stereotype.
 */
void UMLStereotype::decrRefCount()
{
    m_refCount--;
}

/**
 * Returns the reference count for this stereotype.
 */
int UMLStereotype::refCount() const
{
    return m_refCount;
}

/**
 * Returns the name as string
 */
QString UMLStereotype::name(bool includeAdornments) const
{
    if (includeAdornments) {
        return QString::fromUtf8("«") + UMLObject::name() + QString::fromUtf8("»");
    }
    return UMLObject::name();
}
