/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLWIDGETCOLORPAGE_H
#define UMLWIDGETCOLORPAGE_H

#include <qwidget.h>
/**
 * @author Paul Hensgen
 */
class UMLWidget;
class UMLView;
class KColorButton;
class QLabel;
class QPushButton;
class QCheckBox;
class QGroupBox;

class UMLWidgetColorPage : public QWidget {
	Q_OBJECT
public:

	/**
	* 	Constructor
	*/
	UMLWidgetColorPage( QWidget * pParent, UMLWidget * pWidget );

	/**
	*	Deconstructor
	*/
	~UMLWidgetColorPage();

	/**
	* 	Updates the @ref UMLWidget with the dialog properties.
	*/
	void updateUMLWidget();
protected:
	/**
	* 	The widget to set the color for.
	*/
	UMLWidget * m_pUMLWidget;
	//GUI widgets
	QGroupBox * m_pColorGB;
	QLabel * m_pLineColorL, * m_pFillColorL;
	QCheckBox * m_pUseFillColorCB;
	QPushButton * m_pLineDefaultB, * m_pFillDefaultB;
	KColorButton * m_pLineColorB, * m_pFillColorB;
public slots:
	/**
	*	Sets the default line color when default line button
	*	clicked.
	*/
	void slotLineButtonClicked();

	/**
	*	Sets the default fill color when default fill button
	*	clicked.
	*/
	void slotFillButtonClicked();
};

#endif
