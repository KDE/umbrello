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
#include <klocale.h>

#include "interfacewidgetdata.h"

InterfaceWidgetData::InterfaceWidgetData(SettingsDlg::OptionState optionState):UMLWidgetData(optionState) {
	m_bShowOperations = true;
	m_bShowPackage = false;
	m_ShowOpSigs = Uml::st_ShowSig;
	m_bShowScope = true;
	m_bDrawAsCircle = false;
	m_Type = Uml::wt_Interface;
}

InterfaceWidgetData::~InterfaceWidgetData() {}

InterfaceWidgetData::InterfaceWidgetData(InterfaceWidgetData& Other) : UMLWidgetData(Other) {
	*this = Other;
}

InterfaceWidgetData & InterfaceWidgetData::operator=(InterfaceWidgetData& Other) {
	*((UMLWidgetData*)this) = (UMLWidgetData)Other;

	m_bShowOperations = Other.m_bShowOperations;

	m_ShowOpSigs = Other.m_ShowOpSigs;

	m_bShowScope = Other.m_bShowScope;

	m_bShowPackage = Other.m_bShowPackage;

	return *this;
}

bool InterfaceWidgetData::operator==(InterfaceWidgetData& Other) {
	if(!(*((UMLWidgetData*)this) == (UMLWidgetData)Other)) {
		return false;
	}

	if(m_bShowOperations != Other.m_bShowOperations) {
		return false;
	}

	if(m_ShowOpSigs != Other.m_ShowOpSigs) {
		return false;
	}

	if(m_bShowScope != Other.m_bShowScope) {
		return false;
	}

	if(m_bShowPackage != Other.m_bShowPackage) {
		return false;
	}

	return true;
}

long InterfaceWidgetData::getClipSizeOf() {
	long l_size = UMLWidgetData::getClipSizeOf();

	l_size += sizeof(m_bShowOperations) + sizeof((int)m_bShowPackage)
	          + sizeof(m_ShowOpSigs) + sizeof((int)m_bShowScope);

	return l_size;
}

bool InterfaceWidgetData::serialize(QDataStream *s, bool archive, int fileversion) {
	if(!UMLWidgetData::serialize(s, archive, fileversion)) {
		return false;
	}
	if(archive) {
		*s << m_bShowOperations
		<< m_ShowOpSigs
		<< (int)m_bShowScope
		<< (int)m_bShowPackage;
	} else {
		int ss = 0, so = 0, scope = 0, sp = 0;
		*s >> so >> ss >> scope >> sp;
		m_bShowOperations = so;
		m_ShowOpSigs = (Uml::Signature_Type)ss;
		m_bShowScope = (bool)scope;
		m_bShowPackage = (bool)sp;
	}
	return true;
}

bool InterfaceWidgetData::getShowOperations() {
	return m_bShowOperations;
}

void InterfaceWidgetData::setShowOperations( bool ShowOperations) {
	m_bShowOperations = ShowOperations;
}

Uml::Signature_Type InterfaceWidgetData::getShowOpSigs() {
	return m_ShowOpSigs;
}
void InterfaceWidgetData::setShowOpSigs( Uml::Signature_Type ShowOpSigs) {
	m_ShowOpSigs = ShowOpSigs;
}

bool InterfaceWidgetData::getShowScope() {
	return m_bShowScope;
}

void InterfaceWidgetData::setShowScope( bool ShowScope) {
	m_bShowScope = ShowScope;
}

bool InterfaceWidgetData::getShowPackage() {
	return m_bShowPackage;
}

void InterfaceWidgetData::setShowPackage(bool showPackage) {
	m_bShowPackage = showPackage;
}

bool InterfaceWidgetData::getDrawAsCircle() {
	return m_bDrawAsCircle;
}

void InterfaceWidgetData::setDrawAsCircle(bool drawAsCircle) {
	m_bDrawAsCircle = drawAsCircle;
}

void InterfaceWidgetData::print2cerr() {
	UMLWidgetData::print2cerr();
	if(m_bShowOperations) {
		kdDebug() << "m_bShowOperations = true" << endl;
	} else {
		kdDebug() << "m_bShowOperations = false" << endl;
	}
	if(m_bShowPackage) {
		kdDebug() << "m_bShowPackage = true" << endl;
	}
	else {
		kdDebug() << "m_bShowPackage = false" << endl;
	}
	if(m_bShowScope) {
		kdDebug() << "m_bShowScope = true" << endl;
	} else {
		kdDebug() << "m_bShowScope = false" << endl;
	}
	switch(m_ShowOpSigs) {
		case Uml::st_NoSig:
			kdDebug() << "m_ShowOpSigs = UMLObject::None" << endl;
			break;
		case Uml::st_ShowSig:
			kdDebug() << "m_ShowOpSigs = UMLObject::ShowSig" << endl;
			break;

		case Uml::st_SigNoScope:
			kdDebug() << "m_ShowOpSigs = UMLObject::SigNoScope" << endl;
			break;
		case Uml::st_NoSigNoScope:
			kdDebug() << "m_ShowOpSigs = UMLObject::NoSigNoScope" << endl;
			break;
	}
}

bool InterfaceWidgetData::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement conceptElement = qDoc.createElement("interfacewidget");
	bool status = UMLWidgetData::saveToXMI(qDoc, conceptElement);

	conceptElement.setAttribute("showoperations", m_bShowOperations);
	conceptElement.setAttribute("showopsigs", m_ShowOpSigs);
	conceptElement.setAttribute("showpackage", m_bShowPackage);
	conceptElement.setAttribute("showscope", m_bShowScope);
	conceptElement.setAttribute("drawascircle", m_bDrawAsCircle);
	qElement.appendChild(conceptElement);

	return status;
}

bool InterfaceWidgetData::loadFromXMI( QDomElement & qElement ) {
	if ( !UMLWidgetData::loadFromXMI(qElement) ) {
		return false;
	}
	QString showops = qElement.attribute("showoperations", "1");
	QString showopsigs = qElement.attribute("showopsigs", "600");
	QString showpackage = qElement.attribute("showpackage", "0");
	QString showscope = qElement.attribute("showscope", "0");
	QString drawascircle = qElement.attribute("drawascircle", "0");

	m_bShowOperations = (bool)showops.toInt();
	m_ShowOpSigs = (Uml::Signature_Type)showopsigs.toInt();
	m_bShowPackage = (bool)showpackage.toInt();
	m_bShowScope = (bool)showscope.toInt();
	m_bDrawAsCircle = (bool)drawascircle.toInt();

	return true;
}
