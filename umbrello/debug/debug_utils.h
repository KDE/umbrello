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

#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#include <QtGui/QTreeWidget>

/**
 * @short The singleton class for switching on or off debug messages.
 *
 * This class ... .
 */
class Tracer : public QTreeWidget
{
    Q_OBJECT
public:
    static Tracer* instance();

    ~Tracer();

    void registerClass(const QString& name, const QString& folder = QString());

    void enable(const QString& name);
    void disable(const QString& name);

    void enableAll();
    void disableAll();

    bool isEnabled(const QString& name);

private:
    static Tracer* m_instance;

    explicit Tracer(QWidget *parent = 0);

};


#include <kdebug.h>

// convenience macros for console output to the Umbrello area
#define uDebug()   kDebug(8060)
#define uError()   kError(8060)
#define uWarning() kWarning(8060)

#define DBG_SRC  QString(metaObject()->className())
#define DEBUG_REGISTER(src) Tracer::instance()->registerClass(src);
#define DEBUG_REGISTER_DISABLED(src) Tracer::instance()->registerClass(src); Tracer::instance()->disable(src);
#define DEBUG_SHOW_FILTER() Tracer::instance()->show()
#define DEBUG(src)  if (Tracer::instance()->isEnabled(src)) uDebug()


#define uIgnoreZeroPointer(a) if (!a) { uDebug() << "zero pointer detected" << __FILE__ << __LINE__; continue; }


/**
 * In a Q_OBJECT class define any enum as Q_ENUMS.
 * With the above the following macro returns the name of a given enum.
 * This can be used in debug output.
 * TODO: convert it to a function.
 */
#define ENUM_NAME(o,e,v) (o::staticMetaObject.enumerator(o::staticMetaObject.indexOfEnumerator(#e)).valueToKey((v)))

#endif
