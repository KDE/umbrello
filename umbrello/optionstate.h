/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
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

	enum Diagram
	{
	    diagram_no = 0,
	    diagram_class,
	    diagram_usecase,
	    diagram_sequence,
	    diagram_collaboration,
	    diagram_state,
	    diagram_activity,
	    diagram_component,
	    diagram_deployment
	};

	//public structs
	struct GeneralState {
		bool undo;
		bool autosave;
		int time;        //old autosave time, kept for compatibility
		int autosavetime;
		QString autosavesuffix;  ///< Text input field for suffix of autosave
		bool logo;
		bool tip;
		bool loadlast;
		Diagram diagram;
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
		bool showScope;
		bool showAtts;
		bool showOps;
		bool showStereoType;
		bool showAttSig;
		bool showOpSig;
		bool showPackage;
		Uml::Scope defaultAttributeScope;
		Uml::Scope defaultOperationScope;
	};

	struct OptionState {
		GeneralState generalState;
		UIState uiState;
		ClassState classState;
		CodeViewerState codeViewerState;
	//	CodeGenState codegenState;
	};

}

#endif
