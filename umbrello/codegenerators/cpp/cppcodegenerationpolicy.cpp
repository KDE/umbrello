/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "cppcodegenerationpolicy.h"

// app includes
#include "cppcodegenerationpolicypage.h"
#include "uml.h"
#include "umbrellosettings.h"
#include "optionstate.h"

// kde includes
#include <kconfig.h>

// qt includes
#include <QRegExp>

const char * CPPCodeGenerationPolicy::DEFAULT_VECTOR_METHOD_APPEND = "%VARNAME%.push_back(value);";
const char * CPPCodeGenerationPolicy::DEFAULT_VECTOR_METHOD_REMOVE = "int size = %VARNAME%.size();\nfor (int i = 0; i < size; ++i) {\n\t%ITEMCLASS% item = %VARNAME%.at(i);\n\tif(item == value) {\n\t\tvector<%ITEMCLASS%>::iterator it = %VARNAME%.begin() + i;\n\t\t%VARNAME%.erase(it);\n\t\treturn;\n\t}\n }";
const char * CPPCodeGenerationPolicy::DEFAULT_VECTOR_METHOD_INIT = " "; // nothing to do in std::vector krazy:exclude=doublequote_chars
const char * CPPCodeGenerationPolicy::DEFAULT_OBJECT_METHOD_INIT = "%VARNAME% = new %ITEMCLASS%();";

/**
 * Constructor.
 */
CPPCodeGenerationPolicy::CPPCodeGenerationPolicy()
{
    init();
}

/**
 * Destructor.
 */
CPPCodeGenerationPolicy::~CPPCodeGenerationPolicy()
{
}

/**
 * Set the value of publicAccessors
 * @param var the new value
 */
void CPPCodeGenerationPolicy::setAccessorsArePublic(bool var)
{
    Settings::optionState().codeGenerationState.cppCodeGenerationState.publicAccessors = var;
    // @todo we should probably use an own signal for this
    UMLApp::app()->commonPolicy()->emitModifiedCodeContentSig();
}

/**
 * Get the value of m_publicAccessors
 * @return the boolean value of m_publicAccessors
 */
bool CPPCodeGenerationPolicy::getAccessorsArePublic()
{
    return Settings::optionState().codeGenerationState.cppCodeGenerationState.publicAccessors;
}

/**
 * Set the value of m_inlineAccessors
 * @param var the new value
 */
void CPPCodeGenerationPolicy::setAccessorsAreInline(bool var)
{
    Settings::optionState().codeGenerationState.cppCodeGenerationState.inlineAccessors = var;
    UMLApp::app()->commonPolicy()->emitModifiedCodeContentSig();
}

/**
 * Get the value of m_inlineAccessors.
 * @return the boolean value of m_inlineAccessors
 */
bool CPPCodeGenerationPolicy::getAccessorsAreInline()
{
    return Settings::optionState().codeGenerationState.cppCodeGenerationState.inlineAccessors;
}

/**
 * Set the value of m_inlineOperations.
 * @param var the new value
 */
void CPPCodeGenerationPolicy::setOperationsAreInline(bool var)
{
    Settings::optionState().codeGenerationState.cppCodeGenerationState.inlineOps = var;
    UMLApp::app()->commonPolicy()->emitModifiedCodeContentSig();
}

/**
 * Get the value of m_inlineOperations.
 * @return the boolean value of m_inlineOperations
 */
bool CPPCodeGenerationPolicy::getOperationsAreInline()
{
    return Settings::optionState().codeGenerationState.cppCodeGenerationState.inlineOps;
}

/**
 * Set the value of m_virtualDestructors.
 * @param var the new value
 */
void CPPCodeGenerationPolicy::setDestructorsAreVirtual(bool var)
{
    Settings::optionState().codeGenerationState.cppCodeGenerationState.virtualDestructors = var;
    UMLApp::app()->commonPolicy()->emitModifiedCodeContentSig();
}

/**
 * Get the value of m_virtualDestructors.
 * @return the boolean value of m_virtualDestructors
 */
bool CPPCodeGenerationPolicy::getDestructorsAreVirtual()
{
    return Settings::optionState().codeGenerationState.cppCodeGenerationState.virtualDestructors;
}

