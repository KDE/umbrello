/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>
    SPDX-FileCopyrightText: 2012 Ralf Habacker <ralf.habacker@freenet.de>
    SPDX-FileCopyrightText: 2022 Oliver Kellogg <okellogg@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "debug_utils.h"

#include <KLocalizedString>

#include <QFileInfo>

Q_LOGGING_CATEGORY(UMBRELLO, "umbrello")

enum ItemType { RootAll, Directory, Class };

Tracer* Tracer::s_instance = nullptr;
Tracer::MapType* Tracer::s_classes;
bool Tracer::s_logToConsole;

#define MAX_TRACERCLIENTS 500

/**
 * The client info filled by registerClass() needs to be Plain Old Data
 * (not requiring a constructor) as opposed to C++ class with complex
 * constructor because registerClass() is called very early in program
 * execution, in the "static initialization".
 * The static initialization is done before the main program starts
 * executing.
 * The compiler/linker determines the order in which static data are
 * initialized, and the compiler/linker chosen order may not coincide
 * with the programmer's expectation.
 * If a global or static object of a class has a constructor in which
 * methods of other global/static class objects are called then these
 * calls can easily result in crash. It cannot be naively assumed that
 * the other objects have already been constructed at the time of the
 * call because the compiler/linker may have chosen a contrary ordering
 * whereby their static initialization is done afterwards.
 * Concrete examples of such crashes are given in the source.
 */
