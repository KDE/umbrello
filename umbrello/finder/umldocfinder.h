/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLDOCFINDER_H
#define UMLDOCFINDER_H

#include "umlfinder.h"
#include "umlscene.h"

#include <QList>

/**
  * UMLDocFinder provides finding of items for all diagrams in a document
  * 
  * @author: Ralf Habacker <ralf.habacker@freenet.de>
  */
class UMLDocFinder : public UMLFinder
{
public:
    UMLDocFinder();
    virtual ~UMLDocFinder();
    virtual int collect(Category category, const QString &text);

public slots:
    virtual Result displayNext();
    virtual Result displayPrevious();
protected:
    QList<UMLSceneFinder> m_sceneFinder;
};

#endif // UMLDOCFINDER_H
