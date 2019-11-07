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
#include "umbrellosettings.h"

#if QT_VERSION < 0x050000
#include <kglobal.h>
#endif

namespace Settings {

    void GeneralState::load()
    {
        undo = UmbrelloSettings::undo();
        tabdiagrams = UmbrelloSettings::tabdiagrams();
    #ifdef ENABLE_NEW_CODE_GENERATORS
        newcodegen = UmbrelloSettings::newcodegen();
    #endif
        angularlines = UmbrelloSettings::angularlines();
        layoutType = UmbrelloSettings::layoutType();
        footerPrinting =  UmbrelloSettings::footerPrinting();
        uml2 = UmbrelloSettings::uml2();
        autosave =  UmbrelloSettings::autosave();
        time =  UmbrelloSettings::time(); //old autosavetime value kept for compatibility
        autosavetime =  UmbrelloSettings::autosavetime();
        //if we don't have a "new" autosavetime value, convert the old one
        if (autosavetime == 0) {
            switch (time) {
            case 0: autosavetime =  5; break;
            case 1: autosavetime = 10; break;
            case 2: autosavetime = 15; break;
            case 3: autosavetime = 20; break;
            case 4: autosavetime = 25; break;
            default: autosavetime = 5; break;
            }
        }

        autosavesuffix =  UmbrelloSettings::autosavesuffix();
        loadlast =  UmbrelloSettings::loadlast();
        diagram  = UmbrelloSettings::diagram();
        defaultLanguage =  UmbrelloSettings::defaultLanguage();
    }

    void GeneralState::save()
    {
        UmbrelloSettings::setUndo(undo);
        UmbrelloSettings::setTabdiagrams(tabdiagrams);
        UmbrelloSettings::setNewcodegen(newcodegen);
        UmbrelloSettings::setAngularlines(angularlines);
        UmbrelloSettings::setFooterPrinting(footerPrinting);
        UmbrelloSettings::setAutosave(autosave);
        UmbrelloSettings::setTime(time);
        UmbrelloSettings::setAutosavetime(autosavetime);
        UmbrelloSettings::setAutosavesuffix(autosavesuffix);
        UmbrelloSettings::setLoadlast(loadlast);
        UmbrelloSettings::setUml2(uml2);
        UmbrelloSettings::setDiagram(diagram);
        UmbrelloSettings::setDefaultLanguage(defaultLanguage);
    }

    void ClassState::load()
    {
        showVisibility =  UmbrelloSettings::showVisibility();
        showAtts =  UmbrelloSettings::showAtts();
        showOps =  UmbrelloSettings::showOps();
        showStereoType =  UmbrelloSettings::showStereoType();
        showAttSig =  UmbrelloSettings::showAttSig();
        showOpSig =  UmbrelloSettings::showOpSig();
        showPackage =  UmbrelloSettings::showPackage();
        showAttribAssocs =  UmbrelloSettings::showAttribAssocs();
        showPublicOnly =  UmbrelloSettings::showPublicOnly();
        defaultAttributeScope = UmbrelloSettings::defaultAttributeScope();
        defaultOperationScope = UmbrelloSettings::defaultOperationScope();
    }

    void ClassState::save()
    {
        UmbrelloSettings::setShowVisibility(showVisibility);
        UmbrelloSettings::setShowAtts(showAtts);
        UmbrelloSettings::setShowOps(showOps);
        UmbrelloSettings::setShowStereoType(showStereoType);
        UmbrelloSettings::setShowAttSig(showAttSig);
        UmbrelloSettings::setShowOpSig(showOpSig);
        UmbrelloSettings::setShowPackage(showPackage);
        UmbrelloSettings::setShowAttribAssocs(showAttribAssocs);
        UmbrelloSettings::setShowPublicOnly(showPublicOnly);
        UmbrelloSettings::setDefaultAttributeScope(defaultAttributeScope);
        UmbrelloSettings::setDefaultOperationScope(defaultOperationScope);
    }

    /**
     * Save instance into a QDomElement.
     * @param element A QDomElement representing xml element data.
     */
    void ClassState::saveToXMI1(QDomElement &element)
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

