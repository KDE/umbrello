/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umldocfinder.h"

// app include
#include "uml.h"
#include "umldoc.h"
#include "umlscenefinder.h"
#include "umlview.h"
#include "umlwidget.h"
#include "umlviewlist.h"

UMLDocFinder::UMLDocFinder()
  : UMLFinder()
{
}

UMLDocFinder::~UMLDocFinder()
{
}

int UMLDocFinder::collect(UMLFinder::Category category, const QString &text)
{
    m_sceneFinder.clear();
    UMLViewList views = UMLApp::app()->document()->viewIterator();
    int counts = 0;
    foreach (UMLView *view, views) {
        UMLSceneFinder finder(view);
        int count = finder.collect(category, text);
        if (count > 0) {
            m_sceneFinder.append(finder);
            counts += count;
        }
    }
    m_index = 0;
    return counts;
}

UMLFinder::Result UMLDocFinder::displayNext()
{
    if (m_sceneFinder.size() == 0)
        return Empty;

    if (m_index >= m_sceneFinder.size()) {
        m_index = 0;
        return End;
    }
    Result result = m_sceneFinder[m_index].displayNext();
    if (result == End) {
        ++m_index;
        return displayNext();
    }
    else if (result == NotFound) {
        return displayPrevious();
    }
    return result;
}

UMLFinder::Result UMLDocFinder::displayPrevious()
{
    if (m_sceneFinder.size() == 0)
        return Empty;

    if (m_index < 0) {
        m_index = m_sceneFinder.size()-1;
        return End;
    }
    Result result = m_sceneFinder[m_index].displayPrevious();
    if (result == End) {
        --m_index;
        return displayPrevious();
    }
    else if (result == NotFound) {
        return displayPrevious();
    }
    return result;
}
