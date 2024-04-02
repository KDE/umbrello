/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLDOCFINDER_H
#define UMLDOCFINDER_H

#include "umlfinder.h"
#include "umlscene.h"

class UMLSceneFinder;

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

    Q_SLOT virtual Result displayNext();
    Q_SLOT virtual Result displayPrevious();

protected:
    QList<UMLSceneFinder> m_sceneFinder;
};

#endif // UMLDOCFINDER_H