void CPPCodeGenerationPolicy::setGetterWithGetPrefix(bool var)
{
    Settings::optionState().codeGenerationState.cppCodeGenerationState.getterWithGetPrefix = var;
    UMLApp::app()->commonPolicy()->emitModifiedCodeContentSig();
}

bool CPPCodeGenerationPolicy::getGetterWithGetPrefix()
{
    return Settings::optionState().codeGenerationState.cppCodeGenerationState.getterWithGetPrefix;
}

void CPPCodeGenerationPolicy::setRemovePrefixFromAccessorMethods(bool var)
{
    Settings::optionState().codeGenerationState.cppCodeGenerationState.removePrefixFromAccessorMethods = var;
    UMLApp::app()->commonPolicy()->emitModifiedCodeContentSig();
}

bool CPPCodeGenerationPolicy::getRemovePrefixFromAccessorMethods()
{
    return Settings::optionState().codeGenerationState.cppCodeGenerationState.removePrefixFromAccessorMethods;
}

void CPPCodeGenerationPolicy::setAccessorMethodsStartWithUpperCase(bool var)
{
    Settings::optionState().codeGenerationState.cppCodeGenerationState.accessorMethodsStartWithUpperCase = var;
    UMLApp::app()->commonPolicy()->emitModifiedCodeContentSig();
}

bool CPPCodeGenerationPolicy::getAccessorMethodsStartWithUpperCase()
{
    return Settings::optionState().codeGenerationState.cppCodeGenerationState.accessorMethodsStartWithUpperCase;
}

/**
 * Set the value of m_packageIsNamespace.
 * @param var the new value
 */
void CPPCodeGenerationPolicy::setPackageIsNamespace(bool var)
{
    Settings::optionState().codeGenerationState.cppCodeGenerationState.packageIsNamespace = var;
    UMLApp::app()->commonPolicy()->emitModifiedCodeContentSig();
}

/**
 * Get the value of m_packageIsNamespace.
 * @return the boolean value of m_packageIsNamespace
 */
bool CPPCodeGenerationPolicy::getPackageIsNamespace()
{
    return Settings::optionState().codeGenerationState.cppCodeGenerationState.packageIsNamespace;
}

/**
 * Set the value of m_autoGenerateAccessors.
 * @param var the new value
 */
void CPPCodeGenerationPolicy::setAutoGenerateAccessors(bool var)
{
    Settings::optionState().codeGenerationState.cppCodeGenerationState.autoGenAccessors = var;
    UMLApp::app()->commonPolicy()->emitModifiedCodeContentSig();
}

/**
 * Get the value of m_autoGenerateAccessors.
 * @return the boolean value of m_autoGenerateAccessors
 */
bool CPPCodeGenerationPolicy::getAutoGenerateAccessors()
{
    return Settings::optionState().codeGenerationState.cppCodeGenerationState.autoGenAccessors;
}

QString CPPCodeGenerationPolicy::getStringClassName()
{
    return Settings::optionState().codeGenerationState.cppCodeGenerationState.stringClassName;
}

QString CPPCodeGenerationPolicy::getStringClassNameInclude()
{
    return Settings::optionState().codeGenerationState.cppCodeGenerationState.stringClassNameInclude;
}

QString CPPCodeGenerationPolicy::getVectorClassName()
{
    return Settings::optionState().codeGenerationState.cppCodeGenerationState.vectorClassName;
}

QString CPPCodeGenerationPolicy::getVectorClassNameInclude()
{
    return Settings::optionState().codeGenerationState.cppCodeGenerationState.vectorClassNameInclude;
}

void CPPCodeGenerationPolicy::setStringClassName(const QString &value)
{
    Settings::optionState().codeGenerationState.cppCodeGenerationState.stringClassName = value;
    UMLApp::app()->commonPolicy()->emitModifiedCodeContentSig();
}

void CPPCodeGenerationPolicy::setStringClassNameInclude(const QString &value)
{
    Settings::optionState().codeGenerationState.cppCodeGenerationState.stringClassNameInclude = value;
    UMLApp::app()->commonPolicy()->emitModifiedCodeContentSig();
}

