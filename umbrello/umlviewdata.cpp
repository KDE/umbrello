/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdatastream.h>
#include <qcolor.h>

#include <kdebug.h>
#include <klocale.h>

#include "activitywidgetdata.h"
#include "actorwidgetdata.h"
#include "associationwidgetdata.h"
#include "classwidgetdata.h"
#include "packagewidgetdata.h"
#include "componentwidgetdata.h"
#include "nodewidgetdata.h"
#include "artifactwidgetdata.h"
#include "interfacewidgetdata.h"
#include "floatingtextdata.h"
#include "notewidgetdata.h"
#include "boxwidgetdata.h"
#include "messagewidgetdata.h"
#include "objectwidgetdata.h"
#include "statewidgetdata.h"
#include "umlview.h"
#include "umlviewdata.h"
#include "umlwidgetdata.h"
#include "usecasewidgetdata.h"
#include "associationwidgetlist.h"
#include "associationwidget.h"

UMLViewData::UMLViewData() {
	m_nID = -1;
	m_Documentation = "";
	m_Name = "umlview";
	m_Type = dt_Undefined;
	m_View = NULL;
	m_nLocalID = 30000;
	m_bUseSnapToGrid = false;
	m_bUseSnapComponentSizeToGrid = false;
	m_bShowSnapGrid = false;
	m_nSnapX = 10;
	m_nSnapY = 10;
	m_nZoom = 100;
	m_nCanvasWidth = UMLView::defaultCanvasSize;
	m_nCanvasHeight = UMLView::defaultCanvasSize;
}

UMLViewData::UMLViewData( UMLViewData & other ) {
	*this = other;
}

UMLViewData::~UMLViewData() {}

UMLViewData & UMLViewData::operator=( UMLViewData & rhs ) {
	if( this == &rhs )
		return *this;

	m_nID = rhs.m_nID;
	m_Documentation = rhs.m_Documentation;
	m_Name = rhs.m_Name;
	m_Type = rhs.m_Type;
	m_nLocalID = rhs.m_nLocalID;
	m_Options = rhs.m_Options;

	return *this;
}

bool UMLViewData::operator==( UMLViewData & rhs ) {
	if( this == &rhs )
		return true;

	if( m_nID != rhs.m_nID )
		return false;

	if( m_Documentation != rhs.m_Documentation )
		return false;

	if( m_Name != rhs.m_Name )
		return false;

	if( m_Type != rhs.m_Type )
		return false;

	if( m_nLocalID != rhs.m_nLocalID )
		return false;

	return true;
}

QString UMLViewData::getDoc() {
	return m_Documentation;
}

void UMLViewData::setDoc( QString doc ) {
	m_Documentation = doc;
}

UMLView* UMLViewData::getView() {
	return m_View;
}

void UMLViewData::setView( UMLView* v ) {
	m_View = v;
}

QString UMLViewData::getName() {
	return m_Name;
}

void UMLViewData::setName( QString name ) {
	m_Name = name;
}

Uml::Diagram_Type UMLViewData::getType() {
	return m_Type;
}

void UMLViewData::setType( Uml::Diagram_Type type ) {
	m_Type = type;
}

QColor UMLViewData::getFillColor() {
	return m_Options.uiState.fillColor;
}

void UMLViewData::setFillColor( QColor color ) {
	m_Options.uiState.fillColor = color;
}

QColor UMLViewData::getLineColor() {
	return m_Options.uiState.lineColor;
}

void UMLViewData::setLineColor( QColor color ) {
	m_Options.uiState.lineColor = color;
}

int UMLViewData::getID() {
	return m_nID;
}

void UMLViewData::setID( int id ) {
	m_nID = id;
}

int UMLViewData::getZoom() {
	return m_nZoom;
}

void UMLViewData::setZoom(int zoom) {
	m_nZoom = zoom;
}

int UMLViewData::getCanvasHeight() {
	return m_nCanvasHeight;
}

void UMLViewData::setCanvasHeight(int height) {
	m_nCanvasHeight = height;
}