    void UIState::load()
    {
        useFillColor =  UmbrelloSettings::useFillColor();
        fillColor =  UmbrelloSettings::fillColor();
        lineColor =  UmbrelloSettings::lineColor();
        lineWidth =  UmbrelloSettings::lineWidth();
        textColor = UmbrelloSettings::textColor();
        font =  UmbrelloSettings::uiFont();
        backgroundColor = UmbrelloSettings::backgroundColor();
        gridDotColor = UmbrelloSettings::gridDotColor();
    }

    void UIState::save()
    {
        UmbrelloSettings::setUseFillColor(useFillColor);
        UmbrelloSettings::setFillColor(fillColor);
        UmbrelloSettings::setLineColor(lineColor);
        UmbrelloSettings::setLineWidth(lineWidth);
        UmbrelloSettings::setTextColor(textColor);
        UmbrelloSettings::setUiFont(font);
        UmbrelloSettings::setBackgroundColor(backgroundColor);
        UmbrelloSettings::setGridDotColor(gridDotColor);
    }

    /**
     * Save instance into a QDomElement.
     * @param element A QDomElement representing xml element data.
     */
    void UIState::saveToXMI1(QDomElement &element)
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
    void CodeImportState::load()
    {
        // code importer options
        createArtifacts = UmbrelloSettings::createArtifacts();
        resolveDependencies = UmbrelloSettings::resolveDependencies();
        supportCPP11 = UmbrelloSettings::supportCPP11();
    }

    void CodeImportState::save()
    {
        UmbrelloSettings::setCreateArtifacts(createArtifacts);
        UmbrelloSettings::setResolveDependencies(resolveDependencies);
        UmbrelloSettings::setSupportCPP11(supportCPP11);
    }

    /**
     * Save instance into a QDomElement.
     * @param element A QDomElement representing xml element data.
     */
    void CodeImportState::saveToXMI1(QDomElement &element)
    {
        element.setAttribute(QLatin1String("createartifacts"), createArtifacts);
        element.setAttribute(QLatin1String("resolvedependencies"), resolveDependencies);
        element.setAttribute(QLatin1String("supportcpp11"), supportCPP11);
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
        temp = element.attribute(QLatin1String("supportcpp11"), QLatin1String("0"));
        supportCPP11 = (bool)temp.toInt();

        return true;
    }

    void CodeGenerationState::load()
    {
        // CPP code generation options
        cppCodeGenerationState.autoGenAccessors = UmbrelloSettings::autoGenAccessors();

        cppCodeGenerationState.inlineAccessors = UmbrelloSettings::inlineAccessors();
        cppCodeGenerationState.publicAccessors = UmbrelloSettings::publicAccessors();
        cppCodeGenerationState.inlineOps = UmbrelloSettings::inlineOps();
        cppCodeGenerationState.virtualDestructors = UmbrelloSettings::virtualDestructors();
        cppCodeGenerationState.packageIsNamespace = UmbrelloSettings::packageIsNamespace();

        cppCodeGenerationState.stringClassName = UmbrelloSettings::stringClassName();
        cppCodeGenerationState.stringClassNameInclude = UmbrelloSettings::stringClassNameInclude();
        cppCodeGenerationState.stringIncludeIsGlobal = UmbrelloSettings::stringIncludeIsGlobal();

        cppCodeGenerationState.vectorClassName = UmbrelloSettings::vectorClassName();
        cppCodeGenerationState.vectorClassNameInclude = UmbrelloSettings::vectorClassNameInclude();
        cppCodeGenerationState.vectorIncludeIsGlobal = UmbrelloSettings::vectorIncludeIsGlobal();
        cppCodeGenerationState.docToolTag = UmbrelloSettings::docToolTag();

        // Java code generation options
        javaCodeGenerationState.autoGenerateAttributeAccessors = UmbrelloSettings::autoGenerateAttributeAccessorsJava();
        javaCodeGenerationState.autoGenerateAssocAccessors = UmbrelloSettings::autoGenerateAssocAccessorsJava();

        // D code generation options
        dCodeGenerationState.autoGenerateAttributeAccessors = UmbrelloSettings::autoGenerateAttributeAccessorsD();
        dCodeGenerationState.autoGenerateAssocAccessors = UmbrelloSettings::autoGenerateAssocAccessorsD();

        // Ruby code generation options
        rubyCodeGenerationState.autoGenerateAttributeAccessors = UmbrelloSettings::autoGenerateAttributeAccessorsRuby();
        rubyCodeGenerationState.autoGenerateAssocAccessors = UmbrelloSettings::autoGenerateAssocAccessorsRuby();
    }

