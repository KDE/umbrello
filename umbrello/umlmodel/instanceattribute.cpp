/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

//own header
#include "instanceattribute.h"

//local includes
#include "instance.h"
#include "attribute.h"
#include "debug_utils.h"
#include "model_utils.h"
#include "umldoc.h"
#include "uml.h"
#include "umlinstanceattributedialog.h"
#include "uniqueid.h"
#include "object_factory.h"
#include "optionstate.h"

//kde includes
#include <KLocalizedString>
#include <KMessageBox>

/**
 * Constructor
 * @param parent   The UMLInstance to which this instance attribute belongs.
 * @param umlAttr  The UMLAttribute which this instance attribute reifies.
 *                 It is expected that umlAttr be a non null pointer.
 *                 If umlAttr is passed in as nullptr then the setAttribute
 *                 method shall be used for setting a non null pointer
 *                 before the instance attribute is used by the application.
 * @param value    The value of the instance attribute.
 */
UMLInstanceAttribute::UMLInstanceAttribute(UMLInstance *parent,
                                           UMLAttribute *umlAttr, const QString& value)
    : UMLObject(parent)
{
    m_nId = UniqueID::gen();
    m_pSecondary = umlAttr;
    m_value = value;
    init();
}

/**
 * @brief UMLInstanceAttribute::init
 * Initialize members of this class
 */
void UMLInstanceAttribute::init()
{
    m_BaseType = UMLObject::ot_InstanceAttribute;
}

/**
 * Sets the UMLInstanceAttribute's UML attribute.
 * @param umlAttr  Non null pointer to UMLAttribute.
 */
void UMLInstanceAttribute::setAttribute(UMLAttribute *umlAttr)
{
    Q_ASSERT(umlAttr);
    m_pSecondary = umlAttr;
}

/**
 * Returns the UMLInstanceAttribute's UML attribute.
 * @return  The UMLInstanceAttribute's UML attribute.
 */
UMLAttribute * UMLInstanceAttribute::getAttribute() const
{
    if (m_pSecondary == nullptr)
        return nullptr;
    return m_pSecondary->asUMLAttribute();
}

/**
 * Sets the UMLInstanceAttribute's value.
 * @param value  The value to set.
 */
void UMLInstanceAttribute::setValue(const QString& value)
{
    m_value = value;
}

/**
 * Returns the UMLInstanceAttribute's value.
 * @return  The UMLInstanceAttribute's value.
 */
QString UMLInstanceAttribute::getValue() const
{
    return m_value;
}

/**
 * Returns the textual notation for instance attribute.
 * @return  Stringified attribute name and value.
 */
QString UMLInstanceAttribute::toString() const
{
    QString result(m_pSecondary->name() + QLatin1String(" = ") + m_value);
    return result;
}

/**
 * Creates the <UML:InstanceAttribute> XMI element.
 */
void UMLInstanceAttribute::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("slot"));
    if (Settings::optionState().generalState.uml2) {
        writer.writeAttribute(QLatin1String("xmi:id"), Uml::ID::toString(m_nId));
    } else {
        writer.writeAttribute(QLatin1String("xmi.id"), Uml::ID::toString(m_nId));
    }
    Q_ASSERT(m_pSecondary);
    writer.writeAttribute(QLatin1String("attribute"), Uml::ID::toString(m_pSecondary->id()));
    writer.writeAttribute(QLatin1String("value"), m_value);
    writer.writeEndElement();
}

/**
 * Loads the UMLInstance <slot> XMI element.
 */
bool UMLInstanceAttribute::load1(QDomElement & element)
{
    QString id = Model_Utils::getXmiId(element);
    if (id.isEmpty() || id == QLatin1String("-1")) {
        logWarn0("UMLInstanceAttribute::load1: xmi.id not present, generating a new one");
        m_nId = UniqueID::gen();
    } else {
        m_nId = Uml::ID::fromString(id);
    }
    m_SecondaryId = element.attribute(QLatin1String("attribute"));
    if (m_SecondaryId.isEmpty() || m_SecondaryId == QLatin1String("-1")) {
        logError0("UMLInstanceAttribute::load1: element 'attribute' not set or empty");
        return false;
    }
    UMLDoc *pDoc = UMLApp::app()->document();
    m_pSecondary = pDoc->findObjectById(Uml::ID::fromString(m_SecondaryId));
    if (m_pSecondary)
        m_SecondaryId.clear();
    m_value = element.attribute(QLatin1String("value"));
    return true;
}

/**
 * Display the properties configuration dialog for the instanceattribute.
 */
bool UMLInstanceAttribute::showPropertiesDialog(QWidget* parent)
{
    UMLInstanceAttributeDialog dialog(parent, this);
    return dialog.exec();
}

