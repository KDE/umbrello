/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
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

    explicit CPPCodeGenerationPolicyPage (QWidget *parent = 0, const char *name = 0, CPPCodeGenerationPolicy * policy = 0);

    virtual ~CPPCodeGenerationPolicyPage();

protected:

    CPPCodeGenerationForm * form;

public slots:

    void apply();

};

#endif

