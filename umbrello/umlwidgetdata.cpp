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

#include "umlwidgetdata.h"
#include "associationwidget.h"

UMLWidgetData::UMLWidgetData() {
	m_bUseFillColor = true;
	m_nId = -1;
	m_Type = wt_UMLWidget;
	m_nX = 0;
	m_nY = 0;
	m_nWidth = 0;
	m_nHeight = 0;
	m_FillColour = QColor(0, 0, 0);
	m_LineColour = QColor(0, 0, 0);
	m_bUsesDiagramFillColour = true;
	m_bUsesDiagramLineColour = true;
	m_bUsesDiagramUseFillColour = true;
}

UMLWidgetData::UMLWidgetData(SettingsDlg::OptionState optionState) {
	m_bUseFillColor = true;
	m_nId = -1;
	m_Type = wt_UMLWidget;
	m_nX = 0;
	m_nY = 0;
	m_nWidth = 0;
	m_nHeight = 0;

	m_FillColour = optionState.uiState.fillColor;
	m_LineColour = optionState.uiState.lineColor;
	m_bUsesDiagramFillColour = true;
	m_bUsesDiagramLineColour = true;
	m_bUsesDiagramUseFillColour = true;
}

UMLWidgetData::UMLWidgetData(UMLWidgetData &Other) {
	*this = Other;
}

UMLWidgetData::~UMLWidgetData() {
}

UMLWidgetData & UMLWidgetData::operator=(const UMLWidgetData & Other) {
	m_bUseFillColor = Other.m_bUseFillColor;
	m_nId = Other.m_nId;
	m_Type = Other.m_Type;
	m_nX = Other.m_nX;
	m_nY = Other.m_nY;
	m_Assocs = Other.m_Assocs;
	m_Font = Other.m_Font;
	m_nHeight = Other.m_nHeight;
	m_nWidth = Other.m_nWidth;
	m_bUsesDiagramFillColour = Other.m_bUsesDiagramFillColour;
	m_bUsesDiagramLineColour = Other.m_bUsesDiagramLineColour;
	m_bUsesDiagramUseFillColour = Other.m_bUsesDiagramUseFillColour;
	m_LineColour = Other.m_LineColour;
	m_FillColour = Other.m_FillColour;
	return *this;
}

bool UMLWidgetData::operator==(const UMLWidgetData & Other) {
	if( this == &Other )
		return true;

	if(m_Type != Other.m_Type) {
		return false;
	}

	if(m_Assocs.count() != Other.m_Assocs.count()) {
		return false;
	}

	AssociationWidgetListIt assoc_it( m_Assocs );
	AssociationWidgetListIt assoc_it2( Other.m_Assocs );
	AssociationWidget * assoc = 0, *assoc2 = 0;
	while ( ((assoc=assoc_it.current()) != 0) &&  ((assoc2=assoc_it2.current()) != 0)) {
		++assoc_it;
		++assoc_it2;
		if(!(*assoc == *assoc2)) {
			return false;
		}
	}
	return true;
	//NOTE:  In the comparison tests we are going to do, we don't need these values.
	//They will actually stop things functioning correctly so if you change these, be aware of that.
	/*	if(m_bUseFillColor != Other.m_bUseFillColor)
		{
			return false;
		}*/
	/*	if(m_nId != Other.m_nId)
		{
			return false;
		}    */
	/*	if( m_Font != Other.m_Font )
			return false;       */
	/*	if(m_nX  != Other.m_nX)
		{
			return false;
		}
		if(m_nY != Other.m_nY)
		{
			return false;
		} */
}

Uml::UMLWidget_Type UMLWidgetData::getType() {
	return m_Type;
}

void UMLWidgetData::setType( UMLWidget_Type Type) {
	m_Type = Type;
}

int UMLWidgetData::getX() {
	return m_nX;
}

void UMLWidgetData::setX( int X) {
	m_nX = X;
}

int UMLWidgetData::getY() {
	return m_nY;
}

void UMLWidgetData::setY( int Y) {
	m_nY = Y;
}

int UMLWidgetData::getId() {


	return m_nId;
}

