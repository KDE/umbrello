/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CODEGENPOLICYEXT_H
#define CODEGENPOLICYEXT_H

#include <QObject>

class QWidget;
class CodeGenerationPolicyPage;

/**
 * Base class for programming language specific code generation policy extensions.
 * Not to be confused with CodeGenerationPolicy which contains the programming
 * language independent policies.
 *
 * @author Oliver Kellogg <okellogg@users.sourceforge.net>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
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
