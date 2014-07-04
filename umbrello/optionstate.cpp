/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "optionstate.h"

#include <kglobal.h>

namespace Settings {

    /*
     * Impt: This ensures creation of OptionState object after
     * QApplication thereby avoiding nasty font rendering issues
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
        element.setAttribute(QLatin1String("usefillcolor"),     optstate.uiState.useFillColor);
        element.setAttribute(QLatin1String("fillcolor"),        optstate.uiState.fillColor.name());
        element.setAttribute(QLatin1String("linecolor"),        optstate.uiState.lineColor.name());
        element.setAttribute(QLatin1String("linewidth"),        optstate.uiState.lineWidth);
        element.setAttribute(QLatin1String("textcolor"),        optstate.uiState.textColor.name());
        element.setAttribute(QLatin1String("font"),             optstate.uiState.font.toString());
        element.setAttribute(QLatin1String("backgroundcolor"),  optstate.uiState.backgroundColor.name());
        element.setAttribute(QLatin1String("griddotcolor"),     optstate.uiState.gridDotColor.name());
        // classstate
        element.setAttribute(QLatin1String("showattsig"),       optstate.classState.showAttSig);
        element.setAttribute(QLatin1String("showatts"),         optstate.classState.showAtts);
        element.setAttribute(QLatin1String("showopsig"),        optstate.classState.showOpSig);
        element.setAttribute(QLatin1String("showops"),          optstate.classState.showOps);
        element.setAttribute(QLatin1String("showpackage"),      optstate.classState.showPackage);
        element.setAttribute(QLatin1String("showattribassocs"), optstate.classState.showAttribAssocs);
        element.setAttribute(QLatin1String("showpubliconly"),   optstate.classState.showPublicOnly);
        element.setAttribute(QLatin1String("showscope"),        optstate.classState.showVisibility);
        element.setAttribute(QLatin1String("showstereotype"),   optstate.classState.showStereoType);
    }

    bool loadFromXMI(QDomElement& element, OptionState& optstate)
    {
        // uistate
        QString usefillcolor = element.attribute(QLatin1String("usefillcolor"), QLatin1String("0"));
        optstate.uiState.useFillColor = (bool)usefillcolor.toInt();
        QString fillcolor = element.attribute(QLatin1String("fillcolor"));
        if (!fillcolor.isEmpty())
            optstate.uiState.fillColor = QColor(fillcolor);
        QString linecolor = element.attribute(QLatin1String("linecolor"));
        if (!linecolor.isEmpty())
            optstate.uiState.lineColor = QColor(linecolor);
        QString linewidth = element.attribute(QLatin1String("linewidth"));
        if (!linewidth.isEmpty())
            optstate.uiState.lineWidth = linewidth.toInt();
        QString textColor = element.attribute(QLatin1String("textcolor"));
        if (!textColor.isEmpty())
            optstate.uiState.textColor = QColor(textColor);
        QString font = element.attribute(QLatin1String("font"));
        if (!font.isEmpty()) {
            optstate.uiState.font.fromString(font);
            optstate.uiState.font.setUnderline(false);
        }
        QString backgroundColor = element.attribute(QLatin1String("backgroundcolor"));
        if (!backgroundColor.isEmpty())
            optstate.uiState.backgroundColor = QColor(backgroundColor);
        QString gridDotColor = element.attribute(QLatin1String("griddotcolor"));
        if (!gridDotColor.isEmpty())
            optstate.uiState.gridDotColor = QColor(gridDotColor);
        // classstate
        QString temp = element.attribute(QLatin1String("showattsig"), QLatin1String("0"));
        optstate.classState.showAttSig = (bool)temp.toInt();
        temp = element.attribute(QLatin1String("showatts"), QLatin1String("0"));
        optstate.classState.showAtts = (bool)temp.toInt();
        temp = element.attribute(QLatin1String("showopsig"), QLatin1String("0"));
        optstate.classState.showOpSig = (bool)temp.toInt();
        temp = element.attribute(QLatin1String("showops"), QLatin1String("0"));
        optstate.classState.showOps = (bool)temp.toInt();
        temp = element.attribute(QLatin1String("showpackage"), QLatin1String("0"));
        optstate.classState.showPackage = (bool)temp.toInt();
        temp = element.attribute(QLatin1String("showattribassocs"), QLatin1String("0"));
        optstate.classState.showAttribAssocs = (bool)temp.toInt();
        temp = element.attribute(QLatin1String("showscope"), QLatin1String("0"));
        optstate.classState.showVisibility = (bool)temp.toInt();
        temp = element.attribute(QLatin1String("showstereotype"), QLatin1String("0"));
        optstate.classState.showStereoType = (bool)temp.toInt();
        temp = element.attribute(QLatin1String("showpubliconly"), QLatin1String("0"));
        optstate.classState.showPublicOnly = (bool)temp.toInt();

        return true;
    }

}  // namespace Settings
