/*
    SPDX-FileCopyrightText: 2003 Brian Thomas <brian.thomas@gsfc.nasa.gov>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CODEGENERATIONPOLICYPAGE_H
#define CODEGENERATIONPOLICYPAGE_H

#include "dialogpagebase.h"

class CodeGenPolicyExt;

/**
 * @author Brian Thomas
 */
class CodeGenerationPolicyPage : public DialogPageBase
{
    Q_OBJECT
public:
    explicit CodeGenerationPolicyPage (QWidget    *parent= nullptr, const char *name= nullptr, CodeGenPolicyExt * policy = nullptr);

    virtual ~CodeGenerationPolicyPage();

    void setDefaults();

protected:

    CodeGenPolicyExt * m_parentPolicy;

public Q_SLOTS:

    virtual void apply();


};

#endif

