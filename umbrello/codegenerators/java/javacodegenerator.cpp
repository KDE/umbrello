/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "javacodegenerator.h"

// local includes
#include "javacodecomment.h"
#include "codeviewerdialog.h"
#include "uml.h"
#include "umbrellosettings.h"

// kde includes
#include <kconfig.h>
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QRegExp>

static const char *reserved_words[] = {
    "abstract",
    "AbstractMethodError",
    "ArithmeticException",
    "ArrayIndexOutOfBoundsException",
    "ArrayStoreException",
    "assert",
    "AssertionError",
    "auto",
    "boolean",
    "Boolean",
    "break",
    "byte",
    "Byte",
    "catch",
    "char",
    "Character",
    "CharSequence",
    "Class",
    "ClassCastException",
    "ClassCircularityError",
    "ClassFormatError",
    "ClassLoader",
    "ClassNotFoundException",
    "clone",
    "Cloneable",
    "CloneNotSupportedException",
    "Comparable",
    "Compiler",
    "const",
    "continue",
    "default",
    "delete",
    "do",
    "double",
    "Double",
    "else",
    "enum",
    "equals",
    "Error",
    "Exception",
    "ExceptionInInitializerError",
    "extends",
    "extern",
    "false",
    "final",
    "finalize",
    "finally",
    "float",
    "Float",
    "for",
    "friend",
    "getClass",
    "goto",
    "hashCode",
    "if",
    "IllegalAccessError",
    "IllegalAccessException",
    "IllegalArgumentException",
    "IllegalMonitorStateException",
    "IllegalStateException",
    "IllegalThreadStateException",
    "implements",
    "import",
    "IncompatibleClassChangeError",
    "IndexOutOfBoundsException",
    "InheritableThreadLocal",
    "inline",
    "instanceof",
    "InstantiationError",
    "InstantiationException",
    "int",
    "Integer",
    "interface",
    "InternalError",
    "InterruptedException",
    "LinkageError",
    "long",
    "Long",
    "Math",
    "native",
    "NegativeArraySizeException",
    "new",
    "nextgroup=javaUserLabelRef",
    "NoClassDefFoundError",
    "NoSuchFieldError",
    "NoSuchFieldException",
    "NoSuchMethodError",
    "NoSuchMethodException",
    "notifyAll",
    "null",
    "NullPointerException",
    "Number",
    "NumberFormatException",
    "Object",
    "operator",
    "OutOfMemoryError",
    "package",
    "Package",
    "private",
    "Process",
    "protected",
    "redeclared",
    "register",
    "return",
    "Runnable",
    "Runtime",
    "RuntimeException",
    "RuntimePermission",
    "SecurityException",
    "SecurityManager",
    "serializable",
    "short",
    "Short",
    "signed",
    "sizeof",
    "skipwhite",
    "StackOverflowError",
    "StackTraceElement",
    "static",
    "strictfp",
    "StrictMath",
    "String",
    "StringBuffer",
    "StringIndexOutOfBoundsException",
    "struct",
    "super",
    "switch",
    "synchronized",
    "template",
    "this",
    "Thread",
    "ThreadDeath",
    "ThreadGroup",
    "ThreadLocal",
    "throw",
    "Throwable",
    "throws",
    "toString",
    "transient",
    "true",
    "try",
    "typedef",
    "union",
    "UnknownError",
    "UnsatisfiedLinkError",
    "unsigned",
    "UnsupportedClassVersionError",
    "UnsupportedOperationException",
    "VirtualMachineError",
    "void",
    "Void",
    "volatile",
    "wait",
    "while",
    0
};

/**
 * Constructor.
 */
JavaCodeGenerator::JavaCodeGenerator()
  : AdvancedCodeGenerator()
{
    // load Classifier documents from parent document
    //initFromParentDocument();

    // add in an ANT document
    JavaANTCodeDocument * buildDoc = newANTCodeDocument();
    addCodeDocument(buildDoc);

    // set our 'writeout' policy for that code document
    setCreateANTBuildFile(UmbrelloSettings::buildANTDocumentJava());
    connectSlots();
}

/**
 * Destructor.
 */
JavaCodeGenerator::~JavaCodeGenerator()
{
}

/**
 * Return "Java".
 * @return programming language identifier
 */
Uml::ProgrammingLanguage::Enum JavaCodeGenerator::language() const
{
    return Uml::ProgrammingLanguage::Java;
}

/**
 * Set the value of m_createANTBuildFile
 * @param buildIt the new value of m_createANTBuildFile
 */
