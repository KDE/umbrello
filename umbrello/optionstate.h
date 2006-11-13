/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef OPTIONSTATE_H
#define OPTIONSTATE_H


#include "umlnamespace.h"
#include "codeviewerstate.h"

namespace Settings {

enum Page
{
    page_general = 0,
    page_font,
    page_UI,
    page_class,
    page_codegen,
    page_codeview
};

//public structs
struct GeneralState {
    bool undo;
    bool tabdiagrams;
    bool newcodegen;
    bool angularlines;
    bool autosave;
    int time;        //old autosave time, kept for compatibility
    int autosavetime;
    QString autosavesuffix;  ///< Text input field for suffix of autosave
    bool logo;
    bool tip;
    bool loadlast;
    Uml::Diagram_Type diagram;
    QString lastFile;
};

struct UIState {
    bool useFillColor;
    QColor fillColor;
    QColor lineColor;
    uint   lineWidth;
    QFont font;
};

struct ClassState {
    bool showVisibility;
    bool showAtts;
    bool showOps;
    bool showStereoType;
    bool showAttSig;
    bool showOpSig;
    bool showPackage;
    Uml::Visibility defaultAttributeScope;
    Uml::Visibility defaultOperationScope;
};

struct OptionState {
    GeneralState generalState;
    UIState uiState;
    ClassState classState;
    CodeViewerState codeViewerState;
};


OptionState& getOptionState();
void setOptionState(const OptionState& optstate);

}  // namespace Settings

#endif
