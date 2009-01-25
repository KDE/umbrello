/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CODEGENPOLICYEXT_H
#define CODEGENPOLICYEXT_H

#include <QtCore/QObject>

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
class CodeGenPolicyExt : public QObject
{
    Q_OBJECT
public:
    CodeGenPolicyExt() {}
    virtual ~CodeGenPolicyExt() {}

    virtual CodeGenerationPolicyPage * createPage(QWidget *parent = 0, const char *name = 0) = 0;

    virtual void setDefaults(bool emitUpdateSignal = true) = 0;

};

#endif
