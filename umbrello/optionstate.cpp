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

#if QT_VERSION < 0x050000
#include <kglobal.h>
#endif

namespace Settings {

    /**
     * Save instance into a QDomElement.
     * @param element A QDomElement representing xml element data.
     */
    void ClassState::SaveToXMI1(QDomElement &element)
    {
        element.setAttribute(QLatin1String("showattribassocs"), showAttribAssocs);
        element.setAttribute(QLatin1String("showatts"),         showAtts);
        element.setAttribute(QLatin1String("showattsig"),       showAttSig);
        element.setAttribute(QLatin1String("showops"),          showOps);
        element.setAttribute(QLatin1String("showopsig"),        showOpSig);
        element.setAttribute(QLatin1String("showpackage"),      showPackage);
        element.setAttribute(QLatin1String("showpubliconly"),   showPublicOnly);
        element.setAttribute(QLatin1String("showscope"),        showVisibility);
#ifdef ENABLE_WIDGET_SHOW_DOC
        element.setAttribute(QLatin1String("showdocumentation"),showDocumentation);
#endif
        element.setAttribute(QLatin1String("showstereotype"),   showStereoType);
    }

    /**
     * Load instance from a QDomElement.
     * @param element A QDomElement representing xml element data.
     * @return true on success
     * @return false on error
     */
    bool ClassState::loadFromXMI1(QDomElement &element)
    {
        QString temp = element.attribute(QLatin1String("showattribassocs"), QLatin1String("0"));
        showAttribAssocs = (bool)temp.toInt();
        temp = element.attribute(QLatin1String("showatts"), QLatin1String("0"));
        showAtts = (bool)temp.toInt();
        temp = element.attribute(QLatin1String("showattsig"), QLatin1String("0"));
        showAttSig = (bool)temp.toInt();
        temp = element.attribute(QLatin1String("showops"), QLatin1String("0"));
        showOps = (bool)temp.toInt();
        temp = element.attribute(QLatin1String("showopsig"), QLatin1String("0"));
        showOpSig = (bool)temp.toInt();
        temp = element.attribute(QLatin1String("showpackage"), QLatin1String("0"));
        showPackage = (bool)temp.toInt();
#ifdef ENABLE_WIDGET_SHOW_DOC
        temp = element.attribute(QLatin1String("showdocumentation"), QLatin1String("0"));
        showDocumentation = (bool)temp.toInt();
#endif
        temp = element.attribute(QLatin1String("showpubliconly"), QLatin1String("0"));
        showPublicOnly = (bool)temp.toInt();
        temp = element.attribute(QLatin1String("showscope"), QLatin1String("0"));
        showVisibility = (bool)temp.toInt();
        temp = element.attribute(QLatin1String("showstereotype"), QLatin1String("0"));
        showStereoType = (bool)temp.toInt();
        return true;
    }

    /**
     * Save instance into a QDomElement.
     * @param element A QDomElement representing xml element data.
     */
    void UIState::SaveToXMI1(QDomElement &element)
    {
        element.setAttribute(QLatin1String("backgroundcolor"),  backgroundColor.name());
        element.setAttribute(QLatin1String("fillcolor"),        fillColor.name());
        element.setAttribute(QLatin1String("font"),             font.toString());
        element.setAttribute(QLatin1String("griddotcolor"),     gridDotColor.name());
        element.setAttribute(QLatin1String("linecolor"),        lineColor.name());
        element.setAttribute(QLatin1String("linewidth"),        lineWidth);
        element.setAttribute(QLatin1String("textcolor"),        textColor.name());
        element.setAttribute(QLatin1String("usefillcolor"),     useFillColor);
    }

    /**
     * Load instance from a QDomElement.
     * @param element A QDomElement representing xml element data.
     * @return true on success
     * @return false on error
     */
    bool UIState::loadFromXMI1(QDomElement &element)
    {
        QString backgroundColor = element.attribute(QLatin1String("backgroundcolor"));
        if (!backgroundColor.isEmpty())
            this->backgroundColor = QColor(backgroundColor);
        QString fillcolor = element.attribute(QLatin1String("fillcolor"));
        if (!fillcolor.isEmpty())
            this->fillColor = QColor(fillcolor);
        QString font = element.attribute(QLatin1String("font"));
        if (!font.isEmpty()) {
            this->font.fromString(font);
            this->font.setUnderline(false);
        }
        QString gridDotColor = element.attribute(QLatin1String("griddotcolor"));
        if (!gridDotColor.isEmpty())
            this->gridDotColor = QColor(gridDotColor);
        QString linecolor = element.attribute(QLatin1String("linecolor"));
        if (!linecolor.isEmpty())
            this->lineColor = QColor(linecolor);
        QString linewidth = element.attribute(QLatin1String("linewidth"));
        if (!linewidth.isEmpty())
            this->lineWidth = linewidth.toInt();
        QString textColor = element.attribute(QLatin1String("textcolor"));
        if (!textColor.isEmpty())
            this->textColor = QColor(textColor);
        QString usefillcolor = element.attribute(QLatin1String("usefillcolor"), QLatin1String("0"));
        this->useFillColor = (bool)usefillcolor.toInt();

        return true;
    }

    /**
     * Save instance into a QDomElement.
     * @param element A QDomElement representing xml element data.
     */
    void CodeImportState::SaveToXMI1(QDomElement &element)
    {
        element.setAttribute(QLatin1String("createartifacts"), createArtifacts);
        element.setAttribute(QLatin1String("resolvedependencies"), resolveDependencies);
    }

    /**
     * Load instance from a QDomElement.
     * @param element A QDomElement representing xml element data.
     * @return true on success
     * @return false on error
     */
    bool CodeImportState::loadFromXMI1(QDomElement &element)
    {
        QString temp = element.attribute(QLatin1String("createartifacts"), QLatin1String("0"));
        createArtifacts = (bool)temp.toInt();
        temp = element.attribute(QLatin1String("resolvedependencies"), QLatin1String("0"));
        resolveDependencies = (bool)temp.toInt();

        return true;
    }

    OptionState& optionState()
    {
        return OptionState::instance();
    }

    void setOptionState(const OptionState& optstate)
    {
        OptionState::instance() = optstate;
    }

    OptionState::OptionState()
    {
    }

    /**
     * Save instance into a QDomElement.
     * @param element A QDomElement representing xml element data.
     */
    void OptionState::SaveToXMI1(QDomElement& element)
    {
        uiState.SaveToXMI1(element);
        classState.SaveToXMI1(element);
    }

    /**
     * Load instance from a QDomElement.
     * @param element A QDomElement representing xml element data.
     * @return true on success
     * @return false on error
     */
    bool OptionState::loadFromXMI1(QDomElement& element)
    {
        uiState.loadFromXMI1(element);
        classState.loadFromXMI1(element);

        return true;
    }

    OptionState &OptionState::instance()
    {
    /*
     * Impt: This ensures creation of OptionState object after
     * QApplication thereby avoiding nasty font rendering issues
     * which occurs due to creation of QFont objects before
     * QApplication object is created.
     *
     * QScopedPointer usage covers object destroy on application
     * exit to avoid a memory leak.
     */
        static QScopedPointer<OptionState> optionState(new OptionState);
        return *optionState;
    }

}  // namespace Settings
