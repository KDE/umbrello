/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "widgetset.h"

namespace Umbrello{

WidgetSet::WidgetSet(QObject *parent, const char *name, const QStringList &args): Plugin(parent,name,args) 
{}
 
WidgetSet::~WidgetSet()
{
}

}

#include "widgetset.moc"
