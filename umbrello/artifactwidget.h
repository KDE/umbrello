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

#include "artifactwidgetdata.h"
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
 */
class ArtifactWidget : public UMLWidget {
public:
	/**
	 *	Constructs a ArtifactWidget.
	 *
	 *	@param	view		The parent of this ArtifactWidget.
	 *	@param	pData		The ArtifactWidgetData to represent.
	 */
	ArtifactWidget(UMLView* view, UMLObject* o, UMLWidgetData* pData);

	/**
	 *	Constructs a ArtifactWidget.
	 *
	 *	@param	view		The parent of this ArtifactWidget.
	 *	@param	o		The UMLObject is will be representing.
	 */
	ArtifactWidget(UMLView* view, UMLObject* o);

	/**
	 *	Constructs a ArtifactWidget.
	 *
	 *	@param	view	The parent of this ArtifactWidget.
	 */
	ArtifactWidget(UMLView* view);

	/**
	 *	Standard deconstructor.
	 */
	~ArtifactWidget();

	/**
	 *	Initializes key variables of the class.
	 */
	void init();

	/**
	 * Activate the object after serializing it from a QDataStream
	 */
	virtual bool activate(IDChangeLog* ChangeLog  = 0 );

	/**
	 *	Overrides standards method
	 */
	void draw(QPainter& p, int offsetX, int offsetY);
private:

	/**
	 *	Automatically calculates the size of the object.
	 */
	void calculateSize();
	//FIXMEnow doc
	QSize calculateIconSize();
	QSize calculateNormalSize();

	//FIXMEnow
	void drawAsFile(QPainter& p, int offsetX, int offsetY);
	void drawAsLibrary(QPainter& p, int offsetX, int offsetY);
	void drawAsTable(QPainter& p, int offsetX, int offsetY);
	void drawAsNormal(QPainter& p, int offsetX, int offsetY);

	/**
	 * The right mouse button menu
	 */
	ListPopupMenu* m_pMenu;
};

#endif
