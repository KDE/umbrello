/***************************************************************************
                          classattributespage.h
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
 
#ifndef CLASS_ATTRIBUTES_PAGE
#define CLASS_ATTRIBUTES_PAGE

#include "classattributesbase.h"
#include <qptrlist.h>
#include <qpixmap.h>
#include "dialogpage.h"

class UMLClass;
class UMLAttribute;
class UMLDoc;
class QListViewItem;

/** @short A Page to display / change the attributes of a UMLClass 
 *
 * @description A dialog Page to display / change the attributes of a UMLClass
 * Changes will be made to the UMLClass when apply() is called, or inmediatly if
 * autoApply is true.
 * If the parent widget is null, the page will default to autoApply, but you can 
 * change this anytime.
 * @see also DialogPage
 *
*/
class ClassAttributesPage : public  ClassAttributesBase, public DialogPage
{
Q_OBJECT
public:
	/** Constructor
	* @param c The class being observed
	* @param doc The document (model) the class belongs to. This is needed because
	*        class attributes are not created / destroyed by the class itself, but
	*        by the document
	* @param parent The widget parent, normally a UmbrelloDialog or null
	* @param name   The name of the page*/
	ClassAttributesPage(UMLClass *c, UMLDoc *doc, QWidget *parent, const char *name = 0 );
	
	virtual ~ClassAttributesPage( );
	
public slots:
	/** apply changes to the object being observed*/
	virtual void apply();
	/** reset changes and restore values from observed object*/	
	virtual void cancel();
	/** Inform the page that a field has been modified. Do not use (internal) */
	virtual void pageContentsModified();
	/** Load the widget data from the UMLObject. */
	virtual void loadData();
protected:	
	virtual void moveUp( );
	virtual void moveDown( );
	virtual void createAttribute( );
	virtual void editSelected( );
	virtual void deleteSelected( );
	virtual void itemSelected(QListViewItem *item);

signals:
	void pageModified( );

protected:
/** Apply changes made in the page to the UMLClass being observed */
	virtual void saveData();
	virtual void loadPixmaps();
	struct { QPixmap Public,
			 Protected,
			 Private;
		} m_pixmaps;
	
	UMLClass *m_umlObject;
	UMLDoc   *m_doc;
	QPtrList<UMLAttribute> m_newAtts;
	QPtrList<UMLAttribute> m_deletedAtts;
	QPtrList<UMLAttribute> m_modifiedAtts;
};

#endif
