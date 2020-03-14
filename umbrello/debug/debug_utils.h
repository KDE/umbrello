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

#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#include <QtGlobal>

#if QT_VERSION < 0x050000
#include <kdebug.h>
#endif

#if QT_VERSION >= 0x050000
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(UMBRELLO)
#endif
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
 *      DEBUG(DBG_SRC) << ...
 *
 * - other classes
 *
 *      DEBUG("class name") << ...
 */
class Tracer : public QTreeWidget
{
    Q_OBJECT
public:
    static Tracer* instance();

    ~Tracer();

    bool isEnabled(const QString& name);
    void enable(const QString& name);
    void disable(const QString& name);

    void enableAll();
    void disableAll();

    static void registerClass(const QString& name, bool state=true, const QString &filePath=QString());

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

    static Tracer* m_instance;
    static MapType *m_classes;
    static StateMap *m_states;

    explicit Tracer(QWidget *parent = 0);
};

// convenience macros for console output to the Umbrello area
#if QT_VERSION >= 0x050000
#define uDebug()   qCDebug(UMBRELLO)
#define uError()   qCCritical(UMBRELLO)
#define uWarning() qCWarning(UMBRELLO)
#else
#define uDebug()   kDebug(8060)
#define uError()   kError(8060)
#define uWarning() kWarning(8060)
#endif

#define DBG_SRC  QString::fromLatin1(metaObject()->className())
#define DEBUG_SHOW_FILTER() Tracer::instance()->show()
#define DEBUG(src)  if (Tracer::instance()->isEnabled(src)) uDebug()
#define IS_DEBUG_ENABLED(src) Tracer::instance()->isEnabled(QString::fromLatin1(#src))
#define DEBUG_REGISTER(src) class src##Tracer { public: src##Tracer() { Tracer::registerClass(QString::fromLatin1(#src), true, QLatin1String(__FILE__)); } }; static src##Tracer src##TracerGlobal;
#define DEBUG_REGISTER_DISABLED(src) class src##Tracer { public: src##Tracer() { Tracer::registerClass(QString::fromLatin1(#src), false, QLatin1String(__FILE__)); } }; static src##Tracer src##TracerGlobal;

#define uIgnoreZeroPointer(a) if (!a) { uDebug() << "zero pointer detected" << __FILE__ << __LINE__; continue; }


/**
 * In a Q_OBJECT class define any enum as Q_ENUMS.
 * With the above the following macro returns the name of a given enum.
 * This can be used in debug output.
 * TODO: convert it to a function.
 */
#define ENUM_NAME(o, e, v) (o::staticMetaObject.enumerator(o::staticMetaObject.indexOfEnumerator(#e)).valueToKey((v)))

#endif