struct ClientInfo_POD {
/* First example of problem mentioned in doxygen documentation:
   Controlled termination via fatal message from Qt,
   "QWidget: Must construct a QApplication before a QWidget"
   in this case happening on CodeEditorTracer but happens on other
   classes as well,
   #5  QMessageLogger::fatal (this=this@entry=0x7fffffffd2e0,
       msg=msg@entry=0x7ffff63cebc8 "QWidget: Must construct a QApplication before a QWidget")
       at global/qlogging.cpp:893
   #6  0x00007ffff600c8d1 in QWidgetPrivate::QWidgetPrivate (this=<optimized out>, this@entry=0xb1b070,
       version=<optimized out>, version@entry=331522) at kernel/qwidget.cpp:191
   #7  0x00007ffff61069ae in QFramePrivate::QFramePrivate (this=this@entry=0xb1b070) at widgets/qframe.cpp:60
   #8  0x00007ffff6107989 in QAbstractScrollAreaPrivate::QAbstractScrollAreaPrivate (this=this@entry=0xb1b070)
       at widgets/qabstractscrollarea.cpp:167
   #9  0x00007ffff6286179 in QAbstractItemViewPrivate::QAbstractItemViewPrivate (this=this@entry=0xb1b070)
       at itemviews/qabstractitemview.cpp:119
   #10 0x00007ffff6313e2b in QTreeViewPrivate::QTreeViewPrivate (this=0xb1b070)
       at ../../include/QtWidgets/5.15.2/QtWidgets/private/../../../../../src/widgets/itemviews/qtreeview_p.h:88
   #11 QTreeWidgetPrivate::QTreeWidgetPrivate (this=0xb1b070)
       at ../../include/QtWidgets/5.15.2/QtWidgets/private/../../../../../src/widgets/itemviews/qtreewidget_p.h:225
   #12 QTreeWidget::QTreeWidget (this=0xb17bd0, parent=0x0) at itemviews/qtreewidget.cpp:2662
   #13 0x000000000048ff02 in Tracer::Tracer (this=0xb17bd0, parent=0x0) at /umbrello-master/umbrello/debug/debug_utils.cpp:33
   #14 0x000000000048fec7 in Tracer::instance () at /umbrello-master/umbrello/debug/debug_utils.cpp:23
   #15 0x0000000000490237 in Tracer::registerClass (name="CodeEditor", state=true,
       filePath="/umbrello-master/umbrello/dialogs/codeeditor.cpp")
       at /umbrello-master/umbrello/debug/debug_utils.cpp:109
   #16 0x000000000049ef7f in CodeEditorTracer::CodeEditorTracer (this=0xadd6c0 <CodeEditorTracerGlobal>)
       at /umbrello-master/umbrello/dialogs/codeeditor.cpp:54
   #17 0x000000000049e742 in __static_initialization_and_destruction_0 (__initialize_p=1, __priority=65535)
       at /umbrello-master/umbrello/dialogs/codeeditor.cpp:54
   #18 0x000000000049e758 in _GLOBAL__sub_I_codeeditor.cpp(void) () at /umbrello-master/umbrello/dialogs/codeeditor.cpp:1534
   #19 0x00007ffff4bf46fd in call_init (env=<optimized out>, argv=0x7fffffffd578, argc=1) at ../csu/libc-start.c:145
   #20 __libc_start_main_impl (main=0x46f255 <main(int, char**)>, argc=1, argv=0x7fffffffd578, init=<optimized out>,
       fini=<optimized out>, rtld_fini=<optimized out>, stack_end=0x7fffffffd568) at ../csu/libc-start.c:379
   #21 0x000000000046ea75 in _start () at ../sysdeps/x86_64/start.S:116

   Second example of problem: Crash in different client classes of Tracer,
   in this example running unittests/testoptionstate the SEGV happens on
   Tracer::registerClass call from UMLApp but also happens from other classes,
   (gdb) bt
   #0  std::__atomic_base<int>::load (__m=std::memory_order_relaxed, this=0x0) at /usr/include/c++/11/bits/atomic_base.h:481
   #1  QAtomicOps<int>::loadRelaxed<int> (_q_value=...) at /usr/include/qt5/QtCore/qatomic_cxx11.h:239
   #2  0x00000000004721c8 in QBasicAtomicInteger<int>::loadRelaxed (this=0x0) at /usr/include/qt5/QtCore/qbasicatomic.h:107
   #3  0x000000000047162c in QtPrivate::RefCount::isShared (this=0x0) at /usr/include/qt5/QtCore/qrefcount.h:101
   #4  0x000000000051c2b3 in QMap<QString, MapEntry>::detach (this=0xac5c30 <s_classes>) at /usr/include/qt5/QtCore/qmap.h:361
   #5  0x000000000051bf27 in QMap<QString, MapEntry>::operator[] (this=0xac5c30 <s_classes>, akey="UMLApp")
       at /usr/include/qt5/QtCore/qmap.h:680
   #6  0x000000000051b325 in Tracer::registerClass (name="UMLApp", state=true, filePath="/umbrello-master/umbrello/uml.cpp")
       at /umbrello-master/umbrello/debug/debug_utils.cpp:123
   #7  0x0000000000497cd9 in UMLAppTracer::UMLAppTracer (this=0xac5640 <UMLAppTracerGlobal>)
       at /umbrello-master/umbrello/uml.cpp:127
   #8  0x0000000000496240 in __static_initialization_and_destruction_0 (__initialize_p=1, __priority=65535)
       at /umbrello-master/umbrello/uml.cpp:127
   #9  0x0000000000496256 in _GLOBAL__sub_I_uml.cpp(void) () at /umbrello-master/umbrello/uml.cpp:3590
   #10 0x00007ffff4b9a6fd in call_init (env=<optimized out>, argv=0x7fffffffd568, argc=1) at ../csu/libc-start.c:145
   #11 __libc_start_main_impl (main=0x4754fe <main(int, char**)>, argc=1, argv=0x7fffffffd568, init=<optimized out>,
       fini=<optimized out>, rtld_fini=<optimized out>, stack_end=0x7fffffffd558) at ../csu/libc-start.c:379
   #12 0x000000000046e9e5 in _start () at ../sysdeps/x86_64/start.S:116
 */
    const char * name;
    bool state;
    const char * filePath;
};
static ClientInfo_POD g_clientInfo[MAX_TRACERCLIENTS];
static int n_clients = 0;

