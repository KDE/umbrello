/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "artifact.h"
#include "association.h"
#include "clipboard/idchangelog.h"
#include "umldoc.h"

#include <kdebug.h>
#include <klocale.h>

UMLArtifact::UMLArtifact(UMLDoc * parent, const QString & name, int id) : UMLCanvasObject(parent, name, id) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLArtifact::~UMLArtifact() {
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLArtifact::init() {
	m_BaseType = ot_Artifact;
	m_drawAsType = defaultDraw;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLArtifact::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement artifactElement = qDoc.createElement("UML:Artifact");
	bool status = UMLObject::saveToXMI(qDoc, artifactElement);
	artifactElement.setAttribute("drawas", m_drawAsType);
	qElement.appendChild(artifactElement);
	return status;
}

bool UMLArtifact::loadFromXMI(QDomElement& element) {
	if ( !UMLObject::loadFromXMI(element) ) {
		return false;
	}
	QString drawAs = element.attribute("drawas", "0");
	m_drawAsType = (Artifact_draw_type)drawAs.toInt();
	return true;
}

void UMLArtifact::setDrawAsType(Artifact_draw_type type) {
	m_drawAsType = type;
}

Artifact_draw_type UMLArtifact::getDrawAsType() {
	return m_drawAsType;
}

#include "artifact.moc"