void CPPCodeGenerationPolicy::setVectorClassName(const QString &value)
{
    Settings::optionState().codeGenerationState.cppCodeGenerationState.vectorClassName = value;
    UMLApp::app()->commonPolicy()->emitModifiedCodeContentSig();
}

void CPPCodeGenerationPolicy::setVectorClassNameInclude(const QString &value)
{
    Settings::optionState().codeGenerationState.cppCodeGenerationState.vectorClassNameInclude = value;
    UMLApp::app()->commonPolicy()->emitModifiedCodeContentSig();
}

void CPPCodeGenerationPolicy::setClassMemberPrefix(const QString &value)
{
    Settings::optionState().codeGenerationState.cppCodeGenerationState.classMemberPrefix = value;
    UMLApp::app()->commonPolicy()->emitModifiedCodeContentSig();
}

QString CPPCodeGenerationPolicy::getClassMemberPrefix()
{
    return Settings::optionState().codeGenerationState.cppCodeGenerationState.classMemberPrefix;
}

void CPPCodeGenerationPolicy::setDocToolTag(const QString &value)
{
    Settings::optionState().codeGenerationState.cppCodeGenerationState.docToolTag = value;
    UMLApp::app()->commonPolicy()->emitModifiedCodeContentSig();
}

QString CPPCodeGenerationPolicy::getDocToolTag()
{
    return Settings::optionState().codeGenerationState.cppCodeGenerationState.docToolTag;
}

/**
 * Determine if the string include is global.
 * @return value of flag
 */
bool CPPCodeGenerationPolicy::stringIncludeIsGlobal()
{
    return Settings::optionState().codeGenerationState.cppCodeGenerationState.stringIncludeIsGlobal;
}

/**
 * Determine if the vector include is global.
 * @return value of flag
 */
bool CPPCodeGenerationPolicy::vectorIncludeIsGlobal()
{
    return Settings::optionState().codeGenerationState.cppCodeGenerationState.vectorIncludeIsGlobal;
}

/**
 * Set flag whether string include is global.
 * @param value   the value of the flag
 */
void CPPCodeGenerationPolicy::setStringIncludeIsGlobal(bool value)
{
    Settings::optionState().codeGenerationState.cppCodeGenerationState.stringIncludeIsGlobal = value;
    UMLApp::app()->commonPolicy()->emitModifiedCodeContentSig();
}

/**
 * Set flag whether vector include is global.
 * @param value   the value of the flag
 */
void CPPCodeGenerationPolicy::setVectorIncludeIsGlobal(bool value)
{
    Settings::optionState().codeGenerationState.cppCodeGenerationState.vectorIncludeIsGlobal = value;
    UMLApp::app()->commonPolicy()->emitModifiedCodeContentSig();
}

QString CPPCodeGenerationPolicy::getVectorMethodAppend(const QString & variableName, const QString & itemClassName)
{
    QString value = m_vectorMethodAppendBase;
    if(!variableName.isEmpty())
        value.replace(QRegExp(QLatin1String("%VARNAME%")), variableName);
    value.replace(QRegExp(QLatin1String("%VECTORTYPENAME%")), Settings::optionState().codeGenerationState.cppCodeGenerationState.vectorClassName);
    if(!itemClassName.isEmpty())
        value.replace(QRegExp(QLatin1String("%ITEMCLASS%")), itemClassName);
    return value;
}

QString CPPCodeGenerationPolicy::getVectorMethodRemove(const QString & variableName, const QString & itemClassName)
{
    QString value = m_vectorMethodRemoveBase;
    if(!variableName.isEmpty())
        value.replace(QRegExp(QLatin1String("%VARNAME%")), variableName);
    value.replace(QRegExp(QLatin1String("%VECTORTYPENAME%")), Settings::optionState().codeGenerationState.cppCodeGenerationState.vectorClassName);
    if(!itemClassName.isEmpty())
        value.replace(QRegExp(QLatin1String("%ITEMCLASS%")), itemClassName);
    return value;
}

