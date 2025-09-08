/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umlartifact.h"
#include "association.h"
#include "clipboard/idchangelog.h"

#include <KLocalizedString>

#include <QDir>

/**
 * Sets up an Artifact.
 * @param name   The name of the Concept.
 * @param id     The unique id of the Concept.
 */
UMLArtifact::UMLArtifact(const QString & name, Uml::ID::Type id)
  : UMLPackage(name, id),
    m_drawAsType(defaultDraw)
{
    m_BaseType = UMLObject::ot_Artifact;
}

/**
 * Standard deconstructor.
 */
UMLArtifact::~UMLArtifact()
{
}

/**
 * Make a clone of this object.
 * @return the cloned object
 */
UMLObject* UMLArtifact::clone() const
{
    UMLArtifact *clone = new UMLArtifact();
    UMLObject::copyInto(clone);
    return clone;
}

/**
 * Creates the UML:Artifact element including its operations,
 * attributes and templates
 * @param writer   the QXmlStreamWriter serialization target
 */
void UMLArtifact::saveToXMI(QXmlStreamWriter& writer) 
{
    UMLObject::save1(writer, QStringLiteral("Artifact"));
    writer.writeAttribute(QStringLiteral("drawas"), QString::number(m_drawAsType));
    UMLObject::save1end(writer);
}

/**
 * Loads the UML:Artifact element including its operations,
 * attributes and templates.
 * @param element   the xml element to load
 * @return the success status of the operation
 */
bool UMLArtifact::load1(QDomElement& element)
{
    QString drawAs = element.attribute(QStringLiteral("drawas"), QStringLiteral("0"));
    m_drawAsType = (Draw_Type)drawAs.toInt();
    return true;
}

/**
 * Sets m_drawAsType for which method to draw the artifact as.
 * @param type   the draw type
 */
void UMLArtifact::setDrawAsType(Draw_Type type)
{
    m_drawAsType = type;
}

/**
 * Returns the value of m_drawAsType.
 * @return the value of the draw type attribute
 */
UMLArtifact::Draw_Type UMLArtifact::getDrawAsType() const
{
    return m_drawAsType;
}

/**
 * Return full path of this artifact including its parent
 * @return full path
 */
QString UMLArtifact::fullPath() const
{
    QString path = name();
    for(UMLPackage *p = umlPackage(); p != nullptr && p->umlPackage() != nullptr; p = p->umlPackage()) {
        path.insert(0, p->name() + QLatin1Char('/'));
    }
    return QDir::toNativeSeparators(path);
}

