/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ADVANCEDCODEGENERATOR_H
#define ADVANCEDCODEGENERATOR_H

#include "codegenerator.h"

/**
 * AdvancedCodeGenerator is the base class for advanced cocde generators like
 * CPPCodeGenerator, JavaCodeGenerator and so on.
 */
class AdvancedCodeGenerator : public CodeGenerator
{
    Q_OBJECT
public:
    AdvancedCodeGenerator();
    virtual ~AdvancedCodeGenerator();

    virtual CodeDocument * newClassifierCodeDocument(UMLClassifier * classifier) = 0;
    virtual CodeViewerDialog * getCodeViewerDialog(QWidget* parent, CodeDocument * doc,
            Settings::CodeViewerState & state);

public Q_SLOTS:
    virtual void checkAddUMLObject(UMLObject * obj);
    virtual void checkRemoveUMLObject(UMLObject * obj);

protected:
    virtual void initFromParentDocument();
    void connectSlots();
};

#endif // ADVANCEDCODEGENERATOR_H
