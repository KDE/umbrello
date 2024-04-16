/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/
#ifndef CPPCODEGENERATIONPOLICYPAGE_H
#define CPPCODEGENERATIONPOLICYPAGE_H

#include "codegenerationpolicypage.h"
#include "cppcodegenerationform.h"
#include "cppcodegenerationpolicy.h"

/**
 * @author Brian Thomas
 */
class CPPCodeGenerationPolicyPage : public CodeGenerationPolicyPage
{
    Q_OBJECT
public:

    explicit CPPCodeGenerationPolicyPage (QWidget    *parent = nullptr, const char *name = nullptr, CPPCodeGenerationPolicy * policy = nullptr);

    virtual ~CPPCodeGenerationPolicyPage();

protected:

    CPPCodeGenerationForm * form;

public Q_SLOTS:

    void apply();

};

#endif

