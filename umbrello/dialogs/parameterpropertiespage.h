/***************************************************************************
                          parameterpropertiespage.h
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
 
#ifndef PARAMETER_PROPERTIES_PAGE
#define PARAMETER_PROPERTIES_PAGE

#include "parameterpropertiesbase.h"
#include "dialogpage.h"

class UMLAttribute;

namespace Umbrello{

/** @short A Page to display / change the properties of a UMLAttribute
 * 
 * @description Dialog page to display / change the properties of
 * a parameter: Name, type, and initial value.
 * If the parent widget is null, the page will default to autoApply, but you can
 * change this at any time. 
 * @see also DialogPage
 *
*/
class ParameterPropertiesPage : public  ParameterPropertiesBase, public DialogPage
{
Q_OBJECT
public:
	/** Constructor.
	 * @param c The UMLAttribute (parameter)  to observe
	 * @param parent The widget parent, normally a UmbrelloDialog or null
	 * @param name   The name of the page
	 */
	ParameterPropertiesPage(UMLAttribute *a, QWidget *parent = 0, const char *name = 0 );
	
	/** Destructor */
	virtual ~ParameterPropertiesPage( );
	
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
	
	UMLAttribute *m_umlObject;	
};

} //  namespace Umbrello

#endif // PARAMETER_PROPERTIES_PAGE

