/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "infowidget.h"

InfoWidget::InfoWidget(QWidget *parent, const char *name ) : QWidget(parent,name) {
	setBackgroundColor(white);
	resize( 300, 200 );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
InfoWidget::~InfoWidget() {}
