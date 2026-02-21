/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "optionstate.h"
#include "umbrellosettings.h"

namespace Settings {

    void GeneralState::load()
    {
        undo = UmbrelloSettings::undo();
        tabdiagrams = UmbrelloSettings::tabdiagrams();
    #ifdef ENABLE_NEW_CODE_GENERATORS
        newcodegen = UmbrelloSettings::newcodegen();
    #endif
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
        showDebugWindows =  UmbrelloSettings::showDebugWindows();
    }

    void GeneralState::save()
    {
        UmbrelloSettings::setUndo(undo);
        UmbrelloSettings::setTabdiagrams(tabdiagrams);
        UmbrelloSettings::setNewcodegen(newcodegen);
        UmbrelloSettings::setFooterPrinting(footerPrinting);
        UmbrelloSettings::setAutosave(autosave);
        UmbrelloSettings::setTime(time);
        UmbrelloSettings::setAutosavetime(autosavetime);
        UmbrelloSettings::setAutosavesuffix(autosavesuffix);
        UmbrelloSettings::setLoadlast(loadlast);
        UmbrelloSettings::setUml2(uml2);
        UmbrelloSettings::setDiagram(diagram);
        UmbrelloSettings::setDefaultLanguage(defaultLanguage);
        UmbrelloSettings::setShowDebugWindows(showDebugWindows);
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
     * Save instance to XMI.
     * @param writer The QXmlStreamWriter to save to.
     */
    void ClassState::saveToXMI(QXmlStreamWriter& writer)
    {
        writer.writeAttribute(QStringLiteral("showattribassocs"), QString::number(showAttribAssocs));
        writer.writeAttribute(QStringLiteral("showatts"),         QString::number(showAtts));
        writer.writeAttribute(QStringLiteral("showattsig"),       QString::number(showAttSig));
        writer.writeAttribute(QStringLiteral("showops"),          QString::number(showOps));
        writer.writeAttribute(QStringLiteral("showopsig"),        QString::number(showOpSig));
        writer.writeAttribute(QStringLiteral("showpackage"),      QString::number(showPackage));
        writer.writeAttribute(QStringLiteral("showpubliconly"),   QString::number(showPublicOnly));
        writer.writeAttribute(QStringLiteral("showscope"),        QString::number(showVisibility));
#ifdef ENABLE_WIDGET_SHOW_DOC
        writer.writeAttribute(QStringLiteral("showdocumentation"),QString::number(showDocumentation));
#endif
        writer.writeAttribute(QStringLiteral("showstereotype"),   QString::number(showStereoType));
    }

    /**
     * Load instance from a QDomElement.
     * @param element A QDomElement representing xml element data.
     * @return true on success
     * @return false on error
     */
    bool ClassState::loadFromXMI(QDomElement &element)
    {
        QString temp = element.attribute(QStringLiteral("showattribassocs"), QStringLiteral("0"));
        showAttribAssocs = (bool)temp.toInt();
        temp = element.attribute(QStringLiteral("showatts"), QStringLiteral("0"));
        showAtts = (bool)temp.toInt();
        temp = element.attribute(QStringLiteral("showattsig"), QStringLiteral("0"));
        showAttSig = (bool)temp.toInt();
        temp = element.attribute(QStringLiteral("showops"), QStringLiteral("0"));
        showOps = (bool)temp.toInt();
        temp = element.attribute(QStringLiteral("showopsig"), QStringLiteral("0"));
        showOpSig = (bool)temp.toInt();
        temp = element.attribute(QStringLiteral("showpackage"), QStringLiteral("0"));
        showPackage = (bool)temp.toInt();
#ifdef ENABLE_WIDGET_SHOW_DOC
        temp = element.attribute(QStringLiteral("showdocumentation"), QStringLiteral("0"));
        showDocumentation = (bool)temp.toInt();
#endif
        temp = element.attribute(QStringLiteral("showpubliconly"), QStringLiteral("0"));
        showPublicOnly = (bool)temp.toInt();
        temp = element.attribute(QStringLiteral("showscope"), QStringLiteral("0"));
        showVisibility = (bool)temp.toInt();
        temp = element.attribute(QStringLiteral("showstereotype"), QStringLiteral("0"));
        showStereoType = (Uml::ShowStereoType::Enum)temp.toInt();
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
        useBackgroundColor = UmbrelloSettings::useBackgroundColor();
        gridDotColor = UmbrelloSettings::gridDotColor();
        useAlignmentGuides = UmbrelloSettings::useAlignmentGuides();
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
        UmbrelloSettings::setUseBackgroundColor(useBackgroundColor);
        UmbrelloSettings::setGridDotColor(gridDotColor);
        UmbrelloSettings::setUseAlignmentGuides(useAlignmentGuides);
    }

    /**
     * Save instance to XMI.
     * @param writer The QXmlStreamWriter to save to.
     */
    void UIState::saveToXMI(QXmlStreamWriter& writer)
    {
        writer.writeAttribute(QStringLiteral("backgroundcolor"),  backgroundColor.name());
        writer.writeAttribute(QStringLiteral("usebackgroundcolor"),  QString::number(useBackgroundColor));
        writer.writeAttribute(QStringLiteral("fillcolor"),        fillColor.name());
        writer.writeAttribute(QStringLiteral("font"),             font.toString());
        writer.writeAttribute(QStringLiteral("griddotcolor"),     gridDotColor.name());
        writer.writeAttribute(QStringLiteral("linecolor"),        lineColor.name());
        writer.writeAttribute(QStringLiteral("linewidth"),        QString::number(lineWidth));
        writer.writeAttribute(QStringLiteral("textcolor"),        textColor.name());
        writer.writeAttribute(QStringLiteral("usefillcolor"),     QString::number(useFillColor));
        writer.writeAttribute(QStringLiteral("usealignmentguides"), QString::number(useAlignmentGuides));
    }

    /**
     * Load instance from a QDomElement.
     * @param element A QDomElement representing xml element data.
     * @return true on success
     * @return false on error
     */
    bool UIState::loadFromXMI(QDomElement &element)
    {
        QString backgroundColor = element.attribute(QStringLiteral("backgroundcolor"));
        if (!backgroundColor.isEmpty())
            this->backgroundColor = QColor(backgroundColor);
        QString fillcolor = element.attribute(QStringLiteral("fillcolor"));
        if (!fillcolor.isEmpty())
            this->fillColor = QColor(fillcolor);
        QString font = element.attribute(QStringLiteral("font"));
        if (!font.isEmpty()) {
            this->font.fromString(font);
            this->font.setUnderline(false);
        }
        QString gridDotColor = element.attribute(QStringLiteral("griddotcolor"));
        if (!gridDotColor.isEmpty())
            this->gridDotColor = QColor(gridDotColor);
        QString linecolor = element.attribute(QStringLiteral("linecolor"));
        if (!linecolor.isEmpty())
            this->lineColor = QColor(linecolor);
        QString linewidth = element.attribute(QStringLiteral("linewidth"));
        if (!linewidth.isEmpty())
            this->lineWidth = linewidth.toInt();
        QString textColor = element.attribute(QStringLiteral("textcolor"));
        if (!textColor.isEmpty())
            this->textColor = QColor(textColor);

        QString usebackgroundcolor = element.attribute(QStringLiteral("usebackgroundcolor"), QStringLiteral("0"));
        this->useBackgroundColor = (bool)usebackgroundcolor.toInt();

        QString usefillcolor = element.attribute(QStringLiteral("usefillcolor"), QStringLiteral("0"));
        this->useFillColor = (bool)usefillcolor.toInt();

        QString usealignmentguides = element.attribute(QStringLiteral("usealignmentguides"), QStringLiteral("1"));
        this->useAlignmentGuides = (bool)usealignmentguides.toInt();

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
     * Save instance to XMI.
     * @param writer The QXmlStreamWriter to save to.
     */
    void CodeImportState::saveToXMI(QXmlStreamWriter& writer)
    {
        writer.writeAttribute(QStringLiteral("createartifacts"), QString::number(createArtifacts));
        writer.writeAttribute(QStringLiteral("resolvedependencies"), QString::number(resolveDependencies));
        writer.writeAttribute(QStringLiteral("supportcpp11"), QString::number(supportCPP11));
    }

    /**
     * Load instance from a QDomElement.
     * @param element A QDomElement representing xml element data.
     * @return true on success
     * @return false on error
     */
    bool CodeImportState::loadFromXMI(QDomElement &element)
    {
        QString temp = element.attribute(QStringLiteral("createartifacts"), QStringLiteral("0"));
        createArtifacts = (bool)temp.toInt();
        temp = element.attribute(QStringLiteral("resolvedependencies"), QStringLiteral("0"));
        resolveDependencies = (bool)temp.toInt();
        temp = element.attribute(QStringLiteral("supportcpp11"), QStringLiteral("0"));
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
        UmbrelloSettings::setClassMemberPrefix(cppCodeGenerationState.classMemberPrefix);

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
     * Save instance to a QXmlStreamWriter.
     * @param writer The QXmlStreamWriter to save to.
     */
    void OptionState::saveToXMI(QXmlStreamWriter& writer)
    {
        uiState.saveToXMI(writer);
        classState.saveToXMI(writer);
    }

    /**
     * Load instance from a QDomElement.
     * @param element A QDomElement representing xml element data.
     * @return true on success
     * @return false on error
     */
    bool OptionState::loadFromXMI(QDomElement& element)
    {
        uiState.loadFromXMI(element);
        classState.loadFromXMI(element);

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