int UMLViewData::getCanvasWidth() {
	return m_nCanvasWidth;
}

void UMLViewData::setCanvasWidth(int width) {
	m_nCanvasWidth = width;
}

int UMLViewData::getUniqueID() {
	return --m_nLocalID;
}

bool UMLViewData::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement viewElement = qDoc.createElement( "diagram" );
	viewElement.setAttribute( "xmi.id", m_nID );
	viewElement.setAttribute( "name", m_Name );
	viewElement.setAttribute( "type", m_Type );
	viewElement.setAttribute( "documentation", m_Documentation );
	//optionstate uistate
	viewElement.setAttribute( "fillcolor", m_Options.uiState.fillColor.name() );
	viewElement.setAttribute( "linecolor", m_Options.uiState.lineColor.name() );
	viewElement.setAttribute( "usefillcolor", m_Options.uiState.useFillColor );
	viewElement.setAttribute( "font", m_Options.uiState.font.toString() );
	//optionstate classstate
	viewElement.setAttribute( "showattsig", m_Options.classState.showAttSig );
	viewElement.setAttribute( "showatts", m_Options.classState.showAtts);
	viewElement.setAttribute( "showopsig", m_Options.classState.showOpSig );
	viewElement.setAttribute( "showops", m_Options.classState.showOps );
	viewElement.setAttribute( "showpackage", m_Options.classState.showPackage );
	viewElement.setAttribute( "showscope", m_Options.classState.showScope );
	viewElement.setAttribute( "showstereotype", m_Options.classState.showStereoType );
	//misc
	viewElement.setAttribute( "localid", m_nLocalID );
	viewElement.setAttribute( "showgrid", m_bShowSnapGrid );
	viewElement.setAttribute( "snapgrid", m_bUseSnapToGrid );
	viewElement.setAttribute( "snapcsgrid", m_bUseSnapComponentSizeToGrid );
	viewElement.setAttribute( "snapx", m_nSnapX );
	viewElement.setAttribute( "snapy", m_nSnapY );
	viewElement.setAttribute( "zoom", m_nZoom );
	viewElement.setAttribute( "canvasheight", m_nCanvasHeight );
	viewElement.setAttribute( "canvaswidth", m_nCanvasWidth );
	//now save all the widgets
	UMLWidgetData * widgetData = 0;
	UMLWidgetDataListIt w_it( m_WidgetList );
	QDomElement widgetElement = qDoc.createElement( "widgets" );
	while( ( widgetData = w_it.current() ) ) {
		++w_it;
		widgetData -> saveToXMI( qDoc, widgetElement );
	}
	viewElement.appendChild( widgetElement );
	//now save the message widgets
	UMLWidgetDataListIt m_it( m_MessageList );
	QDomElement messageElement = qDoc.createElement( "messages" );
	while( ( widgetData = m_it.current() ) ) {
		++m_it;
		widgetData -> saveToXMI( qDoc, messageElement );
	}
	viewElement.appendChild( messageElement );
	//now save the associations
	QDomElement assocElement = qDoc.createElement( "associations" );
	if (m_View) {  // FIXME: m_View should really always be set upon getting here.
		AssociationWidgetList assocs;
		m_View->getAssocWidgets( assocs );
	  	AssociationWidget * assoc = 0;
		AssociationWidgetListIt a_it( assocs );
		while( ( assoc = a_it.current() ) ) {
			++a_it;
			AssociationWidgetData * assocData = (AssociationWidgetData*)assoc;
			assocData -> saveToXMI( qDoc, assocElement );
		}
	} else {
		// FIXME: This else part should never be entered.
		AssociationWidgetDataListIt a_it( m_AssociationList );
	  	AssociationWidgetData * assocData = 0;
		while( ( assocData = a_it.current() ) ) {
			++a_it;
			assocData -> saveToXMI( qDoc, assocElement );
		}
	}
	viewElement.appendChild( assocElement );
	qElement.appendChild( viewElement );
	return true;
}

