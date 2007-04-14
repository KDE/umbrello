/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CODEGENPOLICYEXT_H
#define CODEGENPOLICYEXT_H

#include <qobject.h>

class QWidget;
class KConfig;
class CodeGenerationPolicyPage;

/**
 * Base class for programming language specific code generation policy extensions.
 * Not to be confused with CodeGenerationPolicy which contains the programming
 * language independent policies.
 *
 * @author Oliver Kellogg <okellogg@users.sourceforge.net>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class CodeGenPolicyExt : public QObject {
public:
    CodeGenPolicyExt() {}
    virtual ~CodeGenPolicyExt() {}

    /**
     * Create a new dialog interface for this object.
     * @return dialog object
     */
    virtual CodeGenerationPolicyPage * createPage(QWidget *parent = 0, const char *name = 0) = 0;

    /**
     * set the defaults from a config file for this code generator from the passed KConfig pointer.
     */
    virtual void setDefaults(KConfig * config, bool emitUpdateSignal = true) = 0;

    /**
     * write Default params to passed KConfig pointer.
     */
    virtual void writeConfig (KConfig * config) = 0;
};

#endif
