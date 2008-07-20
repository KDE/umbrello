/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ARTIFACTWIDGET_H
#define ARTIFACTWIDGET_H

#include "newumlrectwidget.h"

class UMLArtifact;
class TextItemGroup;

/**
 * Defines a graphical version of the @ref UMLArtifact.
 * Most of the functionality will come from the @ref NewUMLRectWidget class.
 *
 * @short A graphical version of a Artifact.
 * @author Jonathan Riddell
 * @author Gopala Krishna (port using TextItems)
 * @see NewUMLRectWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ArtifactWidget : public NewUMLRectWidget
{
public:
	ArtifactWidget(UMLArtifact *a);
	virtual ~ArtifactWidget();

	virtual QSizeF sizeHint(Qt::SizeHint which);
    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *item, QWidget *w);

	// Note: For loading from XMI, the inherited parent method is
	// used.
    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
	virtual void updateGeometry();
	virtual void sizeHasChanged(const QSizeF& oldSize);

private:
    /**
     * Initializes key variables of the class.
     */
    void init();

    /**
     * calculates the size when drawing as an icon (it's the same size for all icons)
     */
    QSizeF calculateIconSize();

    /**
     * calculates the size for drawing as a box
     */
    QSizeF calculateNormalSize();

    void drawAsFile(QPainter *painter);
	void drawAsLibrary(QPainter *painter);
	void drawAsTable(QPainter *painter);
	void drawAsNormal(QPainter *painter);

	static const qreal Margin;
	static const QSizeF MinimumIconSize;

	QSizeF m_minimumSize;

	TextItemGroup *m_textItemGroup;
	enum {
		StereotypeItemIndex = 0,
		NameItemIndex = 1,
		TextItemCount = 2
	};

	qreal m_cachedTextHeight; //< Cache textheight to speedup.
};

#endif
