/***************************************************************************
                          operationpropertiespage.h
                             -------------------
    copyright            : (C) 2003 Luis De la Parra
  Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef OPERATION_PROPERTIES_PAGE
#define OPERATION_PROPERTIES_PAGE

#include "dialogpage.h"
#include "operationpropertiesbase.h"

class UMLOperation;
class UMLAttribute;
class UMLDoc;

/** 
 * @short A Page to display / change the properties of a UMLOperation 
 * 
 * @description Dialog page to display / change the  properties of
 * an operation: Name, type, visibility, parameters, etc
 * If the parent widget is null, the page will default to autoApply, but you can
 * change this at any time. 
 * @see also DialogPage
 */
class OperationPropertiesPage : public  OperationPropertiesBase, public DialogPage
{
Q_OBJECT
public:
	/** Constructor.
	 * @param c The UMLOperation to observe
	 * @param parent The widget parent, normally a UmbrelloDialog or null
	 * @param name   The name of the page
	 */
	OperationPropertiesPage(UMLOperation *c, UMLDoc *m_doc, QWidget *parent = 0, const char *name = 0 );
	
	/** Destructor */
	virtual ~OperationPropertiesPage( );
	
public slots:
	/** apply changes to the object being observed*/
	virtual void apply();
	/** reset changes and restore values from observed object*/	
	virtual void cancel();
	/** Inform the page that a field has been modified. Do not use (internal) */
	virtual void pageContentsModified();
	/** Load the widget data from the UMLObject. */
	virtual void loadData();
signals:
	void pageModified( );
protected:
	virtual void moveUp( );
	virtual void moveDown( );
	virtual void createParameter( );
	virtual void editSelected( );
	virtual void deleteSelected( );
	virtual void itemSelected(QListViewItem *item);

	/** Apply changes made in the page to the UMLOperation being observed */
	virtual void saveData();
	virtual void loadPixmaps();
	struct { QPixmap Public,
			 Protected,
			 Private;
		} m_pixmaps;
	
	UMLOperation *m_umlObject;
	UMLDoc *m_doc;
	QPtrList<UMLAttribute> m_paramList;
	QMap<QListViewItem*,UMLAttribute*> m_paramMap;
};

#endif
