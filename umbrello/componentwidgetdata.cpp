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

#include "componentwidgetdata.h"

ComponentWidgetData::ComponentWidgetData(SettingsDlg::OptionState optionState):UMLWidgetData(optionState) {
	m_bShowStereotype = true;
	m_Type = Uml::wt_Component;
	m_bIsInstance = false;
	m_instanceName = "";
}

ComponentWidgetData::~ComponentWidgetData() {}

ComponentWidgetData::ComponentWidgetData(ComponentWidgetData& Other) : UMLWidgetData(Other) {
	*this = Other;
}

ComponentWidgetData & ComponentWidgetData::operator=(ComponentWidgetData& Other) {
	*((UMLWidgetData*)this) = (UMLWidgetData)Other;

	m_bShowStereotype = Other.m_bShowStereotype;

	return *this;
}

bool ComponentWidgetData::operator==(ComponentWidgetData & Other) {
	if(!(*((UMLWidgetData*)this) == (UMLWidgetData)Other)) {
		return false;
	}

	if(m_bShowStereotype != Other.m_bShowStereotype) {
		return false;
	}

	return true;
}

long ComponentWidgetData::getClipSizeOf() {
	long l_size = UMLWidgetData::getClipSizeOf();
	//Q_UINT32 tmp; //tmp is used to calculate the size of each serialized null string

	l_size += sizeof((int)m_bShowStereotype);

	return l_size;
}

/** No descriptions */
bool ComponentWidgetData::serialize(QDataStream *s, bool archive, int fileversion) {
	if(!UMLWidgetData::serialize(s, archive, fileversion)) {
		return false;
	}
	if(archive) {
		*s << (int)m_bShowStereotype;
	} else {
		int showStereotype = 0;
		*s >> showStereotype;
		m_bShowStereotype = (bool)showStereotype;
	}

	return true;
}

/** Read property of bool m_bShowStereotype. */
bool ComponentWidgetData::getShowStereotype() {
	return m_bShowStereotype;
}

/** Write property of bool m_bShowStereotype. */
void ComponentWidgetData::setShowStereotype( bool ShowStereotype) {
	m_bShowStereotype = ShowStereotype;
}

/** No descriptions */
void ComponentWidgetData::print2cerr() {
	UMLWidgetData::print2cerr();
	if(m_bShowStereotype) {
		kdDebug() << "m_bShowStereotype = true" << endl;
	} else {
		kdDebug() << "m_bShowStereotype = false" << endl;
	}
}

bool ComponentWidgetData::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement conceptElement = qDoc.createElement("componentwidget");
	bool status = UMLWidgetData::saveToXMI(qDoc, conceptElement);
	conceptElement.setAttribute("showstereotype", m_bShowStereotype);
	conceptElement.setAttribute("isinstance", m_bIsInstance);
	conceptElement.setAttribute("instancename", m_instanceName);
	qElement.appendChild(conceptElement);
	return status;
}

bool ComponentWidgetData::loadFromXMI(QDomElement& qElement) {
	if ( !UMLWidgetData::loadFromXMI(qElement) ) {
		return false;
	}
	QString showstereo = qElement.attribute("showstereotype", "0");
	m_bShowStereotype = (bool)showstereo.toInt();
	QString isinstance = qElement.attribute("isinstance", "0");
	m_bIsInstance = (bool)isinstance.toInt();
	m_instanceName = qElement.attribute("instancename", "");
	return true;
}

void ComponentWidgetData::setIsInstance(bool isInstance) {
	m_bIsInstance = isInstance;
}

bool ComponentWidgetData::getIsInstance() {
	return m_bIsInstance;
}

void ComponentWidgetData::setInstanceName(QString instanceName) {
	m_instanceName = instanceName;
}

QString ComponentWidgetData::getInstanceName() {
	return m_instanceName;
}