Tracer* Tracer::instance()
{
    if (s_instance == nullptr) {
        s_instance = new Tracer();
        s_classes = new MapType();
        // Transfer g_clientInfo (C plain old data) to s_classes (C++)
        for (int i = 0; i < n_clients; i++) {
            ClientInfo_POD & cli = g_clientInfo[i];
            QFileInfo fi(QLatin1String(cli.filePath));
            QString dirName = fi.absolutePath();
            QFileInfo f(dirName);
            QString path = f.fileName();
            uDebug() << "Tracer::registerClass(" << cli.name << ") : " << path;
            QString name = QString::fromLatin1(cli.name);
            (*s_classes)[name] = MapEntry(path, cli.state);
        }
        QString umbrello_logToConsole = QString::fromLatin1(qgetenv("UMBRELLO_LOG_TO_CONSOLE"));
        s_logToConsole = (umbrello_logToConsole == QStringLiteral("1"));
    }
    return s_instance;
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
    connect(this, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(slotItemClicked(QTreeWidgetItem*,int)));
}

/**
 * Destructor.
 */
Tracer::~Tracer()
{
    clear();
}

/**
 * Return debugging state for a given class
 * @param name   the class name to check
 */
bool Tracer::isEnabled(const QString& name) const
{
    if (!s_classes->contains(name)) {
        // Classes that are not registered are enabled by default.
        // The intent is that classes which produce few debug messages, or whose
        // debug messages for some reason shall not be suppressible, shall not
        // require registration.
        // Furthermore, returning false here would prevent a class from ever
        // producing debug messages.
        return true;
    }
    return (*s_classes)[name].state;
}

/**
 * Enable debug output for the given class.
 * @param name   class name
 */
void Tracer::enable(const QString& name)
{
    (*s_classes)[name].state = true;
    update(name);
}

/**
 * Disable debug output for the given class.
 * @param name   class name
 */
void Tracer::disable(const QString& name)
{
    (*s_classes)[name].state = false;
    update(name);
}


void Tracer::enableAll()
{
    for (auto it = s_classes->begin(); it != s_classes->end(); ++it)
        it.value().state = true;

    QTreeWidgetItem* allItem = topLevelItem(0);

    for (int i = 0; i < allItem->childCount(); ++i) {
        QTreeWidgetItem* dir = allItem->child(i);
        for (int j = 0; j < dir->childCount(); ++j)
            dir->child(j)->setCheckState(0, Qt::Checked);
        dir->setCheckState(0, Qt::Checked);
    }

    allItem->setCheckState(0, Qt::Checked);
}

void Tracer::disableAll()
{
    for (auto it = s_classes->begin(); it != s_classes->end(); ++it)
        it.value().state = false;

    QTreeWidgetItem* allItem = topLevelItem(0);

    for (int i = 0; i < allItem->childCount(); ++i) {
        QTreeWidgetItem* dir = allItem->child(i);
        for (int j = 0; j < dir->childCount(); ++j)
            dir->child(j)->setCheckState(0, Qt::Unchecked);
        dir->setCheckState(0, Qt::Unchecked);
    }

    allItem->setCheckState(0, Qt::Unchecked);
}

bool Tracer::logToConsole()
{
    return s_logToConsole;
}

/**
 * Register class for debug output
 * @param name   class name
 * @param state  initial enabled state
 * @param filePath  path qualified source filename of class
 */
void Tracer::registerClass(const char  *name, bool state, const char * filePath /* = nullptr */)
{
    if (n_clients >= MAX_TRACERCLIENTS) {
        uError() << "Tracer::registerClass : MAX_TRACERCLIENTS is exceeded";
        return;
    }
    ClientInfo_POD & client = g_clientInfo[n_clients];
    client.name = strdup(name);
    client.state = state;
    if (filePath)
        client.filePath = strdup(filePath);
    else
        client.filePath = nullptr;
    n_clients++;
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
    for(QTreeWidgetItem *item : items) {
        item->setCheckState(0, (*s_classes)[name].state ? Qt::Checked : Qt::Unchecked);
    }
}

