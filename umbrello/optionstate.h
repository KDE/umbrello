/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef OPTIONSTATE_H
#define OPTIONSTATE_H

#include "basictypes.h"
#include "codeviewerstate.h"
#include "codegenerationpolicy.h"

#include <QtXml/QDomElement>

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
struct GeneralState {
    bool undo;
    bool tabdiagrams;
    bool newcodegen;
    bool angularlines;
    bool footerPrinting;
    bool autosave;
    int time;        ///< old autosave time, kept for compatibility
    int autosavetime;
    QString autosavesuffix;  ///< Text input field for suffix of autosave
    bool loadlast;
    Uml::DiagramType::Value diagram;
    Uml::ProgrammingLanguage::Value defaultLanguage;
    QString lastFile;
};

struct UIState {
    bool   useFillColor;
    QColor fillColor;
    QColor lineColor;
    uint   lineWidth;
    QColor textColor;
    QFont  font;
    QColor backgroundColor;
    QColor gridDotColor;
};

struct ClassState {
    bool showVisibility;
    bool showAtts;
    bool showOps;
    bool showStereoType;
    bool showAttSig;
    bool showOpSig;
    bool showPackage;
    bool showAttribAssocs;
    bool showPublicOnly;
    Uml::Visibility defaultAttributeScope;
    Uml::Visibility defaultOperationScope;
};

struct CodeGenerationState {
    bool autoGenEmptyConstructors;
    CodeGenerationPolicy::CommentStyle commentStyle;
    Uml::Visibility::Value defaultAssocFieldScope;
    Uml::Visibility::Value defaultAttributeAccessorScope;
    bool forceDoc;
    bool forceSections;
    QDir headingsDirectory;
    bool includeHeadings;
    int indentationAmount;
    CodeGenerationPolicy::IndentationType indentationType;
    CodeGenerationPolicy::NewLineType lineEndingType;
    CodeGenerationPolicy::ModifyNamePolicy modnamePolicy;
    QDir outputDirectory;
    CodeGenerationPolicy::OverwritePolicy overwritePolicy;

    struct CPPCodeGenerationState {
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
        QString docToolTag;
        bool vectorIncludeIsGlobal;
        bool virtualDestructors;
    };

    struct DCodeGenerationState {
        bool autoGenerateAttributeAccessors;
        bool autoGenerateAssocAccessors;
        bool buildANTDocument;
    };

    struct JavaCodeGenerationState{
        bool autoGenerateAttributeAccessors;
        bool autoGenerateAssocAccessors;
        bool buildANTDocument;
    };

    struct RubyCodeGenerationState{
        bool autoGenerateAttributeAccessors;
        bool autoGenerateAssocAccessors;
    };

    CPPCodeGenerationState cppCodeGenerationState;
    DCodeGenerationState dCodeGenerationState;
    JavaCodeGenerationState javaCodeGenerationState;
    RubyCodeGenerationState rubyCodeGenerationState;
};

struct CodeImportState {
    bool createArtifacts;
};

struct OptionState {
    GeneralState        generalState;
    UIState             uiState;
    ClassState          classState;
    CodeViewerState     codeViewerState;
    CodeGenerationState codeGenerationState;
    CodeImportState     codeImportState;
};

OptionState& optionState();
void setOptionState(const OptionState& optstate);
void saveToXMI(QDomElement& element, const OptionState& optstate);
bool loadFromXMI(QDomElement& element, OptionState& optstate);

}  // namespace Settings

#endif
