
#ifndef DIA_UMLWIDGET_H
#define DIA_UMLWIDGET_H


#include "diagramelement.h"

class UMLObject;

namespace Umbrello{
class Diagram;
class AssociationWidget;


class UMLWidget : public DiagramElement
{
Q_OBJECT

public:
	UMLWidget( Diagram *diagram, uint id, UMLObject *object);
	
	virtual ~UMLWidget();
	
	inline UMLObject* umlObject( ) const;
	
	inline uint width() const;
	
	inline uint height() const;
	
	virtual QPointArray areaPoints() const;
	
//<FIXME FIXME FIXME>
	virtual void registerAssociation(AssociationWidget *a);
	virtual void getConnectingPoint(QPoint &conn1, QPoint &conn2, const QPoint &from);
//</FIXME>

	
public slots:
	virtual void umlObjectModified();
	
//	virtual void editProperties();
	
protected:
	virtual void drawShape(QPainter &) = 0;
	
	
	UMLObject *m_umlObject;
	uint m_width, m_height;
	
//	QPtrList<AssociationWidget> m_associations;

};

//inline functions
UMLObject* UMLWidget::umlObject() const { return m_umlObject;}
uint UMLWidget::width() const  { return m_width;  }
uint UMLWidget::height() const { return m_height; }

} //end of namespace Umbrello
#endif  // DIA_UMLWIDGET_H

