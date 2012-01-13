/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "dialogbase.h"

// local includes
#include "icon_utils.h"
#include "umlwidgetstylepage.h"

// qt includes
#include <QFrame>
#include <QHBoxLayout>

/**
 * Constructor
 */
DialogBase::DialogBase(QWidget *parent)
  : KPageDialog( parent)
{
}

/**
 * Create a property page
 * @param name   The Text displayed in the page list
 * @param header The Text displayed above the page
 * @param icon  The icon to display in the page list
 * @return Pointer to created frame
 */
QFrame* DialogBase::createPage(const QString& name, const QString& header, Icon_Utils::IconType icon)
{
    QFrame* page = new QFrame();
    KPageWidgetItem *pageItem = new KPageWidgetItem( page, name );
    pageItem->setHeader( header );
    pageItem->setIcon( Icon_Utils::DesktopIcon(icon) );
    addPage( pageItem );
    page->setMinimumSize( 310, 330 );
    return page;
}
