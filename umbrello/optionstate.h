/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef OPTIONSTATE_H
#define OPTIONSTATE_H

#include "basictypes.h"
#include "codeviewerstate.h"
#include "codegenerationpolicy.h"

#include <QDomElement>
#include <QXmlStreamWriter>

namespace Settings {

enum Page
{
    page_general = 0,
    page_font,
    page_UI,
    page_class,
    page_codeimport,
    page_codegen,
    page_codeview
};

//public structs
class GeneralState {
public:
    GeneralState()
      : undo(false),
        tabdiagrams(false),
        newcodegen(false),
        layoutType(Uml::LayoutType::Undefined),
        footerPrinting(false),
        autosave(false),
        time(0),
        autosavetime(0),
        loadlast(false),
        diagram(Uml::DiagramType::Class),
        defaultLanguage(Uml::ProgrammingLanguage::Cpp),
        uml2(false)
    {
    }

    void load();
    void save();

    bool undo;
    bool tabdiagrams;
    bool newcodegen;
    Uml::LayoutType::Enum layoutType;
    bool footerPrinting;
    bool autosave;
    int time;        ///< old autosave time, kept for compatibility
    int autosavetime;
    QString autosavesuffix;  ///< Text input field for suffix of autosave
    bool loadlast;
    Uml::DiagramType::Enum diagram;
    Uml::ProgrammingLanguage::Enum defaultLanguage;
    QString lastFile;
    bool uml2;
};

class UIState {
public:
    UIState()
     : useFillColor(false),
       lineWidth(0)
    {
    }

    void load();
    void save();

    void saveToXMI(QXmlStreamWriter& writer);
    bool loadFromXMI(QDomElement& element);

    bool   useFillColor;
    QColor fillColor;
    QColor lineColor;
    uint   lineWidth;
    QColor textColor;
    QFont  font;
    QColor backgroundColor;
    QColor gridDotColor;
};

class ClassState {
public:
    ClassState()
      : showVisibility(false),
        showAtts(false),
        showOps(false),
        showStereoType(Uml::ShowStereoType::None),
        showAttSig(false),
        showOpSig(false),
        showPackage(false),
        showAttribAssocs(false),
        showDocumentation(false),
        showPublicOnly(false),
        defaultAttributeScope(Uml::Visibility::Public),
        defaultOperationScope(Uml::Visibility::Public)
    {
    }

    void load();
    void save();

    void saveToXMI(QXmlStreamWriter& writer);
    bool loadFromXMI(QDomElement& element);

    bool showVisibility;
    bool showAtts;
    bool showOps;
    Uml::ShowStereoType::Enum showStereoType;
    bool showAttSig;
    bool showOpSig;
    bool showPackage;
    bool showAttribAssocs;
    bool showDocumentation;
    bool showPublicOnly;
    Uml::Visibility::Enum defaultAttributeScope;
    Uml::Visibility::Enum defaultOperationScope;
};

class CodeGenerationState {
public:
    CodeGenerationState()
      : autoGenEmptyConstructors(false),
        commentStyle(CodeGenerationPolicy::SingleLine),
        defaultAssocFieldScope(Uml::Visibility::Public),
        defaultAttributeAccessorScope(Uml::Visibility::Public),
        forceDoc(false),
        writeSectionComments(CodeGenerationPolicy::None),
        includeHeadings(false),
        indentationAmount(false),
        indentationType(CodeGenerationPolicy::NONE),
#ifdef Q_OS_WIN
        lineEndingType(CodeGenerationPolicy::DOS),
#elif defined(Q_OS_MAC)
        lineEndingType(CodeGenerationPolicy::MAC),
#else
        lineEndingType(CodeGenerationPolicy::UNIX),
#endif
        modnamePolicy(CodeGenerationPolicy::No),
        overwritePolicy(CodeGenerationPolicy::Ok)
    {
    }

    void load();
    void save();

    bool autoGenEmptyConstructors;
    CodeGenerationPolicy::CommentStyle commentStyle;
    Uml::Visibility::Enum defaultAssocFieldScope;
    Uml::Visibility::Enum defaultAttributeAccessorScope;
    bool forceDoc;
    CodeGenerationPolicy::WriteSectionCommentsPolicy writeSectionComments;
    QDir headingsDirectory;
    bool includeHeadings;
    int indentationAmount;
    CodeGenerationPolicy::IndentationType indentationType;
    CodeGenerationPolicy::NewLineType lineEndingType;
    CodeGenerationPolicy::ModifyNamePolicy modnamePolicy;
    QDir outputDirectory;
    CodeGenerationPolicy::OverwritePolicy overwritePolicy;

