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

    void registerClass(const char* name, const QString& folder = QString());

    void enable(const QString& name);
    void disable(const QString& name);

    void enableAll();
    void disableAll();

    bool isEnabled(const char * name);

private:
    static Tracer* m_instance;

    explicit Tracer(QWidget *parent = 0);

};


#include <kdebug.h>

// convenience macros for console output to the Umbrello area
#define uDebug()   kDebug(8060)
#define uError()   kError(8060)
#define uWarning() kWarning(8060)

#define DBG_SRC  metaObject()->className()
#define uofDebug(DBG_SRC)  Tracer::instance()->isEnabled(DBG_SRC)?kDebug(8060):kDebugDevNull()
/*
// uof == umbrello object filter
static inline QDebug uofDebug(const char* name)
{
    if (Tracer::instance()->isEnabled(name))
        return kDebugStream(QtDebugMsg, 8060);
}
*/
//static inline QDebug uDebug()
//{
//    return kDebugStream(QtDebugMsg, 8060);
//}


#define uIgnoreZeroPointer(a) if (!a) { uDebug() << "zero pointer detected" << __FILE__ << __LINE__; continue; }


/**
 * In a Q_OBJECT class define any enum as Q_ENUMS.
 * With the above the following macro returns the name of a given enum.
 * This can be used in debug output.
 * TODO: convert it to a function.
 */
#define ENUM_NAME(o,e,v) (o::staticMetaObject.enumerator(o::staticMetaObject.indexOfEnumerator(#e)).valueToKey((v)))

#endif
