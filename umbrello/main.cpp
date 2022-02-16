/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// app includes
#define DBG_SRC QLatin1String("main")
#include "debug_utils.h"
#include "uml.h"
#include "version.h"
#include "umldoc.h"
#include "cmdlineexportallviewsevent.h"
#include "umlviewimageexportermodel.h"
#include "umbrellosettings.h"

// kde includes
#if QT_VERSION > 0x050000
// prevent including of <kaboutdata.h>
#define KABOUTDATA_H
#include <k4aboutdata.h>
#define KAboutData K4AboutData
#include <KCrash>
#else
#include <kaboutdata.h>
#endif

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <KLocalizedString>
#include <ktip.h>
#include <KUrl>

// KF5 uses i18n
#define i18n ki18n

#include <stdio.h>

DEBUG_REGISTER(main)

void getFiles(QStringList& files, const QString& path, QStringList& filters);

/**
 * Determines if the application GUI should be shown based on command line arguments.
 * @todo Add options to use the documentation generators from command line.
 *
 * @param args The command line arguments given.
 * @return True if the GUI should be shown, false otherwise.
 */
bool showGUI(KCmdLineArgs *args)
{
    if (args->getOptionList("export").size() > 0 || args->isSet("export-formats")) {
        return false;
    }
    return true;
}

/**
 * Initializes the document used by the application.
 * If a file was specified in command line arguments, opens that file. Else, it
 * opens the last opened file, or a new file if there isn't any "last file used"
 * in the configuration.
 *
 * @param args The command line arguments given.
 * @param progLang The programming language to set if no existing file was opened.
 */
void initDocument(KCmdLineArgs *args, Uml::ProgrammingLanguage::Enum progLang)
{
    if (args->count()) {
        UMLApp::app()->openDocumentFile(args->url(0));
    } else {
        bool last = UmbrelloSettings::loadlast();
        QString file = UmbrelloSettings::lastFile();
        if(last && !file.isEmpty()) {
            UMLApp::app()->openDocumentFile(KUrl(file));
        } else {
            UMLApp::app()->newDocument();
            if (progLang != Uml::ProgrammingLanguage::Reserved)
                UMLApp::app()->setActiveLanguage(progLang);
        }
    }
}

/**
 * Export all the views in the document using the command line args set by the user.
 * Errors that occurred while exporting, if any, are shown using uError().
 *
 * @param args The command line arguments given.
 * @param exportOpt A list containing all the "export" arguments given.
 */

void exportAllViews(KCmdLineArgs *args, const QStringList &exportOpt)
{
    QString extension(exportOpt.last());
    logDebug1("exportAllViews extension: %1", extension);

    // export to the specified directory, or the directory where the file is saved
    // if no directory was specified
    QUrl directory;
    QStringList directoryOpt = args->getOptionList("directory");
    if (directoryOpt.size() > 0) {
        directory = KCmdLineArgs::makeURL(directoryOpt.last().toLocal8Bit());
    } else {
        QFileInfo fi(UMLApp::app()->document()->url().toLocalFile());
        directory = QUrl::fromLocalFile(fi.absolutePath());
    }

    bool useFolders = args->isSet("use-folders");

    logDebug1("exportAllViews directory: %1", directory.path());

    // the event is posted so when the Qt loop begins it's processed. UMLApp process this event executing
    // the method it provides for exporting the views. Once all the views were exported, a quit event
    // is sent and the app finishes without user interaction
    kapp->postEvent(UMLApp::app(), new CmdLineExportAllViewsEvent(extension, directory, useFolders));
}

