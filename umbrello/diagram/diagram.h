/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef DIA_DIAGRAM_H
#define DIA_DIAGRAM_H


#include <qcanvas.h>
#include <map>
#include <list>
#include <typeinfo>
#include "../umlnamespace.h"

class UMLDoc;
class UMLObject;
class QDropEvent;
class QDragEnterEvent;
class QPopupMenu;


namespace Umbrello{

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
	
	/** Create a Widget which represents the UMLObject with id umlObjectID
	*   The widget will only be created if the diagram allows that kind of object
	*/
	void createUMLWidget( uint umlObjectID, const QPoint &pos);
	/** Create a Widget which represents the UMLObject obj
	*   The widget will only be created if the diagram allows that kind of object
	*/
	void createUMLWidget( UMLObject *obj, const QPoint &pos);
	
	/** Creates a Non-UML Widget in the Diagram. These widgets exist only
	* in the diagram (they do not exist in the UML Model (UMLDoc)
	* FIXME temp only: type = note, text, box, ellipse, etc
	**/
	void createCustomWidget( int type, const QPoint &pos);
	
	void fillContextMenu(QPopupMenu &menu) const;
	
	UMLDoc* document() const;
	
	void moveSelectedBy(int x, int y);
	
	
	inline QPen pen() const;
	inline QBrush brush() const;
	
	bool acceptType( const std::type_info& );

public slots:

	void selectAll();
	void deselectAll();
	void properties();
	
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

};

// Inline functions
QString Diagram::name() const {return m_name;}
Diagram::DiagramType Diagram::diagramType() const {return m_type;}
int Diagram::getID() const    {return m_id;}
QPen Diagram::pen() const     {return m_pen;}
QBrush Diagram::brush() const {return m_brush;}


}



#endif
