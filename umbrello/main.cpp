/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// app includes
#define DBG_SRC QStringLiteral("main")
#include "debug_utils.h"
#include "uml.h"
#include "version.h"
#include "umldoc.h"
#include "cmdlineexportallviewsevent.h"
#include "umlviewimageexportermodel.h"
#include "umbrellosettings.h"

// kde includes
#if QT_VERSION > 0x050000
# include <KAboutData>
# include <QApplication>
# include <QCommandLineParser>
# include <KConfig>
#else
# include <kaboutdata.h>
# include <kapplication.h>
# include <kcmdlineargs.h>
# include <kconfig.h>

  // KF5 uses i18n
# define i18n ki18n
#endif

#include <KLocalizedString>
#include <kcrash.h>
#include <ktip.h>
#include <KUrl>

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
#if QT_VERSION < 0x050000
bool showGUI(KCmdLineArgs *args)
{
    if (args->getOptionList("export").size() > 0 || args->isSet("export-formats")) {
        return false;
    }
    return true;
}
#else
bool showGUI(const QCommandLineParser *args)
{
    if (args->values("export").size() > 0 || args->isSet("export-formats")) {
        return false;
    }
    return true;
}
#endif

/**
 * Initializes the document used by the application.
 * If a file was specified in command line arguments, opens that file. Else, it
 * opens the last opened file, or a new file if there isn't any "last file used"
 * in the configuration.
 *
 * @param args The command line arguments given.
 * @param progLang The programming language to set if no existing file was opened.
 */
