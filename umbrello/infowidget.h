/*
 *  copyright (C) 2002-2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include <qwidget.h>

/**
 *	This is used to display information to the user when no diagrams
 *	have been displayed.
 *
 *	@short	Displays an information widget.
 *	@author Paul Hensgen	<phensgen@techie.com>
 *	@version	1.0
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class InfoWidget : public QWidget {
public:
    /**
     *	Constructs an InfoWidget.
     *
     *	@param	parent	The parent of this InfoWidget.
     *	@param	name	The name of this widget.
     */
    InfoWidget(QWidget *parent=0, const char *name=0);

    /**
     *	Standard deconstructor.
     */
    ~InfoWidget();
};

#endif
