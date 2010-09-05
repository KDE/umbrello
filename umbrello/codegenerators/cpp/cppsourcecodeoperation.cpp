/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2010                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cppsourcecodeoperation.h"

#include "cppcodegenerator.h"
#include "cppcodegenerationpolicy.h"
#include "cppsourcecodedocument.h"
#include "cppcodedocumentation.h"
#include "uml.h"

CPPSourceCodeOperation::CPPSourceCodeOperation(CPPSourceCodeDocument * doc, UMLOperation *parent, const QString & body, const QString & comment )
        : CodeOperation (doc, parent, body, comment)
{
    // lets not go with the default comment and instead use
    // full-blown cpp documentation object instead
    setComment(new CPPCodeDocumentation(doc));

    // these things never change..
    setOverallIndentationLevel(0);
    setEndMethodText("}");
}

CPPSourceCodeOperation::~CPPSourceCodeOperation()
{
}

void CPPSourceCodeOperation::updateContent( )
{
    CodeGenPolicyExt *pe = UMLApp::app()->policyExt();
    CPPCodeGenerationPolicy * policy = dynamic_cast<CPPCodeGenerationPolicy*>(pe);
    bool isInlineMethod = policy->getOperationsAreInline();

    if (!isInlineMethod) {
        setText("");  // change whatever it is to ""
    }
}

void CPPSourceCodeOperation::updateMethodDeclaration()
{
    CPPSourceCodeDocument * doc = dynamic_cast<CPPSourceCodeDocument*>(getParentDocument());
    CodeGenPolicyExt *pe = UMLApp::app()->policyExt();
    CPPCodeGenerationPolicy * policy = dynamic_cast<CPPCodeGenerationPolicy*>(pe);
    UMLClassifier * c = doc->getParentClassifier();
    UMLOperation * o = getParentOperation();
    bool isInterface = doc->parentIsInterface();
    bool isInlineMethod = policy->getOperationsAreInline( );

    // first, the comment on the operation
    QString comment = o->doc();
    getComment()->setText(comment);

    QString returnType = o->getTypeName();
    QString methodName = o->name();
    QString paramStr;
    QString className = CodeGenerator::cleanName(c->name());

    // assemble parameters
    UMLAttributeList list = getParentOperation()->getParmList();
    int nrofParam = list.count();
    int paramNum = 0;
    foreach (UMLAttribute* parm, list ) {
        QString rType = parm->getTypeName();
        QString paramName = parm->name();
        paramStr += rType + ' ' + paramName;
        paramNum++;

        if (paramNum != nrofParam )
            paramStr  += ", ";
    }

    // no return type for constructors/destructors
    if (o->isLifeOperation())
        returnType = "";
    // if an operation isn't a constructor/destructor and it has no return type
    // this operation should be  void
    else if (returnType.isEmpty())
        returnType = QString("void");

    QString startText = returnType + ' ';

    // if a property has a friend stereotype, the operation should
    // not be a class name
    if (o->stereotype() != "friend")
        startText += className + "::";
    startText += methodName + " (" + paramStr + ')';
    if (o->getConst())
        startText += " const";
    startText += " {";

    setStartMethodText(startText);

    // Only write this out if it is a child of an interface OR is abstract.
    // and it is not inline
    if (isInterface || o->isAbstract() || isInlineMethod)
    {
        setWriteOutText(false);
    } else {
        setWriteOutText(true);
    }

}

#include "cppsourcecodeoperation.moc"
