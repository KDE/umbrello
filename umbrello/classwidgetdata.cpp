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

#include "classwidgetdata.h"

ClassWidgetData::ClassWidgetData(SettingsDlg::OptionState optionState):UMLWidgetData(optionState) {
	m_bShowAttributes = true;
	m_bShowOperations = true;
	m_bShowPackage = false;
	m_bShowStereotype = false;
	m_ShowOpSigs = Uml::st_ShowSig;
	m_ShowAttSigs = Uml::st_ShowSig;
	m_bShowScope = true;
	m_Type = Uml::wt_Class;
}

ClassWidgetData::~ClassWidgetData() {}

ClassWidgetData::ClassWidgetData(ClassWidgetData & Other) : UMLWidgetData(Other) {
	*this = Other;
}

ClassWidgetData & ClassWidgetData::operator=(ClassWidgetData & Other) {
	*((UMLWidgetData*)this) = (UMLWidgetData)Other;

	m_bShowAttributes = Other.m_bShowAttributes;

	m_bShowOperations = Other.m_bShowOperations;

	m_ShowOpSigs = Other.m_ShowOpSigs;

	m_bShowScope = Other.m_bShowScope;

	m_ShowAttSigs = Other.m_ShowAttSigs;

	m_bShowPackage = Other.m_bShowPackage;

	m_bShowStereotype = Other.m_bShowStereotype;

	return *this;
}

