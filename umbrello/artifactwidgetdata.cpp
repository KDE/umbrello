/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdatastream.h>
#include <kdebug.h>

#include "artifactwidgetdata.h"

ArtifactWidgetData::ArtifactWidgetData(SettingsDlg::OptionState optionState):UMLWidgetData(optionState) {
	m_Type = Uml::wt_Artifact;
}

ArtifactWidgetData::~ArtifactWidgetData() {}

ArtifactWidgetData::ArtifactWidgetData(ArtifactWidgetData& Other) : UMLWidgetData(Other) {
	*this = Other;
}

ArtifactWidgetData & ArtifactWidgetData::operator=(ArtifactWidgetData& Other) {
	*((UMLWidgetData*)this) = (UMLWidgetData)Other;

	return *this;
}

bool ArtifactWidgetData::operator==(ArtifactWidgetData & Other) {
	if(!(*((UMLWidgetData*)this) == (UMLWidgetData)Other)) {
		return false;
	}

	return true;
}

long ArtifactWidgetData::getClipSizeOf() {
	long l_size = UMLWidgetData::getClipSizeOf();
	//Q_UINT32 tmp; //tmp is used to calculate the size of each serialized null string

	return l_size;
}

/** No descriptions */
bool ArtifactWidgetData::serialize(QDataStream *s, bool archive, int fileversion) {
	if(!UMLWidgetData::serialize(s, archive, fileversion)) {
		return false;
	}

	return true;
}

bool ArtifactWidgetData::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement conceptElement = qDoc.createElement("artifactwidget");
	bool status = UMLWidgetData::saveToXMI(qDoc, conceptElement);
	qElement.appendChild(conceptElement);
	return status;
}

bool ArtifactWidgetData::loadFromXMI(QDomElement& qElement) {
	if ( !UMLWidgetData::loadFromXMI(qElement) ) {
		return false;
	}
	return true;
}




