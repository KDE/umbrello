/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef IMPORTPROJECTDLG_H
#define IMPORTPROJECTDLG_H

// application specific includes
#include "ui_importprojectdlgbase.h"
#include "basictypes.h"

/**
 * Dialog for collecting the "Import project" params and searching the files that match the params.
 * Inherits ImportProjectDlgBase
 * The user chooses the directory with a KUrlRequester widget and chooses the language in a combobox.
 * The slot accept() do the search
 */
class ImportProjectDlg : public KDialog, private Ui::ImportProjectDlgBase
{
  Q_OBJECT
public:
    ImportProjectDlg(QStringList* list, const  Uml::ProgrammingLanguage pl,
                     QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    virtual ~ImportProjectDlg();

protected slots:
    virtual void accept();

private:
    virtual void getFiles(const QString& aPath, QStringList& filters);

    QStringList* m_fileList;

    static const QString ADA;
    static const QString CPP;
    static const QString IDL;
    static const QString JAVA;
    static const QString PASCAL;
    static const QString PYTHON;
};

#endif
