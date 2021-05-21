/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>
    SPDX-FileCopyrightText: 2012 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "debug_utils.h"

#include <KLocalizedString>

#include <QFileInfo>

#if QT_VERSION >= 0x050000
Q_LOGGING_CATEGORY(UMBRELLO, "umbrello")
#endif

Tracer* Tracer::m_instance = nullptr;
Tracer::MapType *Tracer::m_classes = nullptr;
Tracer::StateMap *Tracer::m_states = nullptr;

Tracer* Tracer::instance()
{
    if (m_instance == nullptr) {
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
        m_classes = new Tracer::MapType;
    if (!m_states)
        m_states = new Tracer::StateMap;
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
    m_classes = nullptr;
    delete m_states;
    m_states = nullptr;
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

    (*m_states)[parent->text(0)] = parent->checkState(0);
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
        QTreeWidgetItem* topLevel = nullptr;
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
    Qt::CheckState state = (*m_states)[parent->text(0)];
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
