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

    RubyCodeGenerationPolicyPage (QWidget *parent=0, const char *name=0, RubyCodeGenerationPolicy * policy = 0);

    virtual ~RubyCodeGenerationPolicyPage();

protected:

    RubyCodeGenerationFormBase * form;
    int commentTypeToInteger(RubyCodeGenerationPolicy::RubyCommentStyle type);

public slots:

    void apply();

};

#endif

