/***************************************************************************
                          umlviewlist.h  -  description
                             -------------------
    begin                : Sat Dec 29 2001
    copyright            : (C) 2001 by Gustavo Madrigal
    email                : gmadrigal@nextphere.com
  Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLVIEWLIST_H
#define UMLVIEWLIST_H

//#include "umlview.h"
#include <QList>

//new canvas
#include "soc-umbrello-2011/diagram.h"
#include "soc-umbrello-2011/umlscene.h"
#include "soc-umbrello-2011/umlview.h"

//new canvas
#define SOC2011 1

namespace QGV {
  class Diagram;
  class UMLView;
  class UMLScene;
}

#ifdef SOC2011
  typedef QList<QGV::UMLView*> UMLViewList_new;
  typedef QListIterator<QGV::UMLView*> UMLViewListIt_new;
#endif
  
class UMLView;

typedef QList<UMLView*> UMLViewList;
typedef QListIterator<UMLView*> UMLViewListIt;

#endif
