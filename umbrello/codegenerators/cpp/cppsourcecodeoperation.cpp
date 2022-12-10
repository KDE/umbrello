/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cppsourcecodeoperation.h"

#include "cppcodegenerator.h"
#include "cppcodegenerationpolicy.h"
#include "cppsourcecodedocument.h"
#include "cppcodedocumentation.h"
#include "uml.h"

CPPSourceCodeOperation::CPPSourceCodeOperation(CPPSourceCodeDocument * doc, UMLOperation *parent, const QString & body, const QString & comment)
        : CodeOperation (doc, parent, body, comment)
{
    // lets not go with the default comment and instead use
    // full-blown cpp documentation object instead
    setComment(new CPPCodeDocumentation(doc));

    // these things never change..
    setOverallIndentationLevel(0);
    setEndMethodText(QLatin1String("}"));
}

CPPSourceCodeOperation::~CPPSourceCodeOperation()
{
}

void CPPSourceCodeOperation::updateContent()
{
    CodeGenPolicyExt *pe = UMLApp::app()->policyExt();
    CPPCodeGenerationPolicy * policy = dynamic_cast<CPPCodeGenerationPolicy*>(pe);
    Q_ASSERT(policy);
    bool isInlineMethod = policy->getOperationsAreInline();

    if (!isInlineMethod) {
        setText(QString());  // change whatever it is to ""
    }
}

void CPPSourceCodeOperation::updateMethodDeclaration()
{
    CPPSourceCodeDocument * doc = dynamic_cast<CPPSourceCodeDocument*>(getParentDocument());
    Q_ASSERT(doc);
    CodeGenPolicyExt *pe = UMLApp::app()->policyExt();
    CPPCodeGenerationPolicy * policy = dynamic_cast<CPPCodeGenerationPolicy*>(pe);
    Q_ASSERT(policy);
    UMLClassifier * c = doc->getParentClassifier();
    UMLOperation * o = getParentOperation();
    bool isInterface = doc->parentIsInterface();
    bool isInlineMethod = policy->getOperationsAreInline();

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
    foreach (UMLAttribute* parm, list) {
        QString rType = parm->getTypeName();
        QString paramName = parm->name();
        paramStr += rType + QLatin1Char(' ') + paramName;
        paramNum++;

        if (paramNum != nrofParam)
            paramStr  += QLatin1String(", ");
    }

    // no return type for constructors/destructors
    if (o->isLifeOperation())
        returnType = QString();
    // if an operation isn't a constructor/destructor and it has no return type
    // this operation should be  void
    else if (returnType.isEmpty())
        returnType = QString(QLatin1String("void"));

    QString startText = returnType + QLatin1Char(' ');

    // if a property has a friend stereotype, the operation should
    // not be a class name
    if (o->stereotype() != QLatin1String("friend"))
        startText += className + QLatin1String("::");
    startText += methodName + QLatin1String(" (") + paramStr + QLatin1Char(')');
    if (o->getConst())
        startText += QLatin1String(" const");
    if (o->getOverride())
        startText += QLatin1String(" override");
    startText += QLatin1String(" {");

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

