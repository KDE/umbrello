
 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "umlassociationtool.h"
#include "../../diagram/diagramview.h"
#include "../../diagram/diagram.h"
#include "../../diagram/diagramelement.h"
#include "../../diagram/diagramwidget.h"
#include "umlassociationwidget.h"
#include "umlwidget.h"
#include "../../umldoc.h"
#include "../../association.h"

#include <qcursor.h>
#include <qpopupmenu.h>
#include <kdebug.h>
#include <klocale.h>


namespace Umbrello
{

UMLAssociationTool::UMLAssociationTool( DiagramView  *view): AssociationTool(view)
{
	setName("umlassociationtool");
	setIcon("umlassociation");
	setToolTip(i18n("Creates associations between UML elements"));
}

UMLAssociationTool::~UMLAssociationTool()
{ }

void UMLAssociationTool::activate()
{
	setCursor( );
	AssociationTool::activate();
	kdDebug()<<"UMLAssociationTool tool is now active"<<endl;
}

void UMLAssociationTool::deactivate()
{
 	kdDebug()<<"Deactivating UMLAssociationTool tool"<<endl;
	AssociationTool::deactivate();
	view()->viewport()->setMouseTracking(false);


}

void UMLAssociationTool::setCursor( )
{
	view()->setCursor( QCursor(Qt::ArrowCursor ));
}

void UMLAssociationTool::setType( ToolType t )
{
	m_type = t;
	switch( m_type )
	{
		case Generalization:
			setName("generalizationtool");
			setIcon("generalization");
			setToolTip(i18n("Creates generalization relationships between UML elements"));
		break;
		case Composition:
			setName("compositiontool");
			setIcon("composition");
			setToolTip(i18n("Creates composition relationships between UML elements"));
		break;
		case Aggregation:
			setName("aggregationtool");
			setIcon("aggregation");
			setToolTip(i18n("Creates aggregation relationships between UML elements"));
		break;
		case Dependency:
			setName("dependencytool");
			setIcon("dependency");
			setToolTip(i18n("Creates dependencies relationships between UML elements"));
		break;
		case Association:
			setName("umlassociation");
			setIcon("association");
			setToolTip(i18n("Creates association relationships between UML elements"));
		break;
		case UniDiAssociation:
			setName("uniassociation");
			setIcon("uniassociation");
			setToolTip(i18n("Creates unidirectional association relationships between UML elements"));
		break;

	}
}


void UMLAssociationTool::createPath( )
{
	if( m_underMouse == 0 )
	{
		kdWarning()<<"AssociationTool::createPath( ) called with no widget under the mouse."
		           <<" - ignoring request"<<endl;
		return;
	}
	UMLAssociation *assoc(0);
	UMLWidget *wA = dynamic_cast<UMLWidget*>(m_startWidget);
	UMLWidget *wB = dynamic_cast<UMLWidget*>(m_underMouse);
	if( !wA || !wB )
	{
		return;
	}
	assoc = new UMLAssociation(diagram()->document());
	assoc->setObjectA( wA->umlObject() );
	assoc->setObjectB( wB->umlObject() );
	switch( m_type )
	{
		case Generalization:
			assoc->setAssocType( Uml::at_Generalization );
			break;
		case Composition:
			assoc->setAssocType( Uml::at_Composition );
			break;
		case Aggregation:
			assoc->setAssocType( Uml::at_Aggregation );
			break;
		case Dependency:
			assoc->setAssocType( Uml::at_Dependency );
			break;
		case Association:
			assoc->setAssocType( Uml::at_Association );
			break;
		case UniDiAssociation:
			assoc->setAssocType( Uml::at_UniAssociation );
			break;
	}

	UMLAssociationWidget *w;
	if (assoc) {
		w = new UMLAssociationWidget(diagram(),diagram()->document()->getUniqueID(),
	                                             wA, wB, assoc);
	} else {
		kdWarning() << "unitialised pointer `w'" << endl;
		w = 0;
	}

	QPointArray points( m_linePath.count() + 1 );
	points[0] = m_linePath.at(0)->startPoint();
	for( uint i = 0; i < m_linePath.count(); i++ )
	{
		points[i+1] = m_linePath.at(i)->endPoint();
	}
	w->setPathPoints(points);

	m_linePath.clear();
	m_startWidget = 0L;
	m_underMouse->hideHotSpots( );
	m_underMouse = 0L;

	w->show();
	diagram()->update();
}


}

