/***************************************************************************
                          plugin.h
                             -------------------
    begin                : Mon Jan 13 2003
    copyright            : (C) 2003 by Andrew Sutton
    email                : ansutton@kent.edu
  Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMBRELLO_PLUGIN_H
#define UMBRELLO_PLUGIN_H

// Qt includes
#include <qobject.h>

// KDE includes
#include <kgenericfactory.h>

// local includes
#include "configurable.h"

// forward declarations
class QStringList;
class KConfig;

/**
 * This macro replaces the K_EXPORT_COMPONENT_FACTORY macro because of
 * a simple defficiency for this application - the construction of the
 * factory with a default instance name. This macro must be used in
 * the .cpp file implementing the plugin.
 *
 *
 * @param libname       The name of the plugin. This corresponds to
 *                      the name of the shared object without the ".so"
 *                      extension.
 * @param factory       The type of factory. Typically, this will be
 *                      KGenericFactory<> with the name of the plugin
 *                      as the parameter.
 */
#define UMBRELLO_EXPORT_PLUGIN_FACTORY(libname, factory) \
        extern "C" { KDE_EXPORT void *init_##libname() { return new factory(#libname); } }

namespace Umbrello
{
// forward declarations
class PluginLoader;

/**
 * @ingroup U2_Lib
 *
 * The Plugin class is the base class for all modular functionality in
 * the core Umbrello library. Because Umbrello is a plugin architecture,
 * this class is derived from many times. Plugins are created via the
 * KLibFactory of the encapsulating shared library and created from some
 * other functional object (application, tool or plugin). After the plugin
 * has been created, the init method is called. Before unloading, the
 * shutdown method is called. Derived plugins can implement specific
 * startup/shutdown behavior by overloading the onInit and onShutdown
 * methods respectively.
 *
 * By default, plugins use a configuration group called [LoadActions] in
 * the config file. Entries in this group define any dependant or on-demand
 * plugins that should be loaded in conjunction with this plugin. Known
 * entries (actions) are "Load" and "LoadGUI". Because plugins can be used
 * by both GUI and command line tools, they must be selective about some
 * functionality. Specifically, during configuration, a plugin for a tool
 * must not load GUI plugins.
 *
 * In order to provide application-like functionality, this class offers
 * support for accessing the configuration records of the KComponentData object
 * corresponding to the shared library. Because the KComponentData object is
 * only available within the scope of the shared library, the configuration
 * records must be set in the constructor of the derived plugin class. However,
 * because the construction name is passed to this constructor (as are the
 * parent object and args), we can simply capture the name when the object
 * is constructed.
 */
class Plugin :
            public QObject,
            public Configurable
{
    Q_OBJECT
    friend class PluginLoader;
public:
    /** Destroy a plugin.*/
    virtual ~Plugin();

    /** Return the instance name of the plugin */
    QByteArray instanceName() const;

    /** Return the configuration record for the plugin */
    KConfig *config();

    /** Return the category descriptor string */
    virtual QString category();

    /**
     * Unload the plugin. This method actually only decrements
     * the reference count. When the refcount is 0, the object
     * calls shutdown and deletes itself.
     */
    void unload();

protected:
    /** Construct a plugin */
    Plugin(QObject *parent, const char *name, const QStringList &args);

    /** Can be reimplemented to define plugin specific startup behavior */
    virtual bool onInit();

    /** Can be reimplemented to define plugin specific shutdown behavior */
    virtual bool onShutdown();

private:
    /**
     * This method is called by the loader to initialize and configure the
     * plugin. During initialization, any configured plugins are loaded.
     * Before loading plugins, onInit is called to perform plugin specific
     * initialization. This allows dependencies in the plugin chain.
     *
     * @return True on success, false on failure.
     */
    bool init();

    /**
     * This method is called by the loader to shutdown the plugin. During
     * shutdown, any configured plugins are unloaded this occurs before
     * plugin specific shutdown so as to reduce dependency errors.
     *
     * @return True on success, false on failure.
     */
    bool shutdown();

    /**
     * The configure method is called by init to parse the configuration
     * file and load any plugins. Note that the libraries loaded depends
     * on the GUI state of the application. If the application is type
     * Qt::Tty, then we don't use the "loadGUI" action.
     *
     * @return True on success, false on failure.
     */
    virtual bool configure();

    /** Add to the reference count */
    void ref();

protected:
    uint        _ref;           ///< Reference counter
    QByteArray    _instanceName;  ///< Instance name of the plugin
    KConfig    *_config;        ///< Configuration record
};
}

#endif
