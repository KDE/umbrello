/***************************************************************************
 * Copyright (C) 2011 by Andi Fischer <andi.fischer@hispeed.ch>            *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#include "debug_utils.h"

#include <klocale.h>

Tracer* Tracer::m_instance = 0;

Tracer* Tracer::instance()
{
    if (m_instance == 0) {
        m_instance = new Tracer();
    }
    return m_instance;
}

/**
 * Constructor.
 * @param parent   the parent widget
 */
Tracer::Tracer(QWidget *parent)
  : QTreeWidget(parent)
{
    setRootIsDecorated(true);
    setAlternatingRowColors(true);
//    setSelectionMode(QAbstractItemView::SingleSelection);
    setHeaderLabel(i18n("Class Name"));
    setContextMenuPolicy(Qt::CustomContextMenu);
    resize(300, 400);
}

/**
 * Destructor.
 */
Tracer::~Tracer()
{
    clear();
}

/**
 * ... .
 */
void Tracer::registerClass(const char* name, const QString& folder)
{
    QString itemName = QString(name);
    uDebug() << itemName << " / folder = " << folder;
    QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(itemName));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    item->setCheckState(0, Qt::Checked);
//    m_treeMap[item] = itemName;
    addTopLevelItem(item);
}

/**
 * ...
 * @param name   the class name for which the debug messages are enabled
 */
void Tracer::enable(const QString& name)
{
    QList<QTreeWidgetItem*> items = findItems(name, Qt::MatchFixedString);
    foreach(QTreeWidgetItem* item, items) {
        item->setCheckState(0, Qt::Checked);
    }
}

void Tracer::disable(const QString& name)
{
    QList<QTreeWidgetItem*> items = findItems(name, Qt::MatchFixedString);
    foreach(QTreeWidgetItem* item, items) {
        item->setCheckState(0, Qt::Unchecked);
    }
}

void Tracer::enableAll()
{
    //:TODO:
}

void Tracer::disableAll()
{
    //:TODO:
}

bool Tracer::isEnabled(const char * name)
{
    QList<QTreeWidgetItem*> items = findItems(QString(name), Qt::MatchFixedString);
    if (items.size() > 0) {
        Qt::CheckState state = items.at(0)->checkState(0);
        switch(state) {
            case Qt::Checked:
                return true;
            case Qt::Unchecked:
            default:
                return false;
        }
    }
    return false;
}

#include "debug_utils.moc"
