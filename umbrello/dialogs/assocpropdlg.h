/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ASSOCPROPDLG_H
#define ASSOCPROPDLG_H

//kde class includes
#include <kdialogbase.h>
#include <kfontdialog.h>
#include "../associationwidget.h"

class AssocRolePage;
class AssocGenPage;
class UMLDoc;
// class ObjectWidget;
// class UMLObject;
// class UMLWidget;

/**
 * 	Based off of AssocPropDlg class
 *	@author Brian Thomas <Brian.A.Thomas@gsfc.nasa.gov>
 *	@version	1.0
 */
class AssocPropDlg : public KDialogBase {
	Q_OBJECT
public:

	/**
	 *	Sets up a Association Properties Dialog.
	 *	@param	parent	The parent of the AssocPropDlg
	 *	@param	a	The Association Widget to display properties of.
	 *	@param	pageNum	The page to show first.
	 */

	AssocPropDlg(QWidget *parent, AssociationWidget *a, int pageNum = 0);

	/**
	 *	Standard deconstructor
	 */
	~AssocPropDlg();

	/* accessor methods to underlying association */
	QString getName() { 
		return m_pAssoc->getName(); 
	} 

	QString getRoleAName() { 
		return m_pAssoc->getRoleNameA(); 
	}

	QString getDoc() { 
		return m_pAssoc->getDoc(); 
	}

	QString getRoleADoc() { 
		return m_pAssoc->getRoleADoc(); 
	}

	QString getRoleBName() { 
		return m_pAssoc->getRoleNameB(); 
	} 

	QString getRoleBDoc() { 
		return m_pAssoc->getRoleBDoc(); 
	}

	QString getMultiA() { 
		return m_pAssoc->getMultiA(); 
	} 

	QString getMultiB() { 
		return m_pAssoc->getMultiB(); 
	} 

	Scope getVisibilityA() { 
		return m_pAssoc->getVisibilityA(); 
	}

	Scope getVisibilityB() { 
		return m_pAssoc->getVisibilityB(); 
	}

	Changeability_Type getChangeabilityA() { 
		return m_pAssoc->getChangeabilityA(); 
	}

	Changeability_Type getChangeabilityB() { 
		return m_pAssoc->getChangeabilityB(); 
	}

	enum Page { page_gen = 0, page_role, page_font };

protected:
	void slotOk();
	void slotApply();
	void setupPages(AssociationWidget * assocWidget);
	void setupFontPage();
	void init();
private:

	AssocGenPage *m_pGenPage;
	AssocRolePage *m_pRolePage;
	KFontChooser * m_pChooser;
	AssociationWidget *m_pAssoc;

	UMLDoc *m_pDoc; // is this needed?? 

};

#endif /* ASSOCPROPDLG_H */ 

