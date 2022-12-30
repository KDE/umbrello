/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>
    SPDX-FileCopyrightText: 2012 Ralf Habacker <ralf.habacker@freenet.de>
    SPDX-FileCopyrightText: 2022 Oliver Kellogg <okellogg@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

/*
    This file shall only by #included by implementation files (.cpp),
    not by header files (.h)
*/

#include <QtGlobal>

#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(UMBRELLO)
#include <QMetaEnum>
#include <QTreeWidget>

/**
 * @short The singleton class for switching on or off debug messages.
 *
 * This class provides a user controllable way to enable class related
 * debug output. 
 *
 * Classes could be registered with the static method registerClass().
 *
 * With show() a dialog will be shown, in which the user is able to
 * enable/disable debug output for each registered class. 
 *
 * Class related debug output implementation
 *
 * To register classes independent from related object instantiation time
 * one of the macros
 *
 *         DEBUG_REGISTER(className)
 *         DEBUG_REGISTER_DISABLED(className)
 *
 * should be placed in the implementation part of a class before the
 * first class methods. The first macro enables debug output for the
 * related class, while the latter macro disables it by default.
 *
 * Debug output in class methods should use
 *
 * - QObject based classes
 *
 *      DEBUG() << ...
 *
 * - other classes (Debug with given Name)
 *
 *      DEBUG_N("class name") << ...
 */
class Tracer : public QTreeWidget
{
    Q_OBJECT
public:
    static Tracer* instance();

    ~Tracer();

    bool isEnabled(const QString& name) const;
    void enable(const QString& name);
    void disable(const QString& name);

    void enableAll();
    void disableAll();

    bool logToConsole();

    static void registerClass(const char * name, bool state=true, const char * filePath=0);

protected:
    void update(const QString &name);
    void updateParentItemCheckBox(QTreeWidgetItem *parent);
    virtual void showEvent(QShowEvent*);

private slots:
    void slotParentItemClicked(QTreeWidgetItem *parent);
    void slotItemClicked(QTreeWidgetItem* item, int column);

private:
    class MapEntry {
    public:
        QString filePath;
        bool state;
        MapEntry() : state(false) {}
        MapEntry(const QString &_filePath, bool _state) : filePath(_filePath), state(_state) {}
    };

    typedef QMap<QString, MapEntry> MapType;
    typedef QMap<QString,Qt::CheckState> StateMap;

    static Tracer* s_instance;
    static MapType* s_classes;
    static StateMap* s_states;
    static bool s_logToConsole;

    explicit Tracer(QWidget *parent = 0);
};

// convenience macros for console output to the Umbrello area
#define uDebug()   qCDebug(UMBRELLO)
#define uError()   qCCritical(UMBRELLO)
#define uWarning() qCWarning(UMBRELLO)

#ifndef DBG_SRC
#define DBG_SRC  QString::fromLatin1(metaObject()->className())
#endif
#define DEBUG_SHOW_FILTER() Tracer::instance()->show()
#define DEBUG_N(latin1str)  if (Tracer::instance()->logToConsole() || Tracer::instance()->isEnabled(latin1str)) uDebug()
#define DEBUG()       DEBUG_N(DBG_SRC)
#define IS_DEBUG_ENABLED() Tracer::instance()->isEnabled(DBG_SRC)
#define DEBUG_REGISTER(src)          \
        class src##Tracer { \
          public:           \
            src##Tracer() { Tracer::registerClass(#src, true, __FILE__); } \
        };                  \
        static src##Tracer src##TracerGlobal;
#define DEBUG_REGISTER_DISABLED(src) \
        class src##Tracer { \
          public:           \
            src##Tracer() { Tracer::registerClass(#src, false, __FILE__); } \
        };                  \
        static src##Tracer src##TracerGlobal;

#define uIgnoreZeroPointer(a) if (!a) { uDebug() << "zero pointer detected" << __FILE__ << __LINE__; continue; }


/**
 * In a Q_OBJECT class define any enum as Q_ENUMS.
 * With the above the following macro returns the name of a given enum.
 * This can be used in debug output.
 * TODO: convert it to a function.
 */
#define ENUM_NAME(o, e, v) (o::staticMetaObject.enumerator(o::staticMetaObject.indexOfEnumerator(#e)).valueToKey((v)))

#endif
