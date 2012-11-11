/*
    Copyright 2011  Andi Fischer  <andi.fischer@hispeed.ch>
    Copyright 2012  Ralf Habacker <ralf.habacker@freenet.de>

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
QMap<QString,bool> *Tracer::m_classes = 0;


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
    // in case no one called registerClass() before
    if (!m_classes)
        m_classes = new QMap<QString,bool>;
    setRootIsDecorated(true);
    setAlternatingRowColors(true);
    setHeaderLabel(i18n("Class Name"));
    setContextMenuPolicy(Qt::CustomContextMenu);
    resize(300, 400);
    connect(this,SIGNAL(itemClicked(QTreeWidgetItem*,int)),this,SLOT(slotItemClicked(QTreeWidgetItem*,int)));
}

/**
 * Destructor.
 */
Tracer::~Tracer()
{
    clear();
    delete m_classes;
}

/**
 * Return debugging state for a given class
 * @param name   the class name to check 
 */
bool Tracer::isEnabled(const QString& name)
{
    return (*m_classes)[name];
}

/**
 * Enable debug output for the given class.
 * @param name   class name
 */
void Tracer::enable(const QString& name)
{
    (*m_classes)[name] = true;
    update(name);
}

/**
 * Disable debug output for the given class.
 * @param name   class name
 */
void Tracer::disable(const QString& name)
{
    (*m_classes)[name] = false;
    update(name);
}

void Tracer::enableAll()
{
    //:TODO:
}

void Tracer::disableAll()
{
    //:TODO:
}

/**
 * Register class for debug output
 * @param name   class name
 * @param state  initial enabled state
 */
void Tracer::registerClass(const QString& name, bool state)
{
    if (!m_classes)
        m_classes = new QMap<QString,bool>;
    (*m_classes)[name] = state;
}

/**
 * Transfer class state into tree widget.
 * @param name   class name
 */
void Tracer::update(const QString &name)
{
    if (!isVisible())
        return;
    QList<QTreeWidgetItem*> items = findItems(name, Qt::MatchFixedString);
    foreach(QTreeWidgetItem* item, items) {
        item->setCheckState(0, (*m_classes)[name] ? Qt::Checked : Qt::Unchecked);
    }
}

/**
 * Fill tree widget with collected classes.
 */
void Tracer::showEvent(QShowEvent* e)
{
    Q_UNUSED(e);

    clear();
    QMapIterator<QString, bool> i(*m_classes);
    while (i.hasNext()) {
        i.next();
        QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(i.key()));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setCheckState(0, i.value() ? Qt::Checked : Qt::Unchecked);
        addTopLevelItem(item);
    }
}

/**
 * handle tree widget item selection signal
 * @param item tree widget item
 * @param column selected column
 */
void Tracer::slotItemClicked(QTreeWidgetItem* item, int colum)
{
    Q_UNUSED(colum);

    (*m_classes)[item->text(0)] = !(*m_classes)[item->text(0)];
}
