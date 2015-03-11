/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "umllistviewfinder.h"

// app include
#include "uml.h"
#include "umllistview.h"
#include "umlobject.h"

UMLListViewFinder::UMLListViewFinder()
  : UMLFinder()
{
}

UMLListViewFinder::~UMLListViewFinder()
{
}

int UMLListViewFinder::collect(Category category, const QString &text)
{
    Q_UNUSED(category);
    QList<QTreeWidgetItem*> items = UMLApp::app()->listView()->findItems(text, Qt::MatchContains | Qt::MatchRecursive);
    m_items.clear();
    foreach(QTreeWidgetItem *item, items) {
        UMLListViewItem *ui = dynamic_cast<UMLListViewItem*>(item);
        if (!ui)
            continue;
        UMLObject *o = ui->umlObject();
        if (!includeObject(category, o))
            continue;
        m_items.append(o->id());
    }

    m_index = -1;
    return m_items.size();
}

UMLFinder::Result UMLListViewFinder::displayNext()
{
    if (m_items.size() == 0)
        return Empty;
    if (m_index >= m_items.size()-1) {
        m_index = -1;
        return End;
    }
    return showItem(m_items.at(++m_index)) ? Found : NotFound;
}

UMLFinder::Result  UMLListViewFinder::displayPrevious()
{
    if (m_items.size() == 0)
        return Empty;
    if (m_index < 1) {
        m_index = m_items.size();
        return End;
    }
    return showItem(m_items.at(--m_index)) ? Found : NotFound;
}

/**
 * Show item in Tree View.
 *
 * @param id ID of uml object to show
 * @return false list view entry not found, true otherwise
 */
bool UMLListViewFinder::showItem(Uml::ID::Type id)
{
    UMLListViewItem * item = UMLApp::app()->listView()->findItem(id);
    if (!item)
        return false;
    UMLApp::app()->listView()->setCurrentItem(item);
    return true;
}
