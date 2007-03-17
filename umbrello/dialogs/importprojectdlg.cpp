/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QStringList>
#include <QFileInfo>
#include <QRegExp>
#include <kmessagebox.h>


// application specific includes
#include "importprojectdlg.h"


const QString ImportProjectDlg::ADA = "Ada";
const QString ImportProjectDlg::CPP = "C++";
const QString ImportProjectDlg::IDL = "Idl";
const QString ImportProjectDlg::JAVA = "Java";
const QString ImportProjectDlg::PASCAL = "Pascal"; 
const QString ImportProjectDlg::PYTHON = "Python";


ImportProjectDlg::ImportProjectDlg(QStringList* list, const  Uml::Programming_Language pl, 
                                   QWidget* parent,const char* name,bool modal,Qt::WindowFlags fl)
  : ImportProjectDlgBase(parent,name, modal,fl), fileList(list) {
    QStringList languages;

    languages += ImportProjectDlg::ADA;
    languages += ImportProjectDlg::CPP;
    languages += ImportProjectDlg::IDL;
    languages += ImportProjectDlg::JAVA;
    languages += ImportProjectDlg::PASCAL;
    languages += ImportProjectDlg::PYTHON;

    m_language->addItems(languages);

    QString selectedLanguage = ImportProjectDlg::CPP;
    if (pl == Uml::pl_IDL) {
        selectedLanguage = ImportProjectDlg::IDL;
    } else if (pl == Uml::pl_Python) {
        selectedLanguage = ImportProjectDlg::PYTHON;
    } else if (pl == Uml::pl_Java) {
        selectedLanguage = ImportProjectDlg::JAVA;
    } else if (pl == Uml::pl_Pascal) {
        selectedLanguage = ImportProjectDlg::PASCAL;
    } else if (pl == Uml::pl_Ada) {
        selectedLanguage = ImportProjectDlg::ADA;
    }

    QRegExp languageRegExp(selectedLanguage,Qt::CaseSensitive, QRegExp::FixedString);
    m_language->setCurrentIndex(languages.indexOf(languageRegExp));
    m_language->setEditable(false);
    m_kURL->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
}

QDialog::DialogCode ImportProjectDlg::getFilesToImport(QStringList* list, const  Uml::Programming_Language pl, 
                                     QWidget* parent) {

    ImportProjectDlg dlg(list, pl, parent);
    QDialog::DialogCode code = (QDialog::DialogCode) dlg.exec();
    return code;
}

void ImportProjectDlg::getFiles(const QString& path, QStringList& filters) {
    QDir searchDir(path);
    if (searchDir.exists()) {
        foreach (QFileInfo file, searchDir.entryList(filters, QDir::Files))
            fileList->append(searchDir.absoluteFilePath(file.fileName()));
        foreach (QFileInfo subDir, searchDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks))
            getFiles(searchDir.absoluteFilePath(subDir.fileName()), filters);
    }
}


/*void ImportProjectDlg::languageChange() {
    ImportProjectDlgBase::languageChange();
}
*/
void ImportProjectDlg::accept() {

    const KUrl url = m_kURL->url();
    if (url.isEmpty()) {
        KMessageBox::sorry(this, i18n("You have to choose a directory."));
        return;
    } 
    QDir dir(url.path());
    if (!dir.exists()){
        KMessageBox::sorry(this, i18n("This directory doesn't exist."));
        return;
    }
    QStringList filter;
    QString language = m_language->currentText();
    if (language == ImportProjectDlg::ADA) {
        filter += "*.ads";
        filter += "*.ada";
    } else if (language == ImportProjectDlg::IDL) {
        filter += "*.idl";
    } else if (language == ImportProjectDlg::JAVA) {
        filter += "*.java";
    } if (language == ImportProjectDlg::PASCAL) {
        filter += "*.pas";
    } if (language == ImportProjectDlg::PYTHON) {
        filter += "*.py";
    } else {
        //C++
        filter += "*.h";
        filter += "*.hh";
        filter += "*.hpp";
        filter += "*.hxx";
        filter += "*.H";
    }
    fileList->clear();
    getFiles(url.path(), filter);
    if (fileList->empty()) {
        KMessageBox::sorry(this, i18n("No source file in this directory."));
        return;
    }
    QDialog::accept();
}


#include "importprojectdlg.moc"
