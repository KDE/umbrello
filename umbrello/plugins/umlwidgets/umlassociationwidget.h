
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef UMBRELLO_UMLASSOCIATIONWIDGET_H
#define UMBRELLO_UMLASSOCIATIONWIDGET_H

#include "../../diagram/associationwidget.h"
#include <qcanvas.h>


namespace Umbrello{

class Diagram;
class DiagramWidget;
class AssociationEnd;

/** Graphic representation of a UML Association*/
class UMLAssociationWidget : public AssociationWidget
{
Q_OBJECT
public:
	/** Constructor
	* @param  diagram The diagram this path is in
	* @param  id The unique ID of the diagram element
	*/
	UMLAssociationWidget( Diagram *diagram, uint id, DiagramWidget *start, DiagramWidget *end, 
	                      UMLAssociation *association );
	
	/** Destructor */
	virtual ~UMLAssociationWidget();
	virtual void moveBy( int dx, int dy);
	
	/** Populate a context menu with items / actions for this element
	*  @param menu The popup menu to insert the actions in
	*/
	virtual void fillContextMenu(QPopupMenu &menu);
	virtual void moveHotSpotBy( int h, int dx, int dy );
	virtual void setPathPoints( const QPointArray &a );
public slots:
	virtual void widgetMoved( );
	virtual void umlObjectModified( );
protected:
	virtual void drawShape(QPainter& );
	virtual void createHead( );
	virtual void createTail( );
	
	AssociationEnd *m_head;
	AssociationEnd *m_tail;
	UMLAssociation *m_association;
};

} //end of namespace Umbrello

#endif  // UMBRELLO_UMLASSOCIATIONWIDGET_H
