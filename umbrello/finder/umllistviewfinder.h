/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLLISTVIEWFINDER_H
#define UMLLISTVIEWFINDER_H

#include "umlfinder.h"

/**
  * UMLListViewFinder provides finding of items in the tree view
  *
  * @author: Ralf Habacker <ralf.habacker@freenet.de>
  */
class UMLListViewFinder : public UMLFinder
{
public:
    UMLListViewFinder();
    virtual ~UMLListViewFinder();
    virtual int collect(Category category, const QString &text);
public slots:
    virtual Result displayNext();
    virtual Result displayPrevious();
protected:
    bool showItem(Uml::ID::Type id);
};

#endif // UMLLISTVIEWFINDER_H
