/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "findresults.h"

// app includes
#include "uml.h"
#include "umldocfinder.h"
#include "umllistviewfinder.h"
#include "umlscenefinder.h"
#include "umlview.h"

FindResults::FindResults()
{
}

FindResults::~FindResults()
{
}

/**
 * Collect items limited by a filter, a category and a text.
 *
 * @param  filter filter to limit search
 * @param  catagory find category
 * @param  text search text
 * @return Number of items found
 */
int FindResults::collect(FindResults::Filter filter, Category category, const QString &text)
{
    Q_UNUSED(category)
    clear();
    if (filter == TreeView) {
        m_listViewFinder.append(UMLListViewFinder());
        return m_listViewFinder.last().collect(UMLFinder::Class, text);
    }
    else if (filter == CurrentDiagram) {
        m_sceneFinder.append(UMLSceneFinder(UMLApp::app()->currentView()->umlScene()));
        return m_sceneFinder.last().collect(UMLFinder::Class, text);
    }
    else if (filter == AllDiagrams) {
        m_docFinder.append(UMLDocFinder());
        return m_docFinder.last().collect(UMLFinder::Class, text);
    }
    return 0;
}

/**
 * Clear the find results.
 */
void FindResults::clear()
{
    m_docFinder.clear();
    m_listViewFinder.clear();
    m_sceneFinder.clear();
}

/**
 * Display next find item.
 *
 * @return true - an item has been displayed, false - no more items in this direction
 */
bool FindResults::displayNext()
{
    UMLFinder *finder = 0;
    if (m_listViewFinder.size() > 0)
        finder = &m_listViewFinder.first();
    else if (m_sceneFinder.size() > 0)
        finder = &m_sceneFinder.first();
    else if (m_docFinder.size() > 0)
        finder = &m_docFinder.first();
    UMLFinder::Result result = finder->displayNext();
    if (result == UMLFinder::Empty)
        return false;
    else if (result == UMLFinder::End)
        return finder->displayNext() == UMLFinder::Found;
    return true;
}

/**
 * Display previous find item.
 *
 * @return true - an item has been displayed, false - no more items in this direction
 */
bool FindResults::displayPrevious()
{
    UMLFinder *finder = 0;
    if (m_listViewFinder.size() > 0)
        finder = &m_listViewFinder.first();
    else if (m_sceneFinder.size() > 0)
        finder = &m_sceneFinder.first();
    else if (m_docFinder.size() > 0)
        finder = &m_docFinder.first();
    UMLFinder::Result result = finder->displayPrevious();
    if (result == UMLFinder::Empty)
        return false;
    else if (result == UMLFinder::End)
        return finder->displayPrevious() == UMLFinder::Found;
    return true;
}
