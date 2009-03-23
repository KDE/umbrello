/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2003-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef JAVACODEGENERATIONPOLICY_H
#define JAVACODEGENERATIONPOLICY_H

#include "codegenpolicyext.h"
#include "codegenerationpolicy.h"

#include <QtCore/QString>

class KConfig;
class CodeGenerationPolicyPage;

class JavaCodeGenerationPolicy : public CodeGenPolicyExt
{
    Q_OBJECT
public:

    //JavaCodeGenerationPolicy(CodeGenerationPolicy * defaults = 0);
    JavaCodeGenerationPolicy();

    virtual ~JavaCodeGenerationPolicy();

    void setAutoGenerateAttribAccessors(bool var);
    bool getAutoGenerateAttribAccessors();

    void setAutoGenerateAssocAccessors(bool var);
    bool getAutoGenerateAssocAccessors();

    virtual void setDefaults(CodeGenPolicyExt * defaults, bool emitUpdateSignal = true);
    virtual void setDefaults(bool emitUpdateSignal = true);

    CodeGenerationPolicyPage * createPage ( QWidget *parent = 0, const char * name = 0);

private:

    CodeGenerationPolicy *m_commonPolicy;

    void init();
};

#endif // JAVACODEGENERATIONPOLICY_H