void Tracer::updateAllItemCheckBox()
{
    if (topLevelItemCount() == 0)
        return;

    QTreeWidgetItem* allItem = topLevelItem(0);
    int enabled = 0;
    const int total = s_classes->size();

    for (auto it = s_classes->constBegin(); it != s_classes->constEnd(); ++it) {
        if (it.value().state)
            ++enabled;
    }

    if (enabled == total)
        allItem->setCheckState(0, Qt::Checked);
    else if (enabled == 0)
        allItem->setCheckState(0, Qt::Unchecked);
    else
        allItem->setCheckState(0, Qt::PartiallyChecked);
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
}

/**
 * Fill tree widget with collected classes.
 */
void Tracer::showEvent(QShowEvent* e)
{
    Q_UNUSED(e);

    clear();

    // Root: "All"
    QTreeWidgetItem* allItem = new QTreeWidgetItem(QStringList(i18n("All")));
    allItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    allItem->setData(0, Qt::UserRole, RootAll);
    addTopLevelItem(allItem);

    QHash<QString, QTreeWidgetItem*> dirMap;

    for (auto it = s_classes->constBegin(); it != s_classes->constEnd(); ++it) {
        QTreeWidgetItem* dirItem = dirMap.value(it.value().filePath, nullptr);

        if (!dirItem) {
            dirItem = new QTreeWidgetItem(allItem, QStringList(it.value().filePath));
            dirItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            dirItem->setData(0, Qt::UserRole, Directory);
            dirMap.insert(it.value().filePath, dirItem);
        }

        QTreeWidgetItem* clsItem = new QTreeWidgetItem(dirItem, QStringList(it.key()));
        clsItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        clsItem->setCheckState(0, it.value().state ? Qt::Checked : Qt::Unchecked);
        clsItem->setData(0, Qt::UserRole, Class);
    }

    for (int i = 0; i < topLevelItemCount(); ++i) {
        restoreParentCheckState(topLevelItem(i));
    }
    updateAllItemCheckBox();

    expandItem(allItem);
    sortItems(0, Qt::AscendingOrder);
}

void Tracer::restoreParentCheckState(QTreeWidgetItem* parent)
{
    int checkedCount = 0;
    int totalChildren = parent->childCount();

    for (int i = 0; i < totalChildren; ++i) {
        QTreeWidgetItem* child = parent->child(i);

        if (child->childCount() > 0) { // directory
            restoreParentCheckState(child);
            if (child->checkState(0) == Qt::Checked)
                ++checkedCount;
            else if (child->checkState(0) == Qt::PartiallyChecked)
                checkedCount += 0; // will set parent to partial below
        } else { // class
            QString className = child->text(0);
            if ((*s_classes)[className].state)
                ++checkedCount;
        }
    }

    if (checkedCount == 0)
        parent->setCheckState(0, Qt::Unchecked);
    else if (checkedCount == totalChildren)
        parent->setCheckState(0, Qt::Checked);
    else
        parent->setCheckState(0, Qt::PartiallyChecked);
}

/**
 */
void Tracer::slotParentItemClicked(QTreeWidgetItem* parent)
{
    if (parent->data(0, Qt::UserRole).toInt() != Directory)
        return;

    Qt::CheckState state = parent->checkState(0);
    for(int i = 0; i < parent->childCount(); i++) {
        QString text = parent->child(i)->text(0);
        (*s_classes)[text].state = (state == Qt::Checked);
        parent->child(i)->setCheckState(0, state);
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

    switch (item->data(0, Qt::UserRole).toInt()) {
    case Class: {
        const QString name = item->text(0);
        (*s_classes)[name].state = !(*s_classes)[name].state;
        item->setCheckState(0, (*s_classes)[name].state ? Qt::Checked : Qt::Unchecked);

        updateParentItemCheckBox(item->parent());
        updateAllItemCheckBox();
        break;
    }
    case Directory:
        slotParentItemClicked(item);
        updateAllItemCheckBox();
        break;

    case RootAll: {
        Qt::CheckState state = item->checkState(0);
        if (state == Qt::Checked)
            enableAll();
        else
            disableAll();
        break;
    }
    default:
        break;
    }
}