QString CPPCodeGenerationPolicy::getVectorMethodInit(const QString & variableName, const QString & itemClassName)
{
    QString value = m_vectorMethodInitBase;
    if(!variableName.isEmpty())
        value.replace(QRegExp(QLatin1String("%VARNAME%")), variableName);
    value.replace(QRegExp(QLatin1String("%VECTORTYPENAME%")), Settings::optionState().codeGenerationState.cppCodeGenerationState.vectorClassName);
    if(!itemClassName.isEmpty())
        value.replace(QRegExp(QLatin1String("%ITEMCLASS%")), itemClassName);
    return value;
}

/**
 * Be somewhat flexible about how new object classes are initialized.
 * Not sure if this should be user configureable. For now, it is not.
 * @param   variableName    variable name
 * @param   itemClassName   item class name
 * @return object method init string
 */
QString CPPCodeGenerationPolicy::getObjectMethodInit(const QString & variableName, const QString & itemClassName)
{
    QString value = m_objectMethodInitBase;
    if(!variableName.isEmpty())
        value.replace(QRegExp(QLatin1String("%VARNAME%")), variableName);
    value.replace(QRegExp(QLatin1String("%VECTORTYPENAME%")), Settings::optionState().codeGenerationState.cppCodeGenerationState.vectorClassName);
    if(!itemClassName.isEmpty())
        value.replace(QRegExp(QLatin1String("%ITEMCLASS%")), itemClassName);
    return value;
}

/**
 * Set the defaults for this code generator from the passed generator.
 * @param cppclone           code generation policy object for cloning
 * @param emitUpdateSignal   flag whether to emit update signal
 */
void CPPCodeGenerationPolicy::setDefaults(CPPCodeGenerationPolicy * cppclone, bool emitUpdateSignal)
{
    blockSignals(true); // we need to do this because otherwise most of these
    // settors below will each send the modifiedCodeContent() signal
    // needlessly (we can just make one call at the end).

    {
        setAutoGenerateAccessors(cppclone->getAutoGenerateAccessors());

        setAccessorsAreInline(cppclone->getAccessorsAreInline());
        setOperationsAreInline(cppclone->getOperationsAreInline());
        setDestructorsAreVirtual(cppclone->getDestructorsAreVirtual());
        setGetterWithGetPrefix(cppclone->getGetterWithGetPrefix());
        setRemovePrefixFromAccessorMethods(cppclone->getRemovePrefixFromAccessorMethods());
        setAccessorMethodsStartWithUpperCase(cppclone->getAccessorMethodsStartWithUpperCase());
        setPackageIsNamespace(cppclone->getPackageIsNamespace());

        setStringClassName(cppclone->getStringClassName());
        setStringClassNameInclude(cppclone->getStringClassNameInclude());
        setStringIncludeIsGlobal(cppclone->stringIncludeIsGlobal());

        setVectorClassName(cppclone->getVectorClassName());
        setVectorClassNameInclude(cppclone->getVectorClassNameInclude());
        setVectorIncludeIsGlobal(cppclone->vectorIncludeIsGlobal());

        setDocToolTag(cppclone->getDocToolTag());
        setClassMemberPrefix(cppclone->getClassMemberPrefix());
    }

    blockSignals(false); // "as you were citizen"

    if(emitUpdateSignal)
        UMLApp::app()->commonPolicy()->emitModifiedCodeContentSig();
}

/**
 * Set the defaults from a config file for this code generator from the passed KConfig pointer.
 * @param emitUpdateSignal   flag whether to emit update signal
 */