void JavaCodeGenerator::setCreateANTBuildFile(bool buildIt)
{
    m_createANTBuildFile = buildIt;
    CodeDocument * antDoc = findCodeDocumentByID(QStringLiteral("ANTDOC"));
    if (antDoc)
        antDoc->setWriteOutCode(buildIt);
}

/**
 * Get the value of m_createANTBuildFile
 * @return the value of m_createANTBuildFile
 */
bool JavaCodeGenerator::getCreateANTBuildFile()
{
    return m_createANTBuildFile;
}

/**
 * Get the editing dialog for this code document.
 * In the Java version, we make the ANT build file also available.
 */
CodeViewerDialog * JavaCodeGenerator::getCodeViewerDialog(QWidget* parent, CodeDocument *doc,
        Settings::CodeViewerState & state)
{
    CodeViewerDialog *dialog = new CodeViewerDialog(parent, doc, state);
    if(getCreateANTBuildFile())
        dialog->addCodeDocument(findCodeDocumentByID(QStringLiteral("ANTDOC")));
    return dialog;
}

/**
 * Utility function for getting the java code generation policy.
 */
JavaCodeGenerationPolicy * JavaCodeGenerator::getJavaPolicy()
{
    return dynamic_cast<JavaCodeGenerationPolicy*>(UMLApp::app()->policyExt());
}

/**
 * A utility method to get the javaCodeGenerationPolicy()->getAutoGenerateAttribAccessors() value.
 */
bool JavaCodeGenerator::getAutoGenerateAttribAccessors()
{
    return getJavaPolicy()->getAutoGenerateAttribAccessors ();
}

/**
 * A utility method to get the javaCodeGenerationPolicy()->getAutoGenerateAssocAccessors() value.
 */
bool JavaCodeGenerator::getAutoGenerateAssocAccessors()
{
    return getJavaPolicy()->getAutoGenerateAssocAccessors ();
}

/**
 * Get the list variable class name to use. For Java, we have set this to "Vector".
 */
QString JavaCodeGenerator::getListFieldClassName()
{
    return QStringLiteral("Vector");
}

/**
 * IF the type is "string" we need to declare it as
 * the Java Object "String" (there is no string primitive in Java).
 * Same thing again for "bool" to "boolean".
 */
QString JavaCodeGenerator::fixTypeName(const QString &string)
{
    if (string.isEmpty() || string.contains(QRegExp(QStringLiteral("^\\s+$"))))
        return QStringLiteral("void");
    if (string == QStringLiteral("string"))
        return QStringLiteral("String");
    if (string == QStringLiteral("bool"))
        return QStringLiteral("boolean");
    return cleanName(string);
}

/**
 * Create ANT code document.
 * @return JavaANTCodeDocument object
 */
JavaANTCodeDocument * JavaCodeGenerator::newANTCodeDocument()
{
    return new JavaANTCodeDocument();
}

/**
 * Create a classifier code document.
 * @param classifier   the UML classifier
 * @return the created classifier code document
 */
CodeDocument * JavaCodeGenerator::newClassifierCodeDocument(UMLClassifier * classifier)
{
    JavaClassifierCodeDocument * doc = new JavaClassifierCodeDocument(classifier);
    doc->initCodeClassFields();
    return doc;
}

/**
 * Adds Java's primitives as datatypes.
 * @return a string list of Java primitives
 */
QStringList JavaCodeGenerator::defaultDatatypes() const
{
    QStringList l;
    l.append(QStringLiteral("int"));
    l.append(QStringLiteral("char"));
    l.append(QStringLiteral("boolean"));
    l.append(QStringLiteral("float"));
    l.append(QStringLiteral("double"));
    l.append(QStringLiteral("byte"));
    l.append(QStringLiteral("short"));
    l.append(QStringLiteral("long"));
    l.append(QStringLiteral("String"));
    l.append(QStringLiteral("Integer"));
    l.append(QStringLiteral("Character"));
    l.append(QStringLiteral("Boolean"));
    l.append(QStringLiteral("Float"));
    l.append(QStringLiteral("Double"));
    l.append(QStringLiteral("Byte"));
    l.append(QStringLiteral("Short"));
    l.append(QStringLiteral("Long"));
    l.append(QStringLiteral("StringBuffer"));
    l.append(QStringLiteral("StringBuilder"));
    return l;
}

/**
 * Get list of reserved keywords.
 * @return the string list of reserved keywords for Java
 */
QStringList JavaCodeGenerator::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        for (int i = 0; reserved_words[i]; ++i) {
            keywords.append(QLatin1String(reserved_words[i]));
        }
    }

    return keywords;
}


