/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "javaclassdeclarationblock.h"

#include "codegenerator.h"
#include "codegenerationpolicy.h"
#include "javacodedocumentation.h"
#include "uml.h"

JavaClassDeclarationBlock::JavaClassDeclarationBlock
 (JavaClassifierCodeDocument * parentDoc, const QString &startText, const QString &endText, const QString &comment)
        : OwnedHierarchicalCodeBlock(parentDoc->getParentClassifier(), parentDoc, startText, endText, comment)
{
    init(parentDoc, comment);
}

JavaClassDeclarationBlock::~JavaClassDeclarationBlock ()
{
}

/**
 * Save the XMI representation of this object
 */
void JavaClassDeclarationBlock::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("javaclassdeclarationblock"));
    setAttributesOnNode(writer);
    writer.writeEndElement();
}

/**
 * load params from the appropriate XMI element node.
 */
void JavaClassDeclarationBlock::loadFromXMI (QDomElement & root)
{
    setAttributesFromNode(root);
}

/**
 * update the start and end text for this ownedhierarchicalcodeblock.
 */
void JavaClassDeclarationBlock::updateContent ()
{
    JavaClassifierCodeDocument *parentDoc = dynamic_cast<JavaClassifierCodeDocument*>(getParentDocument());
    Q_ASSERT(parentDoc);
    UMLClassifier *c = parentDoc->getParentClassifier();
    CodeGenerationPolicy *commonPolicy = UMLApp::app()->commonPolicy();
    QString endLine = commonPolicy->getNewLineEndingChars();
    bool isInterface = parentDoc->parentIsInterface(); // a little shortcut
    QString JavaClassName = parentDoc->getJavaClassName(c->name());

    // COMMENT
    if (isInterface)
        getComment()->setText(QLatin1String("Interface ")+JavaClassName+endLine+c->doc());
    else
        getComment()->setText(QLatin1String("Class ")+JavaClassName+endLine+c->doc());

    bool forceDoc = UMLApp::app()->commonPolicy()->getCodeVerboseDocumentComments();
    if (forceDoc || !c->doc().isEmpty())
        getComment()->setWriteOutText(true);
    else
        getComment()->setWriteOutText(false);

    // Now set START/ENDING Text
    QString startText;
    // In Java, we need declare abstract only on classes
    if (c->isAbstract() && !isInterface)
        startText.append(QLatin1String("abstract "));

    if (c->visibility() != Uml::Visibility::Public) {
        // We should probably emit a warning in here .. java doesn't like to allow
        // private/protected classes. The best we can do (I believe)
        // is to let these declarations default to "package visibility"
        // which is a level between traditional "private" and "protected"
        // scopes. To get this visibility level we just print nothing..
    } else
        startText.append(QLatin1String("public "));

    if (parentDoc->parentIsInterface())
        startText.append(QLatin1String("interface "));
    else
        startText.append(QLatin1String("class "));

    startText.append(JavaClassName);

    // write inheritances out
    UMLClassifierList superclasses =
        c->findSuperClassConcepts(UMLClassifier::CLASS);
    UMLClassifierList superinterfaces =
        c->findSuperClassConcepts(UMLClassifier::INTERFACE);
    int nrof_superclasses = superclasses.count();
    int nrof_superinterfaces = superinterfaces.count();

    // write out inheritance
    int i = 0;
    if (nrof_superclasses >0)
        startText.append(QLatin1String(" extends "));
    foreach (UMLClassifier* concept, superclasses) {
        startText.append(parentDoc->cleanName(concept->name()));
        if(i != (nrof_superclasses-1))
            startText.append(QLatin1String(", "));
        i++;
    }

    // write out what we 'implement'
    i = 0;
    if(nrof_superinterfaces >0)
    {
        // In Java interfaces "extend" other interfaces. Classes "implement" interfaces
        if(isInterface)
            startText.append(QLatin1String(" extends "));
        else
            startText.append(QLatin1String(" implements "));
    }
    foreach (UMLClassifier* concept, superinterfaces) {
        startText.append(parentDoc->cleanName(concept->name()));
        if(i != (nrof_superinterfaces-1))
            startText.append(QLatin1String(", "));
        i++;
    }

    // Set the header and end text for the hier.codeblock
    setStartText(startText+QLatin1String(" {"));

    // setEndText(QLatin1String("}")); // not needed
}

void JavaClassDeclarationBlock::init (JavaClassifierCodeDocument *parentDoc, const QString &comment)
{
    setComment(new JavaCodeDocumentation(parentDoc));
    getComment()->setText(comment);
    setEndText(QLatin1String("}"));
}