void CPPCodeGenerationPolicy::setDefaults(bool emitUpdateSignal)
{
    blockSignals(true); // we need to do this because otherwise most of these
    // settors below will each send the modifiedCodeContent() signal
    // needlessly (we can just make one call at the end).

    setAutoGenerateAccessors(UmbrelloSettings::autoGenAccessors());

    setAccessorsAreInline(UmbrelloSettings::inlineAccessors());
    setAccessorsArePublic(UmbrelloSettings::publicAccessors());
    setOperationsAreInline(UmbrelloSettings::inlineOps());
    setDestructorsAreVirtual(UmbrelloSettings::virtualDestructors());
    setGetterWithGetPrefix(UmbrelloSettings::getterWithGetPrefix());
    setRemovePrefixFromAccessorMethods(UmbrelloSettings::removePrefixFromAccessorMethods());
    setAccessorMethodsStartWithUpperCase(UmbrelloSettings::accessorMethodsStartWithUpperCase());
    setPackageIsNamespace(UmbrelloSettings::packageIsNamespace());

    setStringClassName(UmbrelloSettings::stringClassName());
    setStringClassNameInclude(UmbrelloSettings::stringClassNameInclude());
    setStringIncludeIsGlobal(UmbrelloSettings::stringIncludeIsGlobal());

    setVectorClassName(UmbrelloSettings::vectorClassName());
    setVectorClassNameInclude(UmbrelloSettings::vectorClassNameInclude());
    setVectorIncludeIsGlobal(UmbrelloSettings::vectorIncludeIsGlobal());

    setDocToolTag(UmbrelloSettings::docToolTag());
    setClassMemberPrefix(UmbrelloSettings::classMemberPrefix());

    blockSignals(false); // "as you were citizen"

    if(emitUpdateSignal)
        UMLApp::app()->commonPolicy()->emitModifiedCodeContentSig();
}

/**
 * Create a new dialog interface for this object.
 * @param parent   the parent widget
 * @param name     the name of the page
 * @return dialog object
 */
CodeGenerationPolicyPage * CPPCodeGenerationPolicy::createPage(QWidget *parent, const char *name)
{
    return new CPPCodeGenerationPolicyPage(parent, name, this);
}

/**
 * Initialisation routine.
 */
void CPPCodeGenerationPolicy::init()
{
    blockSignals(true);

    m_vectorMethodAppendBase = QLatin1String(DEFAULT_VECTOR_METHOD_APPEND);
    m_vectorMethodRemoveBase = QLatin1String(DEFAULT_VECTOR_METHOD_REMOVE);
    m_vectorMethodInitBase = QLatin1String(DEFAULT_VECTOR_METHOD_INIT);
    m_objectMethodInitBase = QLatin1String(DEFAULT_OBJECT_METHOD_INIT);

    Settings::OptionState optionState = Settings::optionState();
    setAutoGenerateAccessors(optionState.codeGenerationState.cppCodeGenerationState.autoGenAccessors);

    setAccessorsAreInline(optionState.codeGenerationState.cppCodeGenerationState.inlineAccessors);
    setAccessorsArePublic(optionState.codeGenerationState.cppCodeGenerationState.publicAccessors);
    setOperationsAreInline(optionState.codeGenerationState.cppCodeGenerationState.inlineOps);
    setDestructorsAreVirtual(optionState.codeGenerationState.cppCodeGenerationState.virtualDestructors);
    setGetterWithGetPrefix(optionState.codeGenerationState.cppCodeGenerationState.getterWithGetPrefix);
    setRemovePrefixFromAccessorMethods(optionState.codeGenerationState.cppCodeGenerationState.removePrefixFromAccessorMethods);
    setAccessorMethodsStartWithUpperCase(optionState.codeGenerationState.cppCodeGenerationState.accessorMethodsStartWithUpperCase);
    setPackageIsNamespace(optionState.codeGenerationState.cppCodeGenerationState.packageIsNamespace);

    setStringClassName(optionState.codeGenerationState.cppCodeGenerationState.stringClassName);
    setStringClassNameInclude(optionState.codeGenerationState.cppCodeGenerationState.stringClassNameInclude);
    setStringIncludeIsGlobal(optionState.codeGenerationState.cppCodeGenerationState.stringIncludeIsGlobal);

    setVectorClassName(optionState.codeGenerationState.cppCodeGenerationState.vectorClassName);
    setVectorClassNameInclude(optionState.codeGenerationState.cppCodeGenerationState.vectorClassNameInclude);
    setVectorIncludeIsGlobal(optionState.codeGenerationState.cppCodeGenerationState.vectorIncludeIsGlobal);

    setDocToolTag(optionState.codeGenerationState.cppCodeGenerationState.docToolTag);
    setClassMemberPrefix(optionState.codeGenerationState.cppCodeGenerationState.classMemberPrefix);

    blockSignals(false);
}

