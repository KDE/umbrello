/***************************************************************************
                          rubycodegenerationpolicypage.h
                          Derived from the Java code generator by thomas

    begin                : Thur Jul 21 2005
    author               : Richard Dale
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef RUBYCODEGENERATIONPOLICYPAGE_H
#define RUBYCODEGENERATIONPOLICYPAGE_H

#include "../dialogs/codegenerationpolicypage.h"
#include "rubycodegenerationformbase.h"

#include "rubycodegenerationpolicy.h"

/**
 * @author Brian Thomas
 */

class RubyCodeGenerationPolicyPage : public CodeGenerationPolicyPage {
    Q_OBJECT
public:

    explicit RubyCodeGenerationPolicyPage (QWidget *parent=0, const char *name=0, RubyCodeGenerationPolicy * policy = 0);

    virtual ~RubyCodeGenerationPolicyPage();

protected:

    RubyCodeGenerationFormBase * form;

public slots:

    void apply();

};

#endif