void UMLWidgetData::setId( int Id) {
	m_nId = Id;
}

bool UMLWidgetData::getUseFillColor() {
	return m_bUseFillColor;
}

void UMLWidgetData::setUseFillColor( bool UseFillColor) {
	m_bUseFillColor = UseFillColor;
}

QColor UMLWidgetData::getFillColour() {
	return m_FillColour;
}

void UMLWidgetData::setFillColour(QColor colour) {
	m_FillColour = colour;
}

QColor UMLWidgetData::getLineColour() {
	return m_LineColour;
}

void UMLWidgetData::setLineColour(QColor colour) {
	m_LineColour = colour;
}

bool UMLWidgetData::serialize(QDataStream *s, bool archive, int fileversion) {
	if(archive) {
		*s	<< m_Type
		<< m_nX
		<< m_nY
		<< m_nId
		<< m_bUseFillColor
		<< m_Font
		<< m_nHeight
		<< m_nWidth;
	} else {
		int nType;
		*s	>> nType
		>> m_nX
		>> m_nY
		>> m_nId
		>> (int)m_bUseFillColor;
		if (fileversion > 4) {
			m_Type = (Uml::UMLWidget_Type)nType;
			*s >> m_Font
			>> m_nHeight
			>> m_nWidth;
		} else
			switch (nType)
			{
				case /* ACTOR */ 100 :
					m_Type = Uml::wt_Actor;
					break;
				case /* USECASE */ 101 :
					m_Type = Uml::wt_UseCase;
					break;
				case /* CONCEPTW */ 104 :
				case /* CONCEPT */ 102 :
					m_Type = Uml::wt_Class;
					break;
				case /* OBJECT */ 103 :
					m_Type = Uml::wt_Object;
					break;
				case /* MESSAGEW */ 105:
					m_Type = Uml::wt_Message;
					break;
				case /* MESSAGET */ 106:
					m_Type = Uml::wt_Text;
					break;
				default:
					m_Type = Uml::wt_UMLWidget;
					kdDebug() << "Unknown widget type (" << nType << ")" << endl;
					return false;
			}
	}
	return true;
}

uint UMLWidgetData::getNumAssoc() {
	return m_Assocs.count();
}

long UMLWidgetData::getClipSizeOf() {
	return sizeof(m_Type) +
	       sizeof(m_nId) +
	       sizeof(m_nX) +
	       sizeof(m_nY) +
	       sizeof(m_bUseFillColor) +
	       sizeof( m_Font ) +
	       sizeof( m_nHeight ) +
	       sizeof( m_nWidth );
}
/** Prints the data members to standard error */
void UMLWidgetData::print2cerr() {
	if(m_bUseFillColor) {
		kdDebug() << "m_bUseFillColor = " << "true" << endl;
	} else {
		kdDebug() << "m_bUseFillColor = " << "false" << endl;
	}
	kdDebug() << "m_nId = " << m_nId << endl;
	kdDebug() << "m_nNumAssoc = " << m_Assocs.count() << endl;
	QString stype;
	switch(m_Type) {
		case wt_Actor:
			stype = "ACTOR";
			break;
		case wt_UseCase:
			stype = "USECASE";
			break;
		case wt_Class:
			stype = "CONCEPT";
			break;
		case wt_Text:
			stype = "FLOATTEXT";
			break;
		case wt_Note:
			stype = "NOTE";
			break;
		case wt_Box:
			stype = "BOX";
			break;
		case wt_Object:
			stype = "OBJECT";
			break;
		case wt_Message:
			stype = "MESSAGEW";
			break;
		default:
			stype = "UNDEFINED";
			break;
	}
	kdDebug() << "m_Type = " << stype <<endl;
	kdDebug() << "m_nX = " << m_nX << endl;
	kdDebug() << "m_nY = " << m_nY << endl;
}

void UMLWidgetData::addAssoc(AssociationWidget* pAssoc) {
	if(pAssoc) {
		m_Assocs.append(pAssoc);
	}
}

