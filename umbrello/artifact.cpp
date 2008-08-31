/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "artifact.h"
#include "association.h"
#include "clipboard/idchangelog.h"
#include <kdebug.h>
#include <klocale.h>

/**
 * Sets up an Artifact.
 *
 * @param Name              The name of the Concept.
 * @param id                The unique id of the Concept.
 */
UMLArtifact::UMLArtifact(const QString & name, Uml::IDType id)
        : UMLCanvasObject(name, id) {
    init();
}

UMLArtifact::~UMLArtifact() {
}

/**
 * Initializes key variables of the class.
 */
void UMLArtifact::init() {
    m_BaseType = Uml::ot_Artifact;
    m_drawAsType = defaultDraw;
}

/**
 * Make a clone of this object.
 */
UMLObject* UMLArtifact::clone() const {
    UMLArtifact *clone = new UMLArtifact();
    UMLObject::copyInto(clone);
    return clone;
}

/**
 * Creates the UML:Artifact element including its operations,
 * attributes and templates
 */
void UMLArtifact::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement artifactElement = UMLObject::save("UML:Artifact", qDoc);
    artifactElement.setAttribute("drawas", m_drawAsType);
    qElement.appendChild(artifactElement);
}

/**
 * Loads the UML:Artifact element including its operations,
 * attributes and templates
 */
bool UMLArtifact::load(QDomElement& element) {
    QString drawAs = element.attribute("drawas", "0");
    m_drawAsType = (Draw_Type)drawAs.toInt();
    return true;
}

/**
 * sets m_drawAsType for which method to draw the artifact as
 */
void UMLArtifact::setDrawAsType(Draw_Type type) {
    m_drawAsType = type;
}

/**
 * returns the value of m_drawAsType
 */
UMLArtifact::Draw_Type UMLArtifact::getDrawAsType() {
    return m_drawAsType;
}

#include "artifact.moc"
