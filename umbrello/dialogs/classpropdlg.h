/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CLASSPROPDLG_H
#define CLASSPROPDLG_H

//kde class includes
#include <kdialogbase.h>
#include <kfontdialog.h>

class ClassGenPage;
class ClassAttPage;
class ClassOpsPage;
class ClassTemplatePage;
class AssocPage;
class ClassOptionsPage;
class UMLWidgetColorPage;

class ObjectWidget;
class UMLDoc;
class UMLObject;
class UMLWidget;

/**
 *	@author Paul Hensgen	<phensgen@techie.com>
 *	@version	1.0
 */
class ClassPropDlg : public KDialogBase {
	Q_OBJECT
public:
	/**
	 *	Sets up a ClassPropDlg.
	 *
	 *	@param	parent	The parent of the ClassPropDlg
	 *	@param	c	The UMLObject to display properties of.
	 *	@param	pageNum	The page to show first.
	 *	@param assoc	Determines whether to display associations
	 */
	ClassPropDlg(QWidget *parent, UMLObject *c, int pageNum = 0, bool assoc = false);

	/**
	 *	Sets up a ClassPropDlg.
	 *
	 *	@param	parent	The parent of the ClassPropDlg
	 *	@param	o	The ObjectWidget to display properties of.
	 */
	ClassPropDlg(QWidget *parent, ObjectWidget * o);

	/**
	 *	Sets up a ClassPropDlg.
	 *
	 *	@param	parent	The parent of the ClassPropDlg
	 *	@param	o	The UMLWidget to display properties of.
	 */
	ClassPropDlg(QWidget *parent, UMLWidget * o);


	/**
	 *	Standard deconstructor
	 */
	~ClassPropDlg();

	enum Page{page_gen = 0, page_att, page_op, page_template, 
		  page_assoc, page_options, page_color, page_font};

protected:
	void slotOk();
	void slotApply();
	void setupPages(UMLObject * c, bool assoc = false);
	void setupFontPage();
private:
	KFontChooser * m_pChooser;
	ClassGenPage * m_pGenPage;
	ClassAttPage *	m_pAttPage;
	ClassOpsPage * m_pOpsPage;
	ClassTemplatePage* m_pTemplatePage;
	AssocPage * m_pAssocPage;
	ClassOptionsPage * m_pOptionsPage;
	UMLWidgetColorPage * m_pColorPage;
	UMLDoc *m_pDoc;

	UMLObject *m_pObject;
	UMLWidget * m_pWidget;

	enum Page_Type{  pt_Object = 1,    //Show General page + Assoc. page if Class i.e. no colours page
	                 pt_ObjectWidget,  //Shows pages needed for an ObjectWidget
	                 pt_Widget         //Shows pages needed for any other widget
	              };
	Page_Type m_Type;
public slots:
	void slotUpdateChildObject(int id);
};

#endif