void initDocument(const QStringList& args, Uml::ProgrammingLanguage::Enum progLang)
{
    if (args.count()) {
        UMLApp::app()->openDocumentFile(KUrl(args.first()));
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
void exportAllViews(const QString &extension, QUrl directory, bool useFolders)
{
    // export to the specified directory, or the directory where the file is saved
    // if no directory was specified
    if (!directory.isValid()) {
        QFileInfo fi(UMLApp::app()->document()->url().toLocalFile());
        logInfo1("exportAllViews : No directory provided, using %1", fi.absolutePath());
        directory = QUrl::fromLocalFile(fi.absolutePath());
    }

    logDebug2("exportAllViews extension: %1, directory: %2", extension, directory.path());

    // the event is posted so when the Qt loop begins it's processed. UMLApp process this event executing
    // the method it provides for exporting the views. Once all the views were exported, a quit event
    // is sent and the app finishes without user interaction
    qApp->postEvent(UMLApp::app(), new CmdLineExportAllViewsEvent(extension, directory, useFolders));
}

int main(int argc, char *argv[])
{
#if QT_VERSION < 0x050000
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
    options.add("set-language <proglang>", i18n("set active language"));
    KCmdLineArgs::addCmdLineOptions(options); // Add our own options.
    KApplication app;
    KCmdLineArgs *parsedArgs = KCmdLineArgs::parsedArgs();
#else
    QApplication app(argc, argv);
    KCrash::initialize();
    KLocalizedString::setApplicationDomain("umbrello");
    Q_INIT_RESOURCE(ui);
    KAboutData aboutData(QStringLiteral("umbrello"),                   // componentName
                         i18n("Umbrello UML Modeller"),                // displayName
                         umbrelloVersion(),                            // version
                         i18n("Umbrello – Visual development environment for software, "
                              "based on the industry standard Unified Modelling Language (UML).<br/>"
                              "See also <a href=\"http://www.omg.org/spec/\">http://www.omg.org/spec/</a>."),
                         KAboutLicense::GPL,                           // licenseType
                         i18n("Copyright © 2001 Paul Hensgen,\nCopyright © 2002-2022 Umbrello UML Modeller Authors"),
                         QString(),                                    // otherText
                         QStringLiteral("http://umbrello.kde.org/"));  // homePageAddress
    aboutData.addAuthor(QStringLiteral("Paul Hensgen"), i18n("Author of initial version."), QStringLiteral("phensgen@users.sourceforge.net"));
    aboutData.addAuthor(i18n("Umbrello UML Modeller Authors"), QString(), QStringLiteral("umbrello-devel@kde.org"));

    aboutData.addCredit(QStringLiteral("Oliver Kellogg"),
                        i18n("Bug fixing, porting work, code cleanup, new features."),
                        QStringLiteral("okellogg@users.sourceforge.net"));
    aboutData.addCredit(QStringLiteral("Ralf Habacker"),
                        i18n("Bug fixing, porting work, code cleanup, new features."),
                        QStringLiteral("ralf.habacker@freenet.de"));
    aboutData.addCredit(QStringLiteral("Andi Fischer"),
                        i18n("Porting work, code cleanup, new features."),
                        QStringLiteral("andi.fischer@hispeed.ch"));
    aboutData.addCredit(QStringLiteral("Jonathan Riddell"),
                        i18n("Current maintainer."),
                        QStringLiteral("jr@jriddell.org"));
    aboutData.addCredit(QStringLiteral("Brian Thomas"),
                        i18n("A lot of work for C++ and Java code generation. Codeeditor."),
                        QStringLiteral("thomas@mail630.gsfc.nasa.gov"));

    KAboutData::setApplicationData(aboutData);
    QCommandLineParser parser;
    //PORTING SCRIPT: adapt aboutdata variable if necessary
    aboutData.setupCommandLine(&parser);

    parser.addPositionalArgument(QStringLiteral("file"), i18n("File to open"));
    QCommandLineOption exportDiagrams(QStringLiteral("export"), i18n("export diagrams to extension and exit"), QStringLiteral("extension"));
    parser.addOption(exportDiagrams);
    QCommandLineOption listExportFormats(QStringLiteral("export-formats"), i18n("list available export extensions"));
    parser.addOption(listExportFormats);
    QCommandLineOption dirForExport(QStringLiteral("directory"), i18n("the local directory to save the exported diagrams in"), QStringLiteral("url"));
    parser.addOption(dirForExport);
    QCommandLineOption importFiles(QStringLiteral("import-files"), i18n("import files"));
    parser.addOption(importFiles);
    QCommandLineOption listProgLangs(QStringLiteral("languages"), i18n("list supported languages"));
    parser.addOption(listProgLangs);
    QCommandLineOption useFolders(QStringLiteral("use-folders"), i18n("keep the tree structure used to store the views in the document in the target directory"));
    parser.addOption(useFolders);
    QCommandLineOption importDir(QStringLiteral("import-directory"), i18n("import files from directory <dir>"), QStringLiteral("dir"));
    parser.addOption(importDir);
    QCommandLineOption setProgLang(QStringLiteral("set-language"), i18n("set language"), QStringLiteral("proglang"));
    parser.addOption(setProgLang);

    parser.process(app);
    aboutData.processCommandLine(&parser);
#endif

    Q_INIT_RESOURCE(icons);
    app.setLayoutDirection(UmbrelloSettings::rightToLeftUI() ? Qt::RightToLeft : Qt::LeftToRight);
    QPointer<UMLApp> uml;
    if (app.isSessionRestored()) {
        kRestoreMainWindows< UMLApp >();
    } else {
#if QT_VERSION > 0x050000
        const QCommandLineParser *parsedArgs = &parser;
#endif
        if (parsedArgs->isSet("export-formats")) {
            foreach(const QString& type, UMLViewImageExporterModel::supportedImageTypes())
                fprintf(stdout, "%s\n", qPrintable(type));
            return 0;
        } else if (parsedArgs->isSet("languages")) {
            for (int i = 0; i <= Uml::ProgrammingLanguage::Reserved; i++) {
                Uml::ProgrammingLanguage::Enum pl = Uml::ProgrammingLanguage::fromInt(i);
                fprintf(stdout, "%s\n", qPrintable(Uml::ProgrammingLanguage::toString(pl)));
            }
            return 0;
        }

        uml = new UMLApp();
        uml->setup();
        app.processEvents();

        if (showGUI(parsedArgs)) {
            uml->show();
        }

        Uml::ProgrammingLanguage::Enum lang = Uml::ProgrammingLanguage::Reserved;
        if (parsedArgs->isSet("set-language")) {
            QString value;
#if QT_VERSION < 0x050000
            value = parsedArgs->getOption("set-language");
#else
            value = parsedArgs->value("set-language");
#endif
            // special cases: C++, C#
            if (value == QStringLiteral("C++")) {
                lang = Uml::ProgrammingLanguage::Cpp;
            } else if (value == QStringLiteral("C#")) {
                lang = Uml::ProgrammingLanguage::CSharp;
            } else {
                value = value.toLower();
                for(int i = 0; i < Uml::ProgrammingLanguage::Reserved; i++) {
                    Uml::ProgrammingLanguage::Enum pl = Uml::ProgrammingLanguage::fromInt(i);
                    QString langString = Uml::ProgrammingLanguage::toString(pl);
                    if (value == langString.toLower()) {
                        lang = Uml::ProgrammingLanguage::fromInt(i);
                    }
                }
            }
        }

        QStringList args;
#if QT_VERSION < 0x050000
        for (int i = 0; i < parsedArgs->count(); i++)
            args.append(parsedArgs->url(i).toLocalFile());
#else
        args = parsedArgs->positionalArguments();
#endif
        if (parsedArgs->isSet("import-files") && args.count() > 0) {
            uml->newDocument();
            if (lang != Uml::ProgrammingLanguage::Reserved)
                uml->setActiveLanguage(lang);
            uml->importFiles(args);
        }
        else if (parsedArgs->isSet("import-directory")) {
            uml->newDocument();
            if (lang != Uml::ProgrammingLanguage::Reserved)
                uml->setActiveLanguage(lang);
            QStringList filter = Uml::ProgrammingLanguage::toExtensions(uml->activeLanguage());
            QString dir;
#if QT_VERSION < 0x050000
            dir = parsedArgs->getOption("import-directory");
#else
            dir = parsedArgs->value("import-directory");
#endif
            QStringList listFile;
            getFiles(listFile, dir, filter);
            uml->importFiles(listFile, dir);
        }
        else {
            initDocument(args, lang);
        }

        // Handle diagram export related options
        if (parsedArgs->isSet("export")) {
            QString extension;
            QUrl directory;
#if QT_VERSION < 0x050000
            QStringList exportOpt = parsedArgs->getOptionList("export");
            if (exportOpt.size() > 0) {
                extension = exportOpt.last();
            }
            if (parsedArgs->isSet("directory")) {
                QStringList directoryOpt = parsedArgs->getOptionList("directory");
                if (directoryOpt.size() > 0) {
                    directory = KCmdLineArgs::makeURL(directoryOpt.last().toLocal8Bit());
                }
            }
#else
            extension = parsedArgs->value("export");
            if (parsedArgs->isSet("directory")) {
                QString dirValue = parsedArgs->value("directory");
                directory = QUrl::fromUserInput(dirValue, QDir::currentPath());
            }
#endif
            bool useFolders = parsedArgs->isSet("use-folders");
            exportAllViews(extension, directory, useFolders);
        }
    }
    int result = app.exec();
    return result;
}