int main(int argc, char *argv[])
{
    KAboutData aboutData("umbrello",
                         0,
                         i18n("Umbrello UML Modeller"),
                         umbrelloVersion(),
                         i18n("Umbrello – Visual development environment for software, "
                              "based on the industry standard Unified Modelling Language (UML).<br/>"
                              "See also <a href=\"http://www.omg.org/spec/\">http://www.omg.org/spec/</a>."),
                         KAboutData::License_GPL,
                         i18n("Copyright © 2001 Paul Hensgen,\nCopyright © 2002-2022 Umbrello UML Modeller Authors"),
                         KLocalizedString(),
                         "http://umbrello.kde.org/");
    aboutData.addAuthor(i18n("Paul Hensgen"), i18n("Author of initial version."), "phensgen@users.sourceforge.net");
    aboutData.addAuthor(i18n("Umbrello UML Modeller Authors"), KLocalizedString(), "umbrello-devel@kde.org");

    // authors with more than 200 commits: git shortlog -seu | sort -g
    aboutData.addCredit(i18n("Oliver Kellogg"),
                        i18n("Bug fixing, porting work, code cleanup, new features."),
                        "okellogg@users.sourceforge.net");
    aboutData.addCredit(i18n("Ralf Habacker"),
                        i18n("Bug fixing, porting work, code cleanup, new features."),
                        "ralf.habacker@freenet.de");
    aboutData.addCredit(i18n("Andi Fischer"),
                        i18n("Porting work, code cleanup, new features."),
                        "andi.fischer@hispeed.ch");
    aboutData.addCredit(i18n("Jonathan Riddell"),
                        i18n("Current maintainer."),
                        "jr@jriddell.org");
    aboutData.addCredit(i18n("Brian Thomas"),
                        i18n("A lot of work for C++ and Java code generation. Codeeditor."),
                        "thomas@mail630.gsfc.nasa.gov");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions options;
    options.add("+[File]", i18n("File to open"));
    options.add("export <extension>", i18n("export diagrams to extension and exit"));
    options.add("export-formats", i18n("list available export extensions"));
    options.add("directory <url>", i18n("the local directory to save the exported diagrams in"), I18N_NOOP("the directory of the file"));
    options.add("import-files", i18n("import files"));
    options.add("languages", i18n("list supported languages"));
    options.add("use-folders", i18n("keep the tree structure used to store the views in the document in the target directory"));
    options.add("import-directory <dir>", i18n("import files from directory <dir>"));
    for (int i = 0; i <= Uml::ProgrammingLanguage::Reserved; i++) {
        Uml::ProgrammingLanguage::Enum pl = Uml::ProgrammingLanguage::fromInt(i);
        QByteArray optionString = "set-language-" + Uml::ProgrammingLanguage::toString(pl).toLower().toLocal8Bit();
        options.add(optionString, i18n("set active language"));
    }
    KCmdLineArgs::addCmdLineOptions(options); // Add our own options.
    KApplication app;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
#if QT_VERSION > 0x050000
    KCrash::initialize();
    KLocalizedString::setApplicationDomain("umbrello");
    Q_INIT_RESOURCE(ui);
#endif

    Q_INIT_RESOURCE(icons);
    app.setLayoutDirection(UmbrelloSettings::rightToLeftUI() ? Qt::RightToLeft : Qt::LeftToRight);
    QPointer<UMLApp> uml;
    if (app.isSessionRestored()) {
        kRestoreMainWindows< UMLApp >();
    } else {
        if (args->isSet("export-formats")) {
            foreach(const QString& type, UMLViewImageExporterModel::supportedImageTypes())
                fprintf(stdout, "%s\n", qPrintable(type));
            return 0;
        } else if (args->isSet("languages")) {
            for (int i = 0; i <= Uml::ProgrammingLanguage::Reserved; i++) {
                Uml::ProgrammingLanguage::Enum pl = Uml::ProgrammingLanguage::fromInt(i);
                fprintf(stdout, "%s\n", qPrintable(Uml::ProgrammingLanguage::toString(pl)));
            }
            return 0;
        }

        uml = new UMLApp();
        app.processEvents();

        if (showGUI(args)) {
            uml->show();
        }

        Uml::ProgrammingLanguage::Enum lang = Uml::ProgrammingLanguage::Reserved;
        for(int i = 0; i < Uml::ProgrammingLanguage::Reserved; i++) {
            Uml::ProgrammingLanguage::Enum pl = Uml::ProgrammingLanguage::fromInt(i);
            QByteArray langString = "set-language-";
            langString += Uml::ProgrammingLanguage::toString(pl).toLower().toLocal8Bit();
            if (args->isSet(langString)) {
                lang = Uml::ProgrammingLanguage::fromInt(i);
            }
        }

        if (args->isSet("import-files") && args->count() > 0) {
            QStringList importList;
            for (int i = 0; i < args->count(); i++)
                importList.append(args->url(i).toLocalFile());
            uml->newDocument();
            if (lang != Uml::ProgrammingLanguage::Reserved)
                uml->setActiveLanguage(lang);
            uml->importFiles(importList);
        }
        else if (args->isSet("import-directory")) {
            uml->newDocument();
            if (lang != Uml::ProgrammingLanguage::Reserved)
                uml->setActiveLanguage(lang);
            QStringList filter = Uml::ProgrammingLanguage::toExtensions(uml->activeLanguage());
            QString dir = args->getOption("import-directory");
            QStringList listFile;
            getFiles(listFile, dir, filter);
            uml->importFiles(listFile, dir);
        }
        else {
            initDocument(args, lang);
        }

        // export option
        QStringList exportOpt = args->getOptionList("export");
        if (exportOpt.size() > 0) {
             exportAllViews(args, exportOpt);
        }
    }
    int result = app.exec();
    delete uml;
    return result;
}
