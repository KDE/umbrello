/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef USECASEWIDGET_H
#define USECASEWIDGET_H
#include "umlwidget.h"

#define UC_MARGIN 5
#define UC_WIDTH 60
#define UC_HEIGHT 30

// Qt forward declarations
class QPainter;

/**
 * This class is the graphical version of a UML UseCase.  A UseCaseWidget is created
 * by a @ref UMLView.  An UseCaseWidget belongs to only one @ref UMLView instance.
 * When the @ref UMLView instance that this class belongs to, it will be automatically deleted.
 *
 * If the @ref UseCase class that this UseCaseWidget is displaying is deleted, the @ref UMLView will
 * make sure that this instance is also deleted.
 *
 * The UseCaseWidget class inherits from the @ref UMLWidget class which adds most of the functionality
 * to this class.
 *
 * @short  A graphical version of a UML UseCase.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UseCaseWidget : public UMLWidget {
public:
	/**
	 *	Creates a UseCase widget.
	 *
	 *	@param	view		The parent of the widget.
	 *	@param	pData		The UMLWidgetData to represent.
	 */
	UseCaseWidget(UMLView * view, UMLObject *o, UMLWidgetData* pData);

	/**
	 *	Creates a UseCase widget.
	 *
	 *	@param	view		The parent of the widget.
	 *	@param	o		The UMLObject to represent.
	 */
	UseCaseWidget(UMLView * view, UMLObject *o);

	/**
	 *	Creates a UseCase widget.
	 *
	 *	@param	view		The parent of the widget.
	 */
	UseCaseWidget(UMLView * view);

	/**
	 *	Standard deconstructor
	 */
	~UseCaseWidget();

	/**
	*       Synchronize the Widget's m_pData member with its
	*       display properties, for example: the X and Y positions
	*       of the widget, etc
	*/
	virtual void synchronizeData();

	/**
	*	Overrides the standard paint event.
	*/
	void draw(QPainter & p, int offsetX, int offsetY);
protected:
	/**
	 *	Calculates the size of the widget.
	 */
	void calculateSize();
};

#endif
