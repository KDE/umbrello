/***************************************************************************
                          templatepropertiespage.h
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
 
#ifndef TEMPLATE_PROPERTIES_PAGE
#define TEMPLATE_PROPERTIES_PAGE

#include "templatepropertiesbase.h"
#include "dialogpage.h"

class UMLTemplate;

namespace Umbrello{

/** @short A Page to display / change the properties of a UMLTemplate
 * 
 * @description Dialog page to display / change the properties of
 * an template: Name and  type.
 * If the parent widget is null, the page will default to autoApply, but you can
 * change this at any time. 
 * @see also DialogPage
 *
*/
class TemplatePropertiesPage : public  TemplatePropertiesBase, public DialogPage
{
Q_OBJECT
public:
	/** Constructor.
	 * @param c The UMLTemplate to observe
	 * @param parent The widget parent, normally a UmbrelloDialog or null
	 * @param name   The name of the page
	 */
	TemplatePropertiesPage(UMLTemplate *a, QWidget *parent = 0, const char *name = 0 );
	
	/** Destructor */
	virtual ~TemplatePropertiesPage( );
	
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
	
	UMLTemplate *m_umlObject;	
};

} //  namespace Umbrello

#endif // TEMPLATE_PROPERTIES_PAGE

