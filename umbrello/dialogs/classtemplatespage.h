/***************************************************************************
                          classtemplatespage.h
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
 
#ifndef CLASS_TEMPLATES_PAGE
#define CLASS_TEMPLATES_PAGE

#include "classtemplatesbase.h"
#include <qptrlist.h>
#include <qmap.h>
#include "dialogpage.h"
#include "../umltemplatelist.h"

class UMLClass;
class UMLDoc;
class QListViewItem;

namespace Umbrello {


/** @short A Page to display / change the templates of a UMLClass 
 *
 * @description A dialog Page to display / change the templates of a UMLClass
 * Changes will be made to the UMLClass when apply() is called, or inmediatly if
 * autoApply is true.
 * If the parent widget is null, the page will default to autoApply, but you can 
 * change this anytime.
 * @see also DialogPage
 *
*/
class ClassTemplatesPage : public  ClassTemplatesBase, public DialogPage
{
Q_OBJECT
public:
	/** Constructor
	* @param c The class being observed
	* @param doc The document (model) the class belongs to. 
	* @param parent The widget parent, normally a UmbrelloDialog or null
	* @param name   The name of the page*/
	ClassTemplatesPage(UMLClass *c, UMLDoc *doc, QWidget *parent, const char *name = 0 );
	
	virtual ~ClassTemplatesPage( );
	
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
	virtual void createTemplate( );
	virtual void editSelected( );
	virtual void deleteSelected( );
	virtual void itemSelected(QListViewItem *item);

signals:
	void pageModified( );

protected:
/** Apply changes made in the page to the UMLClass being observed */
	virtual void saveData();
	
	UMLClass *m_umlObject;
	UMLDoc   *m_doc;
	UMLTemplateList m_tempsList;
	QMap<QListViewItem*,UMLTemplate*> m_tempMap;
};

} //namespace Umbrello

#endif
