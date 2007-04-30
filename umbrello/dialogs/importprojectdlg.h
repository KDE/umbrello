/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef IMPORTPROJECTDLG_H
#define IMPORTPROJECTDLG_H


#include <QStringList>
// application specific includes
#include "importprojectdlgbase.h"
#include "../umlnamespace.h"



/**
 * Dialog for collecting the "Import project" params and searching the files that match the params.
 * Inherits ImportProjectDlgBase
 * The user chooses the directory with a KUrlRequester widget and chooses the language in a combobox.
 * The slot accept() do the search
 */
class ImportProjectDlg : public ImportProjectDlgBase {
  Q_OBJECT

public:

    /**
     * Constructor for ImportProjectDlg.
     *
     * @param list The list of source files.
     * @param pl The preselected programming language
     * @param parent The parent of the dialog.
     * @param name The internal name.
     * @param modal If modal is true the dialog will block input to other the windows
     *              in the application until it's closed.
     * @param fl Window flags.
     * @param defaultFileExtension The default file extension.
     *
     * @see QDialog::QDialog
     */
    ImportProjectDlg(QStringList* list, const  Uml::Programming_Language pl,
                     QWidget* parent = 0, const char* name = 0,bool modal = false, Qt::WindowFlags fl = 0);

    /**
     * Destructor for ImportProjectDlg.
     */
    virtual ~ImportProjectDlg() {
    }

    /**
     * Static convenience method that creates a ImportProjectDlg and fills the first param
     * with all the source files in the directory chosen by the user.
     *
     * @param list The list of source files.
     * @param pl the programming language for which we look for source files
     * @param parent The parent of the dialog.
     *
     * @return Which button (OK or Cancel) has been clicked
     */
    static QDialog::DialogCode getFilesToImport(QStringList* list, const  Uml::Programming_Language pl, QWidget* parent);

protected slots:

    /**
    * Inherited slot.
    * Once the directory and the language are choose, it looks recursively for the source files of this programming language
    * in this directory
    */
    virtual void accept();

private:

    /**
    * Recursively get all the sources files that matches the filters from the path aPath
    */
    virtual void getFiles(const QString& aPath, QStringList& filters);

    QStringList* fileList;

    static const QString ADA;
    static const QString CPP;
    static const QString IDL;
    static const QString JAVA;
    static const QString PASCAL;
    static const QString PYTHON;
};

#endif
