/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef FINDRESULTS_H
#define FINDRESULTS_H

#include "umlfinder.h"

#include <QList>
#include <QObject>

class UMLDocFinder;
class UMLListViewFinder;
class UMLSceneFinder;
class QString;

/**
  * Class FindResults performs find and provide ui with find results
  * 
  * @author: Ralf Habacker <ralf.habacker@freenet.de>
  */
class FindResults : QObject
{
    Q_OBJECT
public:

    FindResults();
    virtual ~FindResults ();

    void clear();
    int collect(UMLFinder::Filter filter, UMLFinder::Category category, const QString & text);

public slots:
    bool displayNext();
    bool displayPrevious();

protected:
    QList<UMLSceneFinder> m_sceneFinder;
    QList<UMLDocFinder> m_docFinder;
    QList<UMLListViewFinder> m_listViewFinder;
};

#endif // FINDRESULTS_H
