/***************************************************************************
                          codegenerationwizard.h  -  description
                             -------------------
    begin                : Wed Jul 24 2002
    copyright            : (C) 2002 by Luis De la Parra
    email                : luis@delaparra.org
  Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CODEGENERATIONWIZARD_H
#define CODEGENERATIONWIZARD_H

#include <qwidget.h>
#include <qptrlist.h>
#include "codegenerationwizardbase.h"
#include "settingsdlg.h"
#include "../umlclassifierlist.h"

class UMLApp;
class UMLDoc;
class CodeGenerator;
class CodeGenerationOptionsPage;

/**
 * @author Luis De la Parra
 * based on wizard from Paul Hensgen
 */

class CodeGenerationWizard : public CodeGenerationWizardBase {
	Q_OBJECT
public:
	CodeGenerationWizard(UMLDoc *doc, UMLClassifierList *classList,
	                     QDict<GeneratorInfo> ldict, 
			     QString activeLanguage, UMLApp *parent,
			     const char *name=0);
	~CodeGenerationWizard();

	void showPage(QWidget *);

	int exec() {
		return QWizard::exec();
	}
protected slots:
	void selectClass();
	void deselectClass();
	void populateStatusList();
	void generateCode();
	void classGenerated(UMLClassifier* concept, bool generated);

private:
	CodeGenerator* generator();

	UMLApp* m_app;
	UMLDoc* m_doc;
	CodeGenerationOptionsPage* m_CodeGenerationOptionsPage;
	QDict<GeneratorInfo> m_ldict;
};

#endif
