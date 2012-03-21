/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlview.h"
#include "umlscene.h"

/**
 * Constructor
 */
UMLView::UMLView(UMLFolder *parentFolder)
  : UMLScene(parentFolder, this)
{
}

/**
 * Destructor
 */
UMLView::~UMLView()
{
}

#include "umlview.moc"