    class CPPCodeGenerationState {
    public:
        CPPCodeGenerationState()
          : autoGenAccessors(false),
            inlineAccessors(false),
            inlineOps(false),
            packageIsNamespace(false),
            publicAccessors(false),
            stringIncludeIsGlobal(false),
            vectorIncludeIsGlobal(false),
            virtualDestructors(false),
            getterWithGetPrefix(false),
            removePrefixFromAccessorMethods(false),
            accessorMethodsStartWithUpperCase(false)
        {
        }

        bool autoGenAccessors;
        bool inlineAccessors;
        bool inlineOps;
        bool packageIsNamespace;
        bool publicAccessors;
        QString stringClassName;
        QString stringClassNameInclude;
        bool stringIncludeIsGlobal;
        QString vectorClassName;
        QString vectorClassNameInclude;
        QString classMemberPrefix;
        QString docToolTag;
        bool vectorIncludeIsGlobal;
        bool virtualDestructors;
        bool getterWithGetPrefix;
        bool removePrefixFromAccessorMethods;
        bool accessorMethodsStartWithUpperCase;
    };

    class DCodeGenerationState {
    public:
        DCodeGenerationState()
          : autoGenerateAttributeAccessors(false),
            autoGenerateAssocAccessors(false),
            buildANTDocument(false)
        {
        }

        bool autoGenerateAttributeAccessors;
        bool autoGenerateAssocAccessors;
        bool buildANTDocument;
    };

    class JavaCodeGenerationState {
    public:
        JavaCodeGenerationState()
          : autoGenerateAttributeAccessors(false),
            autoGenerateAssocAccessors(false),
            buildANTDocument(false)
        {
        }

        bool autoGenerateAttributeAccessors;
        bool autoGenerateAssocAccessors;
        bool buildANTDocument;
    };

    class RubyCodeGenerationState {
    public:
        RubyCodeGenerationState()
         : autoGenerateAttributeAccessors(false),
           autoGenerateAssocAccessors(false)
        {
        }

        bool autoGenerateAttributeAccessors;
        bool autoGenerateAssocAccessors;
    };

    CPPCodeGenerationState cppCodeGenerationState;
    DCodeGenerationState dCodeGenerationState;
    JavaCodeGenerationState javaCodeGenerationState;
    RubyCodeGenerationState rubyCodeGenerationState;
};

class CodeImportState {
public:
    CodeImportState()
      : createArtifacts(false),
        resolveDependencies(false),
        supportCPP11(true)
    {
    }

    void load();
    void save();

    void saveToXMI(QXmlStreamWriter& writer);
    bool loadFromXMI(QDomElement& element);

    bool createArtifacts;
    bool resolveDependencies;
    bool supportCPP11;
};

class AutoLayoutState {
public:
    AutoLayoutState()
     : autoDotPath(false),
       showExportLayout(false)
    {
    }

    void load();
    void save();

    void saveToXMI(QXmlStreamWriter& writer);
    bool loadFromXMI(QDomElement& element);

    bool autoDotPath;  ///< determine path to dot executable automatically
    QString dotPath;  ///< path to dot executable
    bool showExportLayout;  ///< flag for display export layout
};

class LayoutTypeState {
public:
    LayoutTypeState()
     : showExportLayout(Uml::LayoutType::Enum::Direct)
    {
    }

    void load();
    void save();

    void saveToXMI(QXmlStreamWriter& writer);
    bool loadFromXMI(QDomElement& element);

    Uml::LayoutType::Enum  showExportLayout;  ///< flag for display export layout
};

class OptionState {
public:
    OptionState();

    void load();
    void save();

    void saveToXMI(QXmlStreamWriter& writer);
    bool loadFromXMI(QDomElement& element);

    static OptionState &instance();

    GeneralState        generalState;
    LayoutTypeState     layoutTypeState;
    UIState             uiState;
    ClassState          classState;
    CodeViewerState     codeViewerState;
    CodeGenerationState codeGenerationState;
    CodeImportState     codeImportState;
    AutoLayoutState     autoLayoutState;
};

OptionState& optionState();
void setOptionState(const OptionState& optstate);

}  // namespace Settings

#endif
