/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLSCENEFINDER_H
#define UMLSCENEFINDER_H

#include "umlfinder.h"

class UMLView;
class UMLWidget;

/**
  * UMLListViewFinder provides finding of items in the tree view
  *
  * @author: Ralf Habacker <ralf.habacker@freenet.de>
  */
class UMLSceneFinder : public UMLFinder
{
public:
    UMLSceneFinder(UMLView *view);
    virtual ~UMLSceneFinder();
    virtual int collect(Category category, const QString &text);

public slots:
    virtual Result displayNext();
    virtual Result displayPrevious();

protected:
    Uml::ID::Type m_id; ///< id of scene
    bool showItem(Uml::ID::Type id);
};

#endif // UMLSCENEFINDER_H