bool UMLViewData::loadFromXMI( QDomElement & qElement ) {
	QString id = qElement.attribute( "xmi.id", "-1" );
	m_nID = id.toInt();
	if( m_nID == -1 )
		return false;
	m_Name = qElement.attribute( "name", "" );
	QString type = qElement.attribute( "type", "-1" );
	m_Documentation = qElement.attribute( "documentation", "" );
	QString localid = qElement.attribute( "localid", "0" );
	//optionstate uistate
	QString font = qElement.attribute( "font", "" );
	if( !font.isEmpty() )
		m_Options.uiState.font.fromString( font );
	QString fillcolor = qElement.attribute( "fillcolor", "" );
	QString linecolor = qElement.attribute( "linecolor", "" );
	QString usefillcolor = qElement.attribute( "usefillcolor", "0" );
	m_Options.uiState.useFillColor = (bool)usefillcolor.toInt();
	//optionstate classstate
	QString temp = qElement.attribute( "showattsig", "0" );
	m_Options.classState.showAttSig = (bool)temp.toInt();
	temp = qElement.attribute( "showatts", "0" );
	m_Options.classState.showAtts = (bool)temp.toInt();
	temp = qElement.attribute( "showopsig", "0" );
	m_Options.classState.showOpSig = (bool)temp.toInt();
	temp = qElement.attribute( "showops", "0" );
	m_Options.classState.showOps = (bool)temp.toInt();
	temp = qElement.attribute( "showpackage", "0" );
	m_Options.classState.showPackage = (bool)temp.toInt();
	temp = qElement.attribute( "showscope", "0" );
	m_Options.classState.showScope = (bool)temp.toInt();
	temp = qElement.attribute( "showstereotype", "0" );
	m_Options.classState.showStereoType = (bool)temp.toInt();
	//misc
	QString showgrid = qElement.attribute( "showgrid", "0" );
	m_bShowSnapGrid = (bool)showgrid.toInt();

	QString snapgrid = qElement.attribute( "snapgrid", "0" );
	m_bUseSnapToGrid = (bool)snapgrid.toInt();

	QString snapcsgrid = qElement.attribute( "snapcsgrid", "0" );
	m_bUseSnapComponentSizeToGrid = (bool)snapcsgrid.toInt();

	QString snapx = qElement.attribute( "snapx", "10" );
	m_nSnapX = snapx.toInt();

	QString snapy = qElement.attribute( "snapy", "10" );
	m_nSnapY = snapy.toInt();

	QString zoom = qElement.attribute( "zoom", "100" );
	m_nZoom = zoom.toInt();

	QString height = qElement.attribute( "canvasheight", QString("%1").arg(UMLView::defaultCanvasSize) );
	m_nCanvasHeight = height.toInt();

	QString width = qElement.attribute( "canvaswidth", QString("%1").arg(UMLView::defaultCanvasSize) );
	m_nCanvasWidth = width.toInt();

	m_Type = (Uml::Diagram_Type)type.toInt();
	if( !fillcolor.isEmpty() )
		m_Options.uiState.fillColor = QColor( fillcolor );
	if( !linecolor.isEmpty() )
		m_Options.uiState.lineColor = QColor( linecolor );
	m_nLocalID = localid.toInt();
	//load the widgets
	QDomNode node = qElement.firstChild();
	QDomElement element = node.toElement();
	if( !element.isNull() && element.tagName() != "widgets" )
		return false;
	if( !loadWidgetsFromXMI( element ) ) {
		kdWarning() << "failed umlviewdata load on widgets" << endl;
		return false;
	}

	//load the message widgets
	node = element.nextSibling();
	element = node.toElement();
	if( !element.isNull() && element.tagName() != "messages" )
		return false;
	if( !loadMessagesFromXMI( element ) ) {
		kdWarning() << "failed umlviewdata load on messages" << endl;
		return false;
	}

	//load the associations
	node = element.nextSibling();
	element = node.toElement();
	if( !element.isNull() && element.tagName() != "associations" )
		return false;
	if( !loadAssociationsFromXMI( element ) ) {
		kdWarning() << "failed umlviewdata load on associations" << endl;
		return false;
	}
	return true;
}

