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
#include "conceptwidgetdata.h"
#include "floatingtextdata.h"
#include "notewidgetdata.h"
#include "messagewidgetdata.h"
#include "objectwidgetdata.h"
#include "statewidgetdata.h"
#include "umlview.h"
#include "umlviewdata.h"
#include "umlwidgetdata.h"
#include "usecasewidgetdata.h"

UMLViewData::UMLViewData() {
	m_nID = -1;
	m_Documentation = "";
	m_Name = "umlview";
	m_Type = dt_Undefined;
	m_nLocalID = 30000;
	m_bUseSnapToGrid = false;
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

bool UMLViewData::serialize( QDataStream * stream, bool bArchive, int fileversion ) {
	bool bStatus = true;
	if( bArchive ) {
		int nType = m_Type;
		*stream	<<	nType
		<<	m_Name
		<<	m_nID
		<<	m_Documentation
		<<	m_nLocalID
		<<	m_Options.uiState.fillColor
		<<	m_Options.uiState.lineColor
		<<	m_Options.uiState.font
		<<	(int)m_Options.uiState.useFillColor
		<< m_nSnapX
		<< m_nSnapY
		<< m_nZoom
		<< m_nCanvasWidth
		<< m_nCanvasHeight
		<< (int)m_bShowSnapGrid
		<< (int)m_bUseSnapToGrid
		<< (int)m_Options.classState.showAttSig
		<< (int)m_Options.classState.showAtts
		<< (int)m_Options.classState.showOpSig
		<< (int)m_Options.classState.showOps
		<< (int)m_Options.classState.showPackage
		<< (int)m_Options.classState.showScope
		<< (int)m_Options.classState.showStereoType;
	} else {
		int nType = 0;
		int nSnapgrid = 0, nShowsnap = 0, nUseFC = 1, nTemp = 0;
		*stream	>>	nType
		>>	m_Name
		>>	m_nID;
		if (fileversion > 4)
			*stream >> m_Documentation;
		*stream >> m_nLocalID
		>>	m_Options.uiState.fillColor
		>>	m_Options.uiState.lineColor;
		if (fileversion > 4) {
			*stream	>> m_Options.uiState.font
			>> nUseFC
			>> m_nSnapX
			>> m_nSnapY
			>> m_nZoom
			>> m_nCanvasWidth
			>> m_nCanvasHeight
			>> nShowsnap
			>> nSnapgrid;
		}
		*stream >> nTemp;
		if (fileversion < 5)
			nTemp = 0;
		m_Options.classState.showAttSig = nTemp;
		if (fileversion > 4)
			*stream >> nTemp;
		m_Options.classState.showAtts = nTemp;
		if (fileversion > 4)
			*stream >> nTemp;
		m_Options.classState.showOpSig = nTemp;
		if (fileversion > 4)
			*stream >> nTemp;
		m_Options.classState.showOps = nTemp;
		if (fileversion > 4)
			*stream >> nTemp;
		m_Options.classState.showPackage = nTemp;
		if (fileversion > 4)
			*stream >> nTemp;
		m_Options.classState.showScope = nTemp;
		if (fileversion > 4)
			*stream >> nTemp;
		m_Options.classState.showStereoType = nTemp;
		m_Options.uiState.useFillColor = nUseFC;
		m_bUseSnapToGrid = nSnapgrid;
		m_bShowSnapGrid = nShowsnap;

		if (fileversion > 4)
			m_Type = (Uml::Diagram_Type)nType;
		else
			switch (nType)
			{
				case /* UCDIAGRAM */ 300 :
					m_Type = Uml::dt_UseCase;
					break;
				case /* COLLDIAGRAM */ 301 :
					m_Type = Uml::dt_Collaboration;
					break;
				case /* CONDIAGRAM */ 302 :
					m_Type = Uml::dt_Class;
					break;
				case /* SEQDIAGRAM */ 303 :
					m_Type = Uml::dt_Sequence;
					break;
				default:
					kdDebug() << "Unknown diagram type (" << nType << ")" << endl;
					return false;
			}
	}
	bStatus = serializeWidgets( stream, bArchive, fileversion );
	if( bStatus )
		bStatus = serializeMessages( stream, bArchive, fileversion );
	if( bStatus )
		bStatus = serializeAssociations( stream, bArchive, fileversion );
	return bStatus;
}


long UMLViewData::getClipSizeOf() {
	long l_size = 0;
	l_size += sizeof( int ); //type
	l_size += sizeof( m_Name ); //not allowed to be blank so ok not to test
	l_size += sizeof( m_nID );
	l_size += sizeof( m_nLocalID );
	l_size += sizeof( int ) * 4; //m_Options.classState - docwindow
	l_size += sizeof( int ) * 7;//m_Options.uiState
	l_size += sizeof( m_nSnapX );
	l_size += sizeof( m_nSnapY );
	l_size += sizeof( m_nZoom );
	l_size += sizeof( m_nCanvasWidth );
	l_size += sizeof( m_nCanvasHeight );
	l_size += sizeof( int) * 2; //m_bUseSnapToGrid m_bShowSnapGrid

	if( m_Documentation.length() == 0 )
		l_size += sizeof( Q_UINT32 );
	else
		l_size += ( sizeof( QChar ) * m_Documentation.length() );

	l_size += ( ( sizeof( int ) * 3 ) );//counts of each the three lists
	l_size += ( m_WidgetList.count() * sizeof( int ) ); // type of each widget as header (assoc. and message don't need it)

	UMLWidgetData * widgetData = 0;
	AssociationWidgetData * assocData = 0;
	//get size of each widget data item
	UMLWidgetDataListIt w_it( m_WidgetList );
	while( ( widgetData = w_it.current() ) ) {
		++w_it;
		l_size += widgetData -> getClipSizeOf();
	}

	//get size of each message widget data item
	UMLWidgetDataListIt m_it( m_MessageList );
	widgetData = 0;
	while( ( widgetData = m_it.current() ) ) {
		++m_it;
		l_size += widgetData -> getClipSizeOf();
	}

	//get size of each association data item
	AssociationWidgetDataListIt a_it( m_AssociationList );
	while( ( assocData = a_it.current() ) ) {
		++a_it;
		l_size += assocData -> getClipSizeOf();
	}
	return l_size;
}

QString UMLViewData::getDoc() {
	return m_Documentation;
}

void UMLViewData::setDoc( QString doc ) {
	m_Documentation = doc;
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

bool UMLViewData::serializeWidgets( QDataStream * stream, bool bArchive, int fileversion ) {
	int nCount = 0, nType;
	bool bStatus = true;
	UMLWidgetData * widgetData = 0;
	if( bArchive ) {
		nCount = m_WidgetList.count();
		*stream	<<	nCount;
		UMLWidgetDataListIt w_it( m_WidgetList );
		while( ( widgetData = w_it.current() ) ) {
			++w_it;
			nType = widgetData -> getType();
			*stream	<<	nType;
			bStatus = widgetData -> serialize( stream, bArchive, fileversion );
			if( !bStatus )
				return false;
		}
	} else {
		m_WidgetList.clear();
		if (fileversion > 4) {
			*stream	>>	nCount;
			for( int i = 0; i < nCount; i++ ) {
				*stream	>>	nType;
				Uml::UMLWidget_Type type = (Uml::UMLWidget_Type)nType;
				if( type == wt_UseCase ) {
					UseCaseWidgetData * uc = new UseCaseWidgetData(getOptionState() );
					widgetData = dynamic_cast<UMLWidgetData *>( uc );
				} else if( type == wt_Actor ) {
					ActorWidgetData * a = new ActorWidgetData(getOptionState() );
					widgetData = dynamic_cast<UMLWidgetData *>( a );
				} else if( type == wt_Class ) {
					ConceptWidgetData * c = new ConceptWidgetData(getOptionState() );
					widgetData = dynamic_cast<UMLWidgetData *>( c );
				} else if( type == wt_Object ) {
					ObjectWidgetData * o = new ObjectWidgetData(getOptionState() );
					widgetData = dynamic_cast<UMLWidgetData *>( o );
				} else if( type == wt_Note ) {
					NoteWidgetData * n = new NoteWidgetData(getOptionState() );
					widgetData = dynamic_cast<UMLWidgetData *>( n );
				} else if( type == wt_Text ) {
					FloatingTextData * ft = new FloatingTextData();
					widgetData = dynamic_cast<UMLWidgetData *>( ft );
				} else if( type == wt_State ) {
					StateWidgetData * s = new StateWidgetData(getOptionState() );
					widgetData = dynamic_cast<UMLWidgetData *>( s );
				} else if( type == wt_Activity ) {
					ActivityWidgetData * a = new ActivityWidgetData(getOptionState() );
					widgetData = dynamic_cast<UMLWidgetData *>( a );
				} else {
					kdDebug()<<"UMLViewData::error: trying to load bad widget type."<<endl;
					return false;
				}//end else
				bStatus = widgetData -> serialize( stream, bArchive, fileversion );
				if( !bStatus )
					return false;
				m_WidgetList.append( widgetData );
			}//end for
		} else {
			QString stype;
			*stream >> stype;
			while(stype != "_ENDWIDGETS_") {
				if( stype == "USECASE" ) {
					UseCaseWidgetData * uc = new UseCaseWidgetData(m_Options);
					widgetData = dynamic_cast<UMLWidgetData *>( uc );
				} else if( stype == "ACTOR" ) {
					ActorWidgetData * a = new ActorWidgetData(m_Options);
					widgetData = dynamic_cast<UMLWidgetData *>( a );
				} else if( stype == "CONCEPT" ) {
					ConceptWidgetData * c = new ConceptWidgetData(m_Options);
					widgetData = dynamic_cast<UMLWidgetData *>( c );
				} else if( stype == "OBJECT" ) {
					ObjectWidgetData * o = new ObjectWidgetData(m_Options);
					widgetData = dynamic_cast<UMLWidgetData *>( o );
				} else if( stype == "NOTE" ) {
					NoteWidgetData * n = new NoteWidgetData(m_Options);
					widgetData = dynamic_cast<UMLWidgetData *>( n );
				} else if( stype == "FLOATTEXT" ) {
					FloatingTextData * ft = new FloatingTextData();
					widgetData = dynamic_cast<UMLWidgetData *>( ft );
				} else {
					kdDebug()<<"UMLViewData::error: trying to load bad widget type. (" << stype << ")" <<endl;
					return false;
				}//end else
				bStatus = widgetData -> serialize( stream, bArchive, fileversion );
				if( !bStatus )
					return false;
				m_WidgetList.append( widgetData );
				*stream >> stype;
			}//end while
		}
	}//end else
	return true;
}

bool UMLViewData::serializeMessages( QDataStream * stream, bool bArchive, int fileversion ) {
	int nCount = 0;
	bool bStatus = true;
	UMLWidgetData * widgetData = 0;
	if( bArchive ) {
		nCount = m_MessageList.count();
		*stream	<<	nCount;
		UMLWidgetDataListIt m_it( m_MessageList );
		while( ( widgetData = m_it.current() ) ) {
			++m_it;
			bStatus = widgetData -> serialize( stream, bArchive, fileversion );
			if( !bStatus )
				return false;
		}
	} else {
		m_MessageList.clear();
		if (fileversion > 4) {
			*stream	>>	nCount;
			for( int i = 0; i < nCount; i++ ) {
				MessageWidgetData * messageData = new MessageWidgetData(getOptionState());
				bStatus = messageData -> serialize( stream, bArchive, fileversion );
				if( !bStatus )
					return false;
				m_MessageList.append( messageData );
			}
		} else {
			QString stype;
			*stream >> stype;
			while(stype != "_ENDMESSAGES_") {
				MessageWidgetData *mw = new MessageWidgetData(getOptionState());
				bStatus = mw -> serialize( stream, bArchive, fileversion );
				if( !bStatus )
					return false;
				m_MessageList.append( mw );
				*stream >> stype;
			}
		}
	}
	return true;
}

bool UMLViewData::serializeAssociations( QDataStream * stream, bool bArchive, int fileversion ) {
	int nCount = 0;
	bool bStatus = true;
	AssociationWidgetData * assocData = 0;
	if( bArchive ) {
		nCount = m_AssociationList.count();
		*stream	<<	nCount;
		AssociationWidgetDataListIt a_it( m_AssociationList );
		while( ( assocData = a_it.current() ) ) {
			++a_it;
			bStatus = assocData -> serialize( stream, bArchive, fileversion );
			if( !bStatus )
				return false;
		}
	} else {
		m_AssociationList.clear();
		if (fileversion > 4) {
			*stream	>>	nCount;
			for( int i = 0; i < nCount; i++ ) {
				assocData = new AssociationWidgetData();
				bStatus = assocData -> serialize( stream, bArchive, fileversion );
				if( !bStatus )
					return false;
				m_AssociationList.append( assocData );
			}
		} else {
			QString stype;
			*stream >> stype;
			while (stype != "_ENDASSOC_") {
				assocData = new AssociationWidgetData();
				bStatus = assocData -> serialize( stream, bArchive, fileversion );
				if( !bStatus )
					return false;
				m_AssociationList.append( assocData );
				*stream >> stype;
			}
		}
	}
	return true;
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
	AssociationWidgetData * assocData = 0;
	AssociationWidgetDataListIt a_it( m_AssociationList );
	QDomElement assocElement = qDoc.createElement( "associations" );
	while( ( assocData = a_it.current() ) ) {
		++a_it;
		assocData -> saveToXMI( qDoc, assocElement );
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
	if( font != "" )
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
	if( fillcolor != "" )
		m_Options.uiState.fillColor = QColor( fillcolor );
	if( linecolor != "" )
		m_Options.uiState.lineColor = QColor( linecolor );
	m_nLocalID = localid.toInt();
	//load the widgets
	QDomNode node = qElement.firstChild();
	QDomElement element = node.toElement();
	if( !element.isNull() && element.tagName() != "widgets" )
		return false;
	if( !loadWidgetsFromXMI( element ) )
		return false;

	//load the message widgets
	node = element.nextSibling();
	element = node.toElement();
	if( !element.isNull() && element.tagName() != "messages" )
		return false;
	if( !loadMessagesFromXMI( element ) )
		return false;

	//load the associations
	node = element.nextSibling();
	element = node.toElement();
	if( !element.isNull() && element.tagName() != "associations" )
		return false;
	if( !loadAssociationsFromXMI( element ) )
		return false;
	return true;
}

bool UMLViewData::loadWidgetsFromXMI( QDomElement & qElement ) {
	UMLWidgetData * widgetData = 0;
	QDomNode node = qElement.firstChild();
	QDomElement widgetElement = node.toElement();
	while( !widgetElement.isNull() ) {
		QString tag = widgetElement.tagName();
		if( tag == "UML:ActorWidget" ) {
			widgetData = new ActorWidgetData(getOptionState());
		} else if( tag == "UML:UseCaseWidget" ) {
			widgetData = new UseCaseWidgetData(getOptionState());
		} else if( tag == "UML:ConceptWidget" ) {
			widgetData = new ConceptWidgetData(getOptionState());
		} else if( tag == "UML:StateWidget" ) {
			widgetData = new StateWidgetData(getOptionState());
		} else if( tag == "UML:NoteWidget" ) {
			widgetData = new NoteWidgetData(getOptionState());
		} else if( tag == "UML:FloatingTextWidget" ) {
			widgetData = new FloatingTextData();
		} else if( tag == "UML:ObjectWidget" ) {
			widgetData = new ObjectWidgetData(getOptionState());
		} else if( tag == "UML:ActivityWidget" ) {
			widgetData = new ActivityWidgetData(getOptionState());
		} else {
			kdDebug()<<"Trying to create an unknown widget"<<endl;
			return false;
		}
		if( !widgetData -> loadFromXMI( widgetElement ) )
			return false;
		m_WidgetList.append( widgetData );
		node = widgetElement.nextSibling();
		widgetElement = node.toElement();
	}

	return true;
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





