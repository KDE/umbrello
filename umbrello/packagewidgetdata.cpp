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

#include "packagewidgetdata.h"

PackageWidgetData::PackageWidgetData(SettingsDlg::OptionState optionState):UMLWidgetData(optionState) {
	m_bShowStereotype = true;
	m_Type = Uml::wt_Package;
}

PackageWidgetData::~PackageWidgetData() {}

PackageWidgetData::PackageWidgetData(PackageWidgetData& Other) : UMLWidgetData(Other) {
	*this = Other;
}

PackageWidgetData & PackageWidgetData::operator=(PackageWidgetData& Other) {
	*((UMLWidgetData*)this) = (UMLWidgetData)Other;

	m_bShowStereotype = Other.m_bShowStereotype;

	return *this;
}

bool PackageWidgetData::operator==(PackageWidgetData & Other) {
	if(!(*((UMLWidgetData*)this) == (UMLWidgetData)Other)) {
		return false;
	}

	if(m_bShowStereotype != Other.m_bShowStereotype) {
		return false;
	}

	return true;
}

bool PackageWidgetData::getShowStereotype() {
	return m_bShowStereotype;
}

void PackageWidgetData::setShowStereotype( bool ShowStereotype) {
	m_bShowStereotype = ShowStereotype;
}

void PackageWidgetData::print2cerr() {
	UMLWidgetData::print2cerr();
	if(m_bShowStereotype) {
		kdDebug() << "m_bShowStereotype = true" << endl;
	} else {
		kdDebug() << "m_bShowStereotype = false" << endl;
	}
}

bool PackageWidgetData::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement conceptElement = qDoc.createElement("packagewidget");
	bool status = UMLWidgetData::saveToXMI(qDoc, conceptElement);
	conceptElement.setAttribute("showstereotype", m_bShowStereotype);
	qElement.appendChild(conceptElement);
	return status;
}

bool PackageWidgetData::loadFromXMI(QDomElement& qElement) {
	if ( !UMLWidgetData::loadFromXMI(qElement) ) {
		return false;
	}
	QString showstereo = qElement.attribute("showstereotype", "0");
	m_bShowStereotype = (bool)showstereo.toInt();
	return true;
}




