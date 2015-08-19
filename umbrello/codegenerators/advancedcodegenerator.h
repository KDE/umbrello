/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2015                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

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

public slots:
    virtual void checkAddUMLObject(UMLObject * obj);
    virtual void checkRemoveUMLObject(UMLObject * obj);

protected:
    virtual void initFromParentDocument();
    void connect_newcodegen_slots();
};

#endif // ADVANCEDCODEGENERATOR_H
