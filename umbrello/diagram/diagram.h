 /*
  *  copyright (C) 2003-2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef UMBRELLO_DIAGRAM_H
#define UMBRELLO_DIAGRAM_H


#include <qcanvas.h>
#include <map>
#include <list>
#include <typeinfo>
#include <qptrlist.h>
#include "../umlnamespace.h"

class UMLDoc;
class UMLObject;
class UMLAssociation;
class QDropEvent;
class QDragEnterEvent;
class QPopupMenu;
class QPointArray;
class QRect;

namespace Umbrello{

class DiagramElement;
class DiagramWidget;
class DiagramView;


/** Diagram. Use a DiagramView to view/modify the contents of this diagram */
class Diagram : public QCanvas
{
Q_OBJECT

public:
	enum DiagramType {ClassDiagram = 402, UseCaseDiagram = 400, CollaborationDiagram = 401, 
			ActivityDiagram = 404, SequenceDiagram = 403 };
	
	Diagram( DiagramType type, UMLDoc *docparent, int id, const QString &name = QString::null);
	
	virtual ~Diagram();
	
	inline QString name() const;
	inline DiagramType diagramType() const;
	inline int getID() const;
	
	virtual void dragEnterEvent(QDragEnterEvent *e);
	virtual void dropEvent(QDropEvent *e);
	
	void registerElement( DiagramElement* );
	
	void fillContextMenu(QPopupMenu &menu) const;
	
	UMLDoc* document() const;
	
	void setItemsSelected(const QRect &rect, bool selected);
	
	void moveSelectedBy(int x, int y);
	
	
	inline QPen pen() const;
	inline QBrush brush() const;
	
	bool acceptType( const std::type_info& );
	
	DiagramView* createView( QWidget *parent );
	
	DiagramElement* firstDiagramElement( const QPoint& );
	DiagramWidget* firstDiagramWidget( const QPoint& );
public slots:

	DiagramView* createView();
	void selectAll();
	void deselectAll();
	void properties();
	void elementSelected(bool s);
	void elementDestroyed();
	
signals:
	void modified();
		
protected:
	bool canAcceptDrop(QDropEvent *e);

	DiagramType m_type;
	UMLDoc *m_doc;
	int m_id;
	QString m_name;
	
	QPen m_pen;
	QBrush m_brush;
	
	static std::map<DiagramType,std::list<const std::type_info*> > allowedTypes;

private:
	void initAllowedTypesMap();
	QPtrList<DiagramElement> m_elements;
	QPtrList<DiagramElement> m_selected;

};

// Inline functions
QString Diagram::name() const {return m_name;}
Diagram::DiagramType Diagram::diagramType() const {return m_type;}
int Diagram::getID() const    {return m_id;}
QPen Diagram::pen() const     {return m_pen;}
QBrush Diagram::brush() const {return m_brush;}


}



#endif
