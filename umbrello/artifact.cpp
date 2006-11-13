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

UMLArtifact::UMLArtifact(const QString & name, Uml::IDType id)
        : UMLCanvasObject(name, id) {
    init();
}

UMLArtifact::~UMLArtifact() {
}

void UMLArtifact::init() {
    m_BaseType = Uml::ot_Artifact;
    m_drawAsType = defaultDraw;
}

UMLObject* UMLArtifact::clone() const {
    UMLArtifact *clone = new UMLArtifact();
    UMLObject::copyInto(clone);
    return clone;
}

void UMLArtifact::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement artifactElement = UMLObject::save("UML:Artifact", qDoc);
    artifactElement.setAttribute("drawas", m_drawAsType);
    qElement.appendChild(artifactElement);
}

bool UMLArtifact::load(QDomElement& element) {
    QString drawAs = element.attribute("drawas", "0");
    m_drawAsType = (Draw_Type)drawAs.toInt();
    return true;
}

void UMLArtifact::setDrawAsType(Draw_Type type) {
    m_drawAsType = type;
}

UMLArtifact::Draw_Type UMLArtifact::getDrawAsType() {
    return m_drawAsType;
}

#include "artifact.moc"
