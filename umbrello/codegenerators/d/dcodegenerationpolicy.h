/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DCODEGENERATIONPOLICY_H
#define DCODEGENERATIONPOLICY_H

#include "codegenpolicyext.h"
#include "codegenerationpolicy.h"

#include <QString>

class CodeGenerationPolicyPage;

class DCodeGenerationPolicy : public CodeGenPolicyExt
{
    Q_OBJECT
public:

    //DCodeGenerationPolicy (CodeGenerationPolicy * defaults = 0);
    DCodeGenerationPolicy();

    virtual ~DCodeGenerationPolicy();

    void setAutoGenerateAttribAccessors (bool var);
    bool getAutoGenerateAttribAccessors();

    void setAutoGenerateAssocAccessors (bool var);
    bool getAutoGenerateAssocAccessors();

    virtual void setDefaults(CodeGenPolicyExt * defaults, bool emitUpdateSignal = true);
    virtual void setDefaults(bool emitUpdateSignal = true);

    CodeGenerationPolicyPage * createPage (QWidget *parent = nullptr, const char * *name = nullptr);

protected:

    void init();

private:

    CodeGenerationPolicy *m_commonPolicy;

};

#endif // DCODEGENERATIONPOLICY_H
