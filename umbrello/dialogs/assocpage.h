/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef ASSOCPAGE_H
#define ASSOCPAGE_H

#include <qwidget.h>

#include "../umlobject.h"
#include "../association.h"
#include "../umlassociationlist.h"

#include "../umlview.h"
#include "../listpopupmenu.h"


/**
 * Displays a page with all the Associations in which a Classifier is involved
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AssocPage : public QWidget {
	Q_OBJECT
public:
	/**
	 *	Constructs an instance of AssocPage.
	 *
	 *	@param	parent	The parent of the page
	 *	@param	v	The view the UMLObject being represented.
	 *	@param	o	The UMLObject being represented
	 */
	AssocPage(QWidget *parent, UMLView * v, UMLObject * o);

	/**
	 *	destructor
	 */
	virtual ~AssocPage();
	
public slots:
	void slotDoubleClick(QListBoxItem * i);
	void slotRightButtonClicked(QListBoxItem */* item*/, const QPoint &/* p*/);
	void slotRightButtonPressed(QListBoxItem * item, const QPoint & p);
	void slotPopupMenuSel(int id);
	
	
private:
	/**
	 *	Fills the list box with the objects associations.
	 */
	void fillListBox();
	
	UMLObject *m_pObject;
	UMLAssociationList m_associations;
	
	
	UMLView * m_pView;
	QListBox * m_pAssocLB;
	QGroupBox * m_pAssocGB;
	
	ListPopupMenu * m_pMenu;
};

#endif
