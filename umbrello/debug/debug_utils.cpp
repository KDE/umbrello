/*
    Copyright 2011  Andi Fischer  <andi.fischer@hispeed.ch>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy 
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
void Tracer::registerClass(const QString& name, const QString& folder)
{
    QList<QTreeWidgetItem*> items = findItems(name, Qt::MatchFixedString);
    if (items.empty()) {
        uDebug() << name << " / folder = " << folder;
        QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(name));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setCheckState(0, Qt::Checked);
        addTopLevelItem(item);
    }
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

bool Tracer::isEnabled(const QString& name)
{
    QList<QTreeWidgetItem*> items = findItems(name, Qt::MatchFixedString);
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
