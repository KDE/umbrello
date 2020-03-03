/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLFINDER_H
#define UMLFINDER_H

// app include
#include "basictypes.h"

// qt include
#include <QList>

class UMLObject;

/**
 * The UMLFinder class provides the interface for find related classes.
 *
 * @author: Ralf Habacker <ralf.habacker@freenet.de>
 */
class UMLFinder
{
public:
    typedef enum { All, Classes, Packages, Interfaces, Operations, Attributes } Category;
    typedef enum { TreeView, CurrentDiagram, AllDiagrams } Filter;
    typedef enum { Empty, End, Found, NotFound } Result;
    UMLFinder();
    virtual ~UMLFinder();

    /**
     * Collect items limited by a category and a text
     *
     * @param category Category to search for
     * @param text     Text to search for
     * @return Number of items found
     */
    virtual int collect(Category category, const QString &text) = 0;
    /**
     * Show next item.
     *
     * @return Result result of the operation
     */
    virtual Result displayNext() = 0;

    /**
     * Show previous item.
     *
     * @return Result result of the operation
     */
    virtual Result displayPrevious() = 0;

protected:
    int m_index;
    QList<Uml::ID::Type> m_items;

    bool includeObject(Category category, UMLObject *o);
};

#endif // UMLFINDER_H
