/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DIAGRAMPRINTPAGE_H
#define DIAGRAMPRINTPAGE_H

#include <kdeprint/kprintdialogpage.h>
#include <qlistbox.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>

#include "../umldoc.h"
#include "../umlview.h"
/**
 *	This is a page on the print dialog to select what diagram(s)
 *	you wish to print.  You add it to the @ref KPrinter instance.
 *
 *	You will then need to get the options as shown in @ref KPrinter.
 *
 *	@short	A print dialog page.
 *	@author Paul Hensgen	<phensgen@techie.com>
 *	@version	1.0
 *	@see	KPrinter
 */

class DiagramPrintPage : public KPrintDialogPage {
	Q_OBJECT
public:
	/**
	 *  Constructs the diagram print page.
	 *
	 *  @param parent The parent to the page.
	 *  @param doc	The @ref UMLDoc class instance being used.
	 */
	DiagramPrintPage(QWidget * parent, UMLDoc *doc);

	/**
	 *	Stnadard deconstructor.
	 */
	~DiagramPrintPage();

	/**
	 * Overriden method.
	 */
	void getOptions(QMap<QString,QString>& opts, bool /* incldef=false */);

	/**
	 * Overriden method.
	 */
	void setOptions( const QMap<QString,QString>& /*opts*/ );

	/**
	 * Overriden method.
	 */
	bool isValid( QString& msg );

private:
	QButtonGroup * m_pFilterBG;
	QGroupBox * m_pSelectGB, m_pOptionsGB;
	QListBox * m_pSelectLB;
	QRadioButton * m_pAllRB, * m_pCurrentRB, * m_pSelectRB, * m_pTypeRB;
	QComboBox * m_pTypeCB;

	UMLDoc * m_pDoc;
	Uml::Diagram_Type m_ViewType;
	int m_nIdList[100];//allow 100 diagrams

	enum FilterType{Current = 0, All, Select, Type};
public slots:
	void slotClicked(int id);
	void slotActivated(const QString & text);
};

#endif