bool ClassWidgetData::operator==(ClassWidgetData & Other) {
	if(!(*((UMLWidgetData*)this) == (UMLWidgetData)Other)) {
		return false;
	}
	if(m_bShowAttributes != Other.m_bShowAttributes) {
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

	if(m_ShowAttSigs != Other.m_ShowAttSigs) {
		return false;
	}

	if(m_bShowPackage != Other.m_bShowPackage) {
		return false;
	}

	if(m_bShowStereotype != Other.m_bShowStereotype) {
		return false;
	}

	return true;
}

/** Returns the amount of bytes needed to serialize this object
If the serialization method of this class is changed this function will have to be CHANGED TOO
This function is used by the Copy and Paste Functionality
The Size in bytes of a serialized QString Object is long sz:
		if ( (sz =str.length()*sizeof(QChar)) && !(const char*)str.unicode() )
		{





			sz = size of Q_UINT32; //  typedef unsigned int	Q_UINT32;		// 32 bit unsigned
		}
	This calculation is valid only for QT 2.1.x or superior, this is totally incompatible with QT 2.0.x or QT 1.x or inferior
	That means the copy and paste functionality will work on with QT 2.1.x or superior */
long ClassWidgetData::getClipSizeOf() {
	long l_size = UMLWidgetData::getClipSizeOf();
	//Q_UINT32 tmp; //tmp is used to calculate the size of each serialized null string

	l_size += sizeof(m_bShowAttributes) + sizeof(m_bShowOperations) + sizeof((int)m_bShowPackage)
	          + sizeof((int)m_bShowStereotype) + sizeof(m_ShowOpSigs) + sizeof(m_ShowAttSigs)
	          + sizeof((int)m_bShowScope);

	return l_size;
}

/** No descriptions */
bool ClassWidgetData::serialize(QDataStream *s, bool archive, int fileversion) {
	if(!UMLWidgetData::serialize(s, archive, fileversion)) {
		return false;
	}
	if(archive) {
		*s << m_bShowAttributes
		<< m_bShowOperations
		<< m_ShowOpSigs
		<< m_ShowAttSigs
		<< (int)m_bShowScope
		<< (int)m_bShowPackage
		<< (int)m_bShowStereotype;
	} else {
		int ss = 0, sa = 0, so = 0, sat = 0, scope = 0, si = 0, sp = 0;
		*s >> sa >> so >> ss >> sat >> scope >> sp >> si;
		m_bShowAttributes = sa;
		m_bShowOperations = so;
		m_ShowOpSigs = (Uml::Signature_Type)ss;
		m_ShowAttSigs = (Uml::Signature_Type)sat;
		m_bShowScope = (bool)scope;
		m_bShowPackage = (bool)sp;
		m_bShowStereotype = (bool)si;
	}

	return true;
}
/** Read property of bool m_bShowAttributes. */
bool ClassWidgetData::getShowAttributes() {
	return m_bShowAttributes;
}

/** Write property of bool m_bShowAttributes. */
void ClassWidgetData::setShowAttributes( bool ShowAttributes) {
	m_bShowAttributes = ShowAttributes;
}

/** Read property of bool m_bShowOperations. */
bool ClassWidgetData::getShowOperations() {
	return m_bShowOperations;
}

/** Write property of bool m_bShowOperations. */
void ClassWidgetData::setShowOperations( bool ShowOperations) {
	m_bShowOperations = ShowOperations;
}

/** Read property of ClassWidgetData::SigType m_ShowOpSigs. */
Uml::Signature_Type ClassWidgetData::getShowOpSigs() {
	return m_ShowOpSigs;
}
/** Write property of ClassWidgetData::SigType m_ShowOpSigs. */
void ClassWidgetData::setShowOpSigs( Uml::Signature_Type ShowOpSigs) {
	m_ShowOpSigs = ShowOpSigs;
}

/** Read property of ClassWidgetData::SigType m_ShowAttSigs. */
Uml::Signature_Type ClassWidgetData::getShowAttSigs() {
	return m_ShowAttSigs;
}

/** Write property of ClassWidgetData::SigType m_ShowAttSigs. */
void ClassWidgetData::setShowAttSigs( Uml::Signature_Type ShowAttSigs) {
	m_ShowAttSigs = ShowAttSigs;
}

/** Read property of bool m_bShowScope. */
bool ClassWidgetData::getShowScope() {
	return m_bShowScope;
}

/** Write property of bool m_bShowScope. */
void ClassWidgetData::setShowScope( bool ShowScope) {
	m_bShowScope = ShowScope;
}

/** Read property of bool m_bShowPackage. */
bool ClassWidgetData::getShowPackage() {
	return m_bShowPackage;
}

/** Write property of bool m_bShowPackage. */
void ClassWidgetData::setShowPackage( bool ShowPackage) {
	m_bShowPackage = ShowPackage;
}

/** Read property of bool m_bShowStereotype. */
bool ClassWidgetData::getShowStereotype() {
	return m_bShowStereotype;
}

/** Write property of bool m_bShowStereotype. */
void ClassWidgetData::setShowStereotype( bool ShowStereotype) {
	m_bShowStereotype = ShowStereotype;
}

/** No descriptions */
void ClassWidgetData::print2cerr() {
	UMLWidgetData::print2cerr();
	if(m_bShowAttributes) {
		kdDebug() << "m_bShowAttributes = true" << endl;
	} else {
		kdDebug() << "m_bShowAttributes = false" << endl;
	}
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
	if(m_bShowStereotype) {
		kdDebug() << "m_bShowStereotype = true" << endl;
	} else {
		kdDebug() << "m_bShowStereotype = false" << endl;
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
	switch(m_ShowAttSigs) {
		case Uml::st_NoSig:
			kdDebug() << "m_ShowAttSigs = UMLObject::None" << endl;
			break;
		case Uml::st_ShowSig:
			kdDebug() << "m_ShowAttSigs = UMLObject::ShowSig" << endl;
			break;
		case Uml::st_SigNoScope:
			kdDebug() << "m_ShowAttSigs = UMLObject::SigNoScope" << endl;
			break;
		case Uml::st_NoSigNoScope:
			kdDebug() << "m_ShowAttSigs = UMLObject::NoSigNoScope" << endl;
			break;
	}
}

bool ClassWidgetData::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement conceptElement = qDoc.createElement( "UML:ClassWidget" );
	bool status = UMLWidgetData::saveToXMI( qDoc, conceptElement );
	conceptElement.setAttribute( "showattributes", m_bShowAttributes );
	conceptElement.setAttribute( "showoperations", m_bShowOperations );
	conceptElement.setAttribute( "showattsigs", m_ShowAttSigs );
	conceptElement.setAttribute( "showopsigs", m_ShowOpSigs );
	conceptElement.setAttribute( "showpackage", m_bShowPackage );
	conceptElement.setAttribute( "showstereotype", m_bShowStereotype );
	conceptElement.setAttribute( "showscope", m_bShowScope );
	qElement.appendChild( conceptElement );
	return status;
}

bool ClassWidgetData::loadFromXMI( QDomElement & qElement ) {
	if( !UMLWidgetData::loadFromXMI( qElement ) )
		return false;
	QString showatts = qElement.attribute( "showattributes", "1" );
	QString showops = qElement.attribute( "showoperations", "1" );
	QString showattsigs = qElement.attribute( "showattsigs", "600" );
	QString showopsigs = qElement.attribute( "showopsigs", "600" );
	QString showpackage = qElement.attribute( "showpackage", "0" );
	QString showstereo = qElement.attribute( "showstereotype", "0" );
	QString showscope = qElement.attribute( "showscope", "0" );

	m_bShowAttributes = (bool)showatts.toInt();
	m_bShowOperations = (bool)showops.toInt();
	m_ShowAttSigs = (Uml::Signature_Type)showattsigs.toInt();
	m_ShowOpSigs = (Uml::Signature_Type)showopsigs.toInt();
	m_bShowPackage = (bool)showpackage.toInt();
	m_bShowStereotype = (bool)showstereo.toInt();
	m_bShowScope = (bool)showscope.toInt();

	return true;
}
