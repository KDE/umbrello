/***************************************************************************
                          umlobjectassociationspage.h
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
 
#ifndef UMLOBJECT_ASSOCIATIONS_PAGE
#define UMLOBJECT_ASSOCIATIONS_PAGE

#include "umlobjectassociationsbase.h"
#include "dialogpage.h"

class UMLCanvasObject;

namespace Umbrello{

/** @short A Page to display the associations of a UMLObject
 * 
 * @description Dialog page to display  the properties of all
 * Associations related to a particular object
 * @see also DialogPage
 */
class UMLObjectAssociationsPage : public  UMLObjectAssociationsBase, public DialogPage
{
Q_OBJECT
public:
	/** Constructor.
	 * @param o The UMLObject to observe
	 * @param parent The widget parent, normally a UmbrelloDialog or null
	 * @param name   The name of the page
	 */
	UMLObjectAssociationsPage( UMLCanvasObject *o, QWidget *parent = 0, const char *name = 0 );
	
	/** Destructor */
	virtual ~UMLObjectAssociationsPage( );
	
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
	/** Apply changes made in the page to the UMLClass being observed */
	virtual void saveData();
	
	UMLCanvasObject *m_umlObject;
};

} //  namespace Umbrello

#endif // UMLOBJECT_ASSOCIATIONS_PAGE

