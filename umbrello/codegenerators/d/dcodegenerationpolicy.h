/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   copyright (C) 2008-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DCODEGENERATIONPOLICY_H
#define DCODEGENERATIONPOLICY_H

#include "codegenpolicyext.h"
#include "codegenerationpolicy.h"

#include <QtCore/QString>

class KConfig;
class CodeGenerationPolicyPage;

class DCodeGenerationPolicy : public CodeGenPolicyExt
{
    Q_OBJECT
public:

    //DCodeGenerationPolicy (CodeGenerationPolicy * defaults = 0);
    DCodeGenerationPolicy();

    virtual ~DCodeGenerationPolicy();

    void setAutoGenerateAttribAccessors ( bool var );
    bool getAutoGenerateAttribAccessors();

    void setAutoGenerateAssocAccessors ( bool var );
    bool getAutoGenerateAssocAccessors();

    virtual void setDefaults(CodeGenPolicyExt * defaults, bool emitUpdateSignal = true);
    virtual void setDefaults(bool emitUpdateSignal = true);

    CodeGenerationPolicyPage * createPage ( QWidget *parent = 0, const char * name = 0);

protected:

    void init();

private:

    CodeGenerationPolicy *m_commonPolicy;

};

#endif // DCODEGENERATIONPOLICY_H
