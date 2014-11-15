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

#include <QFileInfo>

Q_LOGGING_CATEGORY(UMBRELLO, "umbrello")

Tracer* Tracer::m_instance = 0;
Tracer::MapType *Tracer::m_classes = 0;

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
    //TODO: can be removed in Qt 5.3
    QLoggingCategory::setFilterRules(QStringLiteral("umbrello.debug = true"));

    // in case no one called registerClass() before
    if (!m_classes) {
        m_classes = new Tracer::MapType;
    }

    setRootIsDecorated(true);
    setAlternatingRowColors(true);
    setHeaderLabel(i18n("Class Name"));
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(slotItemClicked(QTreeWidgetItem*,int)));
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
    return (*m_classes)[name].state;
}

/**
 * Enable debug output for the given class.
 * @param name   class name
 */
void Tracer::enable(const QString& name)
{
    (*m_classes)[name].state = true;
    update(name);
}

/**
 * Disable debug output for the given class.
 * @param name   class name
 */
void Tracer::disable(const QString& name)
{
    (*m_classes)[name].state = false;
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
void Tracer::registerClass(const QString& name, bool state, const QString &filePath)
{
    if (!m_classes)
        m_classes = new MapType;
    QFileInfo fi(filePath);
    QString dirName = fi.absolutePath();
    QFileInfo f(dirName);
    QString path = f.fileName();
    (*m_classes)[name] = MapEntry(path, state);
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
        item->setCheckState(0, (*m_classes)[name].state ? Qt::Checked : Qt::Unchecked);
    }
}

QMap<QString,Qt::CheckState> states;

/**
 * Update check box of parent items.
 *
 * @param parent parent widget item
 */
void Tracer::updateParentItemCheckBox(QTreeWidgetItem* parent)
{
    int selectedCount = 0;
    for(int i = 0; i < parent->childCount(); i++) {
        if (parent->child(i)->checkState(0) == Qt::Checked)
            selectedCount++;
    }
    if (selectedCount == parent->childCount())
        parent->setCheckState(0, Qt::Checked);
    else if (selectedCount == 0)
        parent->setCheckState(0, Qt::Unchecked);
    else
        parent->setCheckState(0, Qt::PartiallyChecked);

    states[parent->text(0)] = parent->checkState(0);
}

/**
 * Fill tree widget with collected classes.
 */
void Tracer::showEvent(QShowEvent* e)
{
    Q_UNUSED(e);

    clear();
    MapType::const_iterator i = m_classes->constBegin();
    for(; i != m_classes->constEnd(); i++) {
        QList<QTreeWidgetItem*> items = findItems(i.value().filePath, Qt::MatchFixedString);
        QTreeWidgetItem* topLevel = 0;
        if (items.size() == 0) {
            topLevel = new QTreeWidgetItem(QStringList(i.value().filePath));
            topLevel->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            updateParentItemCheckBox(topLevel);
            addTopLevelItem(topLevel);
        }
        else
            topLevel = items.first();

        QTreeWidgetItem* item = new QTreeWidgetItem(topLevel, QStringList(i.key()));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setCheckState(0, i.value().state ? Qt::Checked : Qt::Unchecked);
    }

    for(int i = 0; i < topLevelItemCount(); i++)
        updateParentItemCheckBox(topLevelItem(i));
}

/**
 */
void Tracer::slotParentItemClicked(QTreeWidgetItem* parent)
{
    // @TODO parent->checkState(0) do not return the correct state
    // Qt::CheckState state = parent->checkState(0);
    Qt::CheckState state = states[parent->text(0)];
    if (state == Qt::PartiallyChecked || state == Qt::Unchecked) {
        for(int i = 0; i < parent->childCount(); i++) {
            QString text = parent->child(i)->text(0);
            (*m_classes)[text].state = true;
            parent->child(i)->setCheckState(0, (*m_classes)[text].state ? Qt::Checked : Qt::Unchecked);
        }
    } else if (state == Qt::Checked) {
        for(int i = 0; i < parent->childCount(); i++) {
            QString text = parent->child(i)->text(0);
            (*m_classes)[text].state = false;
            parent->child(i)->setCheckState(0, (*m_classes)[text].state ? Qt::Checked : Qt::Unchecked);
        }
    }
    updateParentItemCheckBox(parent);
}

/**
 * handle tree widget item selection signal
 * @param item tree widget item
 * @param column selected column
 */
void Tracer::slotItemClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);

    if (item->parent()) {
        (*m_classes)[item->text(0)].state = !(*m_classes)[item->text(0)].state;
        item->setCheckState(0, (*m_classes)[item->text(0)].state ? Qt::Checked : Qt::Unchecked);
        updateParentItemCheckBox(item->parent());
        return;
    }
    slotParentItemClicked(item);
}