    void CodeGenerationState::save()
    {
        // write config for CPP code generation options
        UmbrelloSettings::setAutoGenAccessors(cppCodeGenerationState.autoGenAccessors);

        UmbrelloSettings::setInlineAccessors(cppCodeGenerationState.inlineAccessors);
        UmbrelloSettings::setPublicAccessors(cppCodeGenerationState.publicAccessors);
        UmbrelloSettings::setInlineOps(cppCodeGenerationState.inlineOps);
        UmbrelloSettings::setVirtualDestructors(cppCodeGenerationState.virtualDestructors);
        UmbrelloSettings::setGetterWithGetPrefix(cppCodeGenerationState.getterWithGetPrefix);
        UmbrelloSettings::setRemovePrefixFromAccessorMethods(cppCodeGenerationState.removePrefixFromAccessorMethods);
        UmbrelloSettings::setAccessorMethodsStartWithUpperCase(cppCodeGenerationState.accessorMethodsStartWithUpperCase);
        UmbrelloSettings::setPackageIsNamespace(cppCodeGenerationState.packageIsNamespace);

        UmbrelloSettings::setStringClassName(cppCodeGenerationState.stringClassName);
        UmbrelloSettings::setStringClassNameInclude(cppCodeGenerationState.stringClassNameInclude);
        UmbrelloSettings::setStringIncludeIsGlobal(cppCodeGenerationState.stringIncludeIsGlobal);

        UmbrelloSettings::setVectorClassName(cppCodeGenerationState.vectorClassName);
        UmbrelloSettings::setVectorClassNameInclude(cppCodeGenerationState.vectorClassNameInclude);
        UmbrelloSettings::setVectorIncludeIsGlobal(cppCodeGenerationState.vectorIncludeIsGlobal);
        UmbrelloSettings::setDocToolTag(cppCodeGenerationState.docToolTag);

        // write config for Java code generation options
        UmbrelloSettings::setAutoGenerateAttributeAccessorsJava(javaCodeGenerationState.autoGenerateAttributeAccessors);
        UmbrelloSettings::setAutoGenerateAssocAccessorsJava(javaCodeGenerationState.autoGenerateAssocAccessors);

    //     CodeGenerator *codegen = getGenerator();
    //     JavaCodeGenerator *javacodegen = dynamic_cast<JavaCodeGenerator*>(codegen);
    //     if (javacodegen)
    //         UmbrelloSettings::setBuildANTDocumentJava(javacodegen->getCreateANTBuildFile());

        // write config for D code generation options
        UmbrelloSettings::setAutoGenerateAttributeAccessorsD(dCodeGenerationState.autoGenerateAttributeAccessors);
        UmbrelloSettings::setAutoGenerateAssocAccessorsD(dCodeGenerationState.autoGenerateAssocAccessors);

        // write config for Ruby code generation options
        UmbrelloSettings::setAutoGenerateAttributeAccessorsRuby(rubyCodeGenerationState.autoGenerateAttributeAccessors);
        UmbrelloSettings::setAutoGenerateAssocAccessorsRuby(rubyCodeGenerationState.autoGenerateAssocAccessors);
    }

    void AutoLayoutState::load()
    {
        autoDotPath =  UmbrelloSettings::autoDotPath();
        dotPath =  UmbrelloSettings::dotPath();
        showExportLayout =  UmbrelloSettings::showExportLayout();
    }

    void AutoLayoutState::save()
    {
        UmbrelloSettings::setAutoDotPath(autoDotPath);
        UmbrelloSettings::setDotPath(dotPath);
        UmbrelloSettings::setShowExportLayout(showExportLayout);
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

    void OptionState::load()
    {
        generalState.load();
        uiState.load();
        classState.load();
        codeViewerState.load();
        codeGenerationState.load();
        codeImportState.load();
        autoLayoutState.load();
    }

    void OptionState::save()
    {
        generalState.save();
        uiState.save();
        classState.save();
        codeViewerState.save();
        codeGenerationState.save();
        codeImportState.save();
        autoLayoutState.save();
    }

    /**
     * Save instance into a QDomElement.
     * @param element A QDomElement representing xml element data.
     */
    void OptionState::saveToXMI1(QDomElement& element)
    {
        uiState.saveToXMI1(element);
        classState.saveToXMI1(element);
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
