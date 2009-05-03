/***************************************************************************
                          configurable.h
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

#ifndef UMBRELLO_CONFIGURABLE_H
#define UMBRELLO_CONFIGURABLE_H

#include <QList>

// forward declarations
class KConfig;

/**
 * @defgroup U2_Lib Umbrello2 API
 * The Umbrello2 API consists of classes available to applications, command
 * line tools and plugins. These classes define common subsets of functionality
 * these objects. Primarily, these classes provide application support for
 * features such as configurability and plugin management. Also provided
 * within this API are the core interfaces for GUI management.
 */
namespace Umbrello
{
// forward declarations
class Plugin;

/**
 * @ingroup U2_Lib
 *
 * The Configurable class is the base class of all functional objects that
 * can be created for modeling applications. There are three types of
 * functionality: applications (with GUIs), command line tools and plugins.
 * This class provides a common configuration interface that the functional
 * classes use for default configuration and plugin management. Although
 * the Configurable class is primarily an interface it does provide some
 * convenience methods that can be used by derived classes to help manage
 * plugins. These methods are conceptually part of a larger (althought
 * currently undefined) set of configuration helper methods that reduce
 * the amount of code duplication for applications, tools and plugins.
 *
 * At this time, this class only assists with the configuration of the
 * event-driven plugin management system. All interfaces and convenience
 * methods support the hidden configuration functionality for derived
 * classes.
 *
 * @todo Do we have to delete the plugin object when its unloaded? Is it
 * possible that we can just unload the library and created objects are
 * automatically destroyed? I need some clarification of what actually
 * happens here...
 */
class Configurable
{
public:

    /** Construct a configurable object. */
    Configurable();

    /**
     * Destroy a configurable object. If there are any plugins that (for
     * some reason) have not been unloaded, we need to unload them here.
     */
    virtual ~Configurable();

    /**
     * The configure interface is required to be implemented by all subclasses
     * of this class. It is expected that configuration implementations all
     * understand how to attain their session configuration file. These files
     * are stored in ~/.kde/share/config. What actions are taken with the
     * configuration class are defined by implementing classes.
     */
    virtual bool configure() = 0;

protected:
    /**
     * This is a convenience method for derived classes. Configuration actions
     * that are intended to load plugins can use this method to parse the string
     * and actually load the plugins. The string is a set of space separated names.
     * Each name corresponds to the share object implementing the plugin.
     *
     * @param config    The object used for configuration.
     * @param key   The group in the config objec.
     * @param key   The key in the group that contains libraries to load.
     *
     * @return True on success, false on failure.
     */
    bool loadPlugins(KConfig *config, const QString &group, const QString &key);

    /**
     * This is a convenience method for derived classes. When a functional object
     * (i.e., application, tool or plugin) is shutdown, it can use this method
     * to automatically unload all dependant plugins.
     *
     * @return True on success false on failure.
     */
    bool unloadPlugins();

private:
    typedef QList<Plugin*> PluginList;

    PluginList  _plugins;       ///< List of loaded plugins
};
}

#endif
