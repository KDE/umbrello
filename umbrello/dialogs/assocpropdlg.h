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


#include <kdialogbase.h>
#include <kfontdialog.h>

#include "../umlnamespace.h"

class UMLAssociation;
class AssocRolePage;
class AssocGenPage;

/**
 * Dialog to display and change the properties of a UMLAssociation
 * @author Brian Thomas <Brian.A.Thomas@gsfc.nasa.gov>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AssocPropDlg : public KDialogBase {
	Q_OBJECT
public:
	enum Page { page_gen = 0, page_role, page_font };
	
	/**
	 *	Sets up a Association Properties Dialog.
	 *	@param	parent	The parent Widget of the AssocPropDlg
	 *	@param	a	The Association to display properties of.
	 *	@param	pageNum	The page to show first.
	 */
	AssocPropDlg(QWidget *parent, UMLAssociation *a, int pageNum = 0);

	/**
	 *	destructor
	 */
	virtual ~AssocPropDlg();

	/* accessor methods to underlying association */
	QString getName() const ;
	QString getRoleAName() const ;
	QString getDoc() const ;
	QString getRoleADoc() const ;
	QString getRoleBName() const ;
	QString getRoleBDoc() const ;
	QString getMultiA() const ;
	QString getMultiB() const ;
	Uml::Scope getVisibilityA() const ;
	Uml::Scope getVisibilityB() const ;
	Uml::Changeability_Type getChangeabilityA() const ;
	Uml::Changeability_Type getChangeabilityB() const ;

protected:
	void slotOk();
	void slotApply();
	void setupPages( );
	void setupFontPage();

private:

	UMLAssociation *m_assoc;
	
	AssocGenPage  *m_pGenPage;
	AssocRolePage *m_pRolePage;
	KFontChooser  *m_pChooser;
	
};

#endif /* ASSOCPROPDLG_H */ 

