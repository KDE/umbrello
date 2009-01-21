/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "importprojectdlg.h"

// qt/kde includes
#include <QtCore/QFileInfo>
#include <QtCore/QRegExp>
#include <kmessagebox.h>

// local includes
#include "model_utils.h"

const QString ImportProjectDlg::ADA    = Model_Utils::progLangToString(Uml::pl_Ada);
const QString ImportProjectDlg::CPP    = Model_Utils::progLangToString(Uml::pl_Cpp);
const QString ImportProjectDlg::IDL    = Model_Utils::progLangToString(Uml::pl_IDL);
const QString ImportProjectDlg::JAVA   = Model_Utils::progLangToString(Uml::pl_Java);
const QString ImportProjectDlg::PASCAL = Model_Utils::progLangToString(Uml::pl_Pascal);
const QString ImportProjectDlg::PYTHON = Model_Utils::progLangToString(Uml::pl_Python);


ImportProjectDlg::ImportProjectDlg(QStringList* list, const  Uml::Programming_Language pl,
                                   QWidget* parent,const char* name,bool modal,Qt::WindowFlags fl)
  : KDialog(parent, fl), fileList(list)
{
    setObjectName(name);
    setModal(modal);
    setupUi(mainWidget());
    setInitialSize(QSize(330, 190));
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

ImportProjectDlg::~ImportProjectDlg()
{
}

/**
 * Recursively get all the sources files that matches the filters from the path aPath
 */
void ImportProjectDlg::getFiles(const QString& path, QStringList& filters)
{
    QDir searchDir(path);
    if (searchDir.exists()) {
        foreach (const QFileInfo &file, searchDir.entryList(filters, QDir::Files))
            fileList->append(searchDir.absoluteFilePath(file.fileName()));
        foreach (const QFileInfo &subDir, searchDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks))
            getFiles(searchDir.absoluteFilePath(subDir.fileName()), filters);
    }
}

/*
void ImportProjectDlg::languageChange()
{
    ImportProjectDlgBase::languageChange();
}
*/

/**
 * Inherited slot.
 * Once the directory and the language are choose, it looks recursively for the source files of this programming language
 * in this directory
 */
void ImportProjectDlg::accept()
{
    const KUrl url = m_kURL->url();
    if (url.isEmpty()) {
        KMessageBox::sorry(this, i18n("You have to choose a directory."));
        return;
    }
    QDir dir(url.path());
    if (!dir.exists()){
        KMessageBox::sorry(this, i18n("This directory does not exist."));
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
    KDialog::accept();
}


#include "importprojectdlg.moc"
