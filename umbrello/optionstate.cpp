/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "optionstate.h"

#include <kglobal.h>

namespace Settings {

    /*
     * Impt: This ensures creation of OptionState object after
     * QApplication there by avoiding nasty font rendering issues
     * which occurs due to creation of QFont objects before
     * QApplication object is created.
     */
    K_GLOBAL_STATIC(OptionState, opState)

    OptionState& optionState()
    {
        return *opState;
    }

    void setOptionState(const OptionState& optstate)
    {
        *opState = optstate;
    }

    void saveToXMI(QDomElement& element, const OptionState& optstate)
    {
        // uistate
        element.setAttribute("usefillcolor",     optstate.uiState.useFillColor);
        element.setAttribute("fillcolor",        optstate.uiState.fillColor.name());
        element.setAttribute("linecolor",        optstate.uiState.lineColor.name());
        element.setAttribute("linewidth",        optstate.uiState.lineWidth);
        element.setAttribute("textcolor",        optstate.uiState.textColor.name());
        element.setAttribute("font",             optstate.uiState.font.toString());
        element.setAttribute("backgroundcolor",  optstate.uiState.backgroundColor.name());
        element.setAttribute("griddotcolor",     optstate.uiState.gridDotColor.name());
        // classstate
        element.setAttribute("showattsig",       optstate.classState.showAttSig);
        element.setAttribute("showatts",         optstate.classState.showAtts);
        element.setAttribute("showopsig",        optstate.classState.showOpSig);
        element.setAttribute("showops",          optstate.classState.showOps);
        element.setAttribute("showpackage",      optstate.classState.showPackage);
        element.setAttribute("showattribassocs", optstate.classState.showAttribAssocs);
        element.setAttribute("showpubliconly",   optstate.classState.showPublicOnly);
        element.setAttribute("showscope",        optstate.classState.showVisibility);
        element.setAttribute("showstereotype",   optstate.classState.showStereoType);
    }

    bool loadFromXMI(QDomElement& element, OptionState& optstate)
    {
        // uistate
        QString usefillcolor = element.attribute("usefillcolor", "0");
        optstate.uiState.useFillColor = (bool)usefillcolor.toInt();
        QString fillcolor = element.attribute("fillcolor", "");
        if (!fillcolor.isEmpty())
            optstate.uiState.fillColor = QColor(fillcolor);
        QString linecolor = element.attribute("linecolor", "");
        if (!linecolor.isEmpty())
            optstate.uiState.lineColor = QColor(linecolor);
        QString linewidth = element.attribute("linewidth", "");
        if (!linewidth.isEmpty())
            optstate.uiState.lineWidth = linewidth.toInt();
        QString textColor = element.attribute("textcolor", "");
        if (!textColor.isEmpty())
            optstate.uiState.textColor = QColor(textColor);
        QString font = element.attribute("font", "");
        if (!font.isEmpty()) {
            optstate.uiState.font.fromString(font);
            optstate.uiState.font.setUnderline(false);
        }
        QString backgroundColor = element.attribute("backgroundcolor", "");
        if (!backgroundColor.isEmpty())
            optstate.uiState.backgroundColor = QColor(backgroundColor);
        QString gridDotColor = element.attribute("griddotcolor", "");
        if (!gridDotColor.isEmpty())
            optstate.uiState.gridDotColor = QColor(gridDotColor);
        // classstate
        QString temp = element.attribute("showattsig", "0");
        optstate.classState.showAttSig = (bool)temp.toInt();
        temp = element.attribute("showatts", "0");
        optstate.classState.showAtts = (bool)temp.toInt();
        temp = element.attribute("showopsig", "0");
        optstate.classState.showOpSig = (bool)temp.toInt();
        temp = element.attribute("showops", "0");
        optstate.classState.showOps = (bool)temp.toInt();
        temp = element.attribute("showpackage", "0");
        optstate.classState.showPackage = (bool)temp.toInt();
        temp = element.attribute("showattribassocs", "0");
        optstate.classState.showAttribAssocs = (bool)temp.toInt();
        temp = element.attribute("showscope", "0");
        optstate.classState.showVisibility = (bool)temp.toInt();
        temp = element.attribute("showstereotype", "0");
        optstate.classState.showStereoType = (bool)temp.toInt();

        return true;
    }

}  // namespace Settings
