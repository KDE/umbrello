/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ARTIFACTWIDGET_H
#define ARTIFACTWIDGET_H

#include <qsize.h>

#include "umlwidget.h"

class UMLView;

#define ARTIFACT_MARGIN 5

/**
 *	Defines a graphical version of the Artifact.  Most of the functionality
 *	will come from the @ref UMLArtifact class.
 *
 *	@short	A graphical version of a Artifact.
 *	@author Jonathan Riddell
 *	@see	UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ArtifactWidget : public UMLWidget {
public:

	/**
	 *	Constructs a ArtifactWidget.
	 *
	 *	@param	view	The parent of this ArtifactWidget.
	 *	@param	o	The UMLObject this will be representing.
	 */
	ArtifactWidget(UMLView * view, UMLObject * o);

	/**
	 *	Constructs a ArtifactWidget.
	 *
	 *	@param	view	The parent of this ArtifactWidget.
	 */
	ArtifactWidget(UMLView * view);

	/**
	 *	Standard deconstructor.
	 */
	~ArtifactWidget();

	/**
	 *	Initializes key variables of the class.
	 */
	void init();

	/**
	 * Activate the object after loading it
	 */
	virtual bool activate(IDChangeLog* ChangeLog  = 0 );

	/**
	 *	Overrides standard method
	 */
	void draw(QPainter& p, int offsetX, int offsetY);

	/**
	 * Saves to the <artifactwidget> element
	 */
	bool saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

	// For loadFromXMI we can use the UMLWidgetData inherited method.

private:

	/**
	 *	Automatically calculates the size of the object.
	 */
	void calculateSize();

	/**
	 * calculates and sets the size when drawing as an icon (it's the same size for all icons)
	 */
	QSize calculateIconSize();

	/**
	 * calculates and sets the size for drawing as a box
	 */
	QSize calculateNormalSize();

	/**
	 * draw as a file icon
	 */
	void drawAsFile(QPainter& p, int offsetX, int offsetY);

	/**
	 * draw as a library file icon
	 */
	void drawAsLibrary(QPainter& p, int offsetX, int offsetY);

	/**
	 * draw as a database table icon
	 */
	void drawAsTable(QPainter& p, int offsetX, int offsetY);

	/**
	 * draw as a box
	 */
	void drawAsNormal(QPainter& p, int offsetX, int offsetY);

	/**
	 * The right mouse button menu
	 */
	ListPopupMenu* m_pMenu;
};

#endif