bool UMLViewData::loadWidgetsFromXMI( QDomElement & qElement ) {
	UMLWidgetData* widgetData = 0;
	QDomNode node = qElement.firstChild();
	QDomElement widgetElement = node.toElement();
	while( !widgetElement.isNull() ) {
		widgetData = loadWidgetFromXMI(widgetElement);
		if (!widgetData) {
			return false;
		}
		m_WidgetList.append( widgetData );
		node = widgetElement.nextSibling();
		widgetElement = node.toElement();
	}

	return true;
}

UMLWidgetData* UMLViewData::loadWidgetFromXMI(QDomElement& widgetElement) {
	UMLWidgetData* widgetData = 0;
	QString tag = widgetElement.tagName();
	if (tag == "UML:ActorWidget") {
		widgetData = new ActorWidgetData(getOptionState());
	} else if (tag == "UML:UseCaseWidget") {
		widgetData = new UseCaseWidgetData(getOptionState());
// Have ConceptWidget for backwards compatability
	} else if (tag == "UML:ClassWidget" || tag == "UML:ConceptWidget") {
		widgetData = new ClassWidgetData(getOptionState());
	} else if (tag == "packagewidget") {
		widgetData = new PackageWidgetData(getOptionState());
	} else if (tag == "componentwidget") {
		widgetData = new ComponentWidgetData(getOptionState());
	} else if (tag == "nodewidget") {
		widgetData = new NodeWidgetData(getOptionState());
	} else if (tag == "artifactwidget") {
		widgetData = new ArtifactWidgetData(getOptionState());
	} else if (tag == "interfacewidget") {
		widgetData = new InterfaceWidgetData(getOptionState());
	} else if (tag == "UML:StateWidget") {
		widgetData = new StateWidgetData(getOptionState());
	} else if (tag == "UML:NoteWidget") {
		widgetData = new NoteWidgetData(getOptionState());
	} else if (tag == "boxwidget") {
		widgetData = new BoxWidgetData();
	} else if (tag == "UML:FloatingTextWidget") {
		widgetData = new FloatingTextData();
	} else if (tag == "UML:ObjectWidget") {
		widgetData = new ObjectWidgetData(getOptionState());
	} else if (tag == "UML:ActivityWidget") {
		widgetData = new ActivityWidgetData(getOptionState());
	} else {
		kdWarning() << "Trying to create an unknown widget:" << tag << endl;
		return 0;
	}
	if (!widgetData->loadFromXMI(widgetElement)) {
		return 0;
	}
	return widgetData;
}

bool UMLViewData::loadMessagesFromXMI( QDomElement & qElement ) {
	MessageWidgetData * messageData = 0;
	QDomNode node = qElement.firstChild();
	QDomElement messageElement = node.toElement();
	while( !messageElement.isNull() ) {
		if( messageElement.tagName() == "UML:MessageWidget" ) {
			messageData = new MessageWidgetData(getOptionState());
			if( !messageData -> loadFromXMI( messageElement ) )
				return false;
			m_MessageList.append( messageData );
		}
		node = messageElement.nextSibling();
		messageElement = node.toElement();
	}
	return true;
}

bool UMLViewData::loadAssociationsFromXMI( QDomElement & qElement ) {
	AssociationWidgetData * assocData = 0;
	QDomNode node = qElement.firstChild();
	QDomElement assocElement = node.toElement();
	while( !assocElement.isNull() ) {
		if( assocElement.tagName() == "UML:AssocWidget" ) {
			assocData = new AssociationWidgetData();
			if( !assocData -> loadFromXMI( assocElement ) )
				return false;
			m_AssociationList.append( assocData );
		}
		node = assocElement.nextSibling();
		assocElement = node.toElement();
	}
	return true;
}





