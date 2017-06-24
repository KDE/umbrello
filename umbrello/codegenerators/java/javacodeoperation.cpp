/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "javacodeoperation.h"

#include "debug_utils.h"
#include "javaclassifiercodedocument.h"
#include "javacodedocumentation.h"
#include "javacodegenerator.h"
#include "uml.h"

JavaCodeOperation::JavaCodeOperation
 (JavaClassifierCodeDocument * doc, UMLOperation *parent, const QString & body, const QString & comment)
        : CodeOperation (doc, parent, body, comment)
{
    // lets not go with the default comment and instead use
    // full-blown java documentation object instead
    setComment(new JavaCodeDocumentation(doc));

    // these things never change..
    setOverallIndentationLevel(1);
}

JavaCodeOperation::~JavaCodeOperation()
{
}

// we basically want to update the doc and start text of this method
void JavaCodeOperation::updateMethodDeclaration()
{
    CodeDocument * doc = getParentDocument();
    JavaClassifierCodeDocument * javadoc = dynamic_cast<JavaClassifierCodeDocument*>(doc);
    Q_ASSERT(javadoc);
    UMLOperation * o = getParentOperation();
    bool isInterface = javadoc->getParentClassifier()->isInterface();
    QString endLine = getNewLineEndingChars();

    // now, the starting text.
    QString strVis = Uml::Visibility::toString(o->visibility());
    // no return type for constructors
    QString fixedReturn = JavaCodeGenerator::fixTypeName(o->getTypeName());
    QString returnType = o->isConstructorOperation() ? QString() : (fixedReturn + QLatin1String(" "));
    QString methodName = o->name();
    QString paramStr;

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
    QString maybeStatic;
    if (o->isStatic())
        maybeStatic = QLatin1String("static ");
    QString startText = strVis + QLatin1Char(' ') + maybeStatic + returnType + methodName + QLatin1String(" (") + paramStr + QLatin1Char(')');

    // IF the parent is an interface, our operations look different
    // e.g. they have no body
    if(isInterface) {
        startText += QLatin1Char(';');
        setEndMethodText(QString());
    } else {
        startText += QLatin1String(" {");
        setEndMethodText(QLatin1String("}"));
    }

    setStartMethodText(startText);

    // Lastly, for text content generation, we fix the comment on the
    // operation, IF the codeop is autogenerated & currently empty
    QString comment = o->doc();
    if(comment.isEmpty() && contentType() == CodeBlock::AutoGenerated)
    {
        UMLAttributeList parameters = o->getParmList();
        foreach (UMLAttribute* currentAtt, parameters) {
            comment += endLine + QLatin1String("@param ") + currentAtt->name() + QLatin1Char(' ');
            comment += currentAtt->doc();
        }
        // add a returns statement too
        if(!returnType.isEmpty())
            comment += endLine + QLatin1String("@return ") + returnType + QLatin1Char(' ');
        getComment()->setText(comment);
    }

    // In Java, for interfaces..we DON'T write out non-public
    // method declarations.
    if(isInterface)
    {
        UMLOperation * o = getParentOperation();
        if(o->visibility() != Uml::Visibility::Public)
            setWriteOutText(false);
    }
}

int JavaCodeOperation::lastEditableLine()
{
    ClassifierCodeDocument * doc = dynamic_cast<ClassifierCodeDocument*>(getParentDocument());

    // Check for dynamic casting failure
    if (doc == 0)
    {
        uError() << "doc: invalid dynamic cast";
        return -1;
    }

    if(doc->parentIsInterface())
        return -1; // very last line is NOT editable as its a one-line declaration w/ no body in
    // an interface.
    return 0;
}