void UMLWidgetData::removeAssoc(AssociationWidget* pAssoc) {
	if(pAssoc) {
		m_Assocs.remove(pAssoc);
	}
}

bool UMLWidgetData::getUsesDiagramFillColour() {
	return m_bUsesDiagramFillColour;
}

bool UMLWidgetData::getUsesDiagramLineColour() {
	return m_bUsesDiagramLineColour;
}

bool UMLWidgetData::getUsesDiagramUseFillColour() {
	return m_bUsesDiagramUseFillColour;
}

void UMLWidgetData::setUsesDiagramFillColour(bool usesDiagramFillColour) {
	m_bUsesDiagramFillColour = usesDiagramFillColour;
}

void UMLWidgetData::setUsesDiagramLineColour(bool usesDiagramLineColour) {
	m_bUsesDiagramLineColour = usesDiagramLineColour;
}

void UMLWidgetData::setUsesDiagramUseFillColour(bool usesDiagramUseFillColour) {
	m_bUsesDiagramUseFillColour = usesDiagramUseFillColour;
}

bool UMLWidgetData::saveToXMI( QDomDocument & /*qDoc*/, QDomElement & qElement ) {
	/*
	  Call after required actions in child class.
	  Type must be set in the child class.
	*/
	qElement.setAttribute( "xmi.id", m_nId );
	qElement.setAttribute( "font", m_Font.toString() );
	qElement.setAttribute( "usefillcolor", m_bUseFillColor );
	qElement.setAttribute( "x", m_nX );
	qElement.setAttribute( "y", m_nY );
	qElement.setAttribute( "width", m_nWidth );
	qElement.setAttribute( "height", m_nHeight );
	qElement.setAttribute( "usesdiagramfillcolour", m_bUsesDiagramFillColour );
	qElement.setAttribute( "usesdiagramlinecolour", m_bUsesDiagramLineColour );
	qElement.setAttribute( "usesdiagramusefillcolour", m_bUsesDiagramUseFillColour );
	if (m_bUsesDiagramFillColour) {
		qElement.setAttribute( "fillcolour", "none" );
	} else {
		qElement.setAttribute( "fillcolour", m_FillColour.name() );
	}
	if (m_bUsesDiagramLineColour) {
		qElement.setAttribute( "linecolour", "none" );
	} else {
		qElement.setAttribute( "linecolour", m_LineColour.name() );
	}
	return true;
}

bool UMLWidgetData::loadFromXMI( QDomElement & qElement ) {
	QString id = qElement.attribute( "xmi.id", "-1" );
	QString font = qElement.attribute( "font", "" );
	QString usefillcolor = qElement.attribute( "usefillcolor", "1" );
	QString x = qElement.attribute( "x", "0" );
	QString y = qElement.attribute( "y", "0" );
	QString h = qElement.attribute( "height", "0" );
	QString w = qElement.attribute( "width", "0" );
	QString fillColour = qElement.attribute( "fillcolour", "none" );
	QString lineColour = qElement.attribute( "linecolour", "none" );
	QString usesDiagramFillColour = qElement.attribute( "usesdiagramfillcolour", "1" );
	QString usesDiagramLineColour = qElement.attribute( "usesdiagramlinecolour", "1" );
	QString usesDiagramUseFillColour = qElement.attribute( "usesdiagramusefillcolour", "1" );

	m_nId = id.toInt();
	if( !font.isEmpty() ) {
		m_Font.fromString( font );
	}
	m_bUseFillColor = (bool)usefillcolor.toInt();
	m_bUsesDiagramFillColour = (bool)usesDiagramFillColour.toInt();
	m_bUsesDiagramLineColour = (bool)usesDiagramLineColour.toInt();
	m_bUsesDiagramUseFillColour = (bool)usesDiagramUseFillColour.toInt();
	m_nX = x.toInt();
	m_nY = y.toInt();
	m_nWidth = w.toInt();
	m_nHeight = h.toInt();
	if (fillColour != "none") {
		m_FillColour = QColor(fillColour);
	}
	if (lineColour != "none") {
		m_LineColour = QColor(lineColour);
	}
	return true;
}


