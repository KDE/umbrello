 /***************************************************************************
                               umlwidget.cpp
                             -------------------
    copyright            : (C) 2003 Luis De la Parra
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "umlwidget.h"
#include "../../umlobject.h"

#include <kdebug.h>


namespace Umbrello{

UMLWidget::UMLWidget( Diagram *diagram, uint id, UMLObject *object) :
	DiagramWidget(diagram, id), m_umlObject(object)
{
	connect(object, SIGNAL(modified()),this, SLOT(umlObjectModified()));
	connect(object, SIGNAL(destroyed()),this, SLOT(deleteLater()));
}

UMLWidget::~UMLWidget()
{
	hide();
	canvas()->update();
}




void UMLWidget::umlObjectModified()
{
	kdWarning()<<"UMLWidget::umlObjectModified() called! "<<endl
		<<"This method should be reimplemented in subclassed to update the widget"<<endl;
}

void UMLWidget::execDefaultAction()
{
	editProperties();
}

}

#include "umlwidget.moc"
