/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef RUBYCODEGENERATIONPOLICY_H
#define RUBYCODEGENERATIONPOLICY_H

#include "codegenpolicyext.h"
#include "codegenerationpolicy.h"

#include <QString>

class CodeGenerationPolicyPage;

class RubyCodeGenerationPolicy : public CodeGenPolicyExt
{
    Q_OBJECT
public:

    RubyCodeGenerationPolicy();
    virtual ~RubyCodeGenerationPolicy();

    void setAutoGenerateAttribAccessors(bool var);
    bool getAutoGenerateAttribAccessors();

    void setAutoGenerateAssocAccessors(bool var);
    bool getAutoGenerateAssocAccessors();

    virtual void setDefaults(CodeGenPolicyExt * defaults, bool emitUpdateSignal = true);
    virtual void setDefaults(bool emitUpdateSignal = true);

    CodeGenerationPolicyPage * createPage (QWidget *parent = nullptr, const char * *name = nullptr);

private:

    CodeGenerationPolicy *m_commonPolicy;

    void init();
};

#endif // RUBYCODEGENERATIONPOLICY_H
