/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// app includes
#include "debug_utils.h"
#include "uml.h"
#include "version.h"
#include "umldoc.h"
#include "cmdlineexportallviewsevent.h"
#include "umlviewimageexportermodel.h"
#include "umbrellosettings.h"

// kde includes
#include <kaboutdata.h>
#if QT_VERSION < 0x050000
#include <kapplication.h>
#include <kcmdlineargs.h>
#endif
#include <kconfig.h>
#include <KLocalizedString>
#include <ktip.h>

#if QT_VERSION >= 0x050000
#include <QApplication>
#include <QCommandLineParser>
#endif

#include <stdio.h>

#if QT_VERSION >= 0x050000
#define StringLiteral(a) QStringLiteral(a)
#define ki18n(a) i18n(a)
#define KLocalizedString(a) QString(a)
#else
#define StringLiteral(a) QByteArray(a)
#endif

/**
 * Description for this application
 */
static const char description[] =
    I18N_NOOP("Umbrello UML Modeller");

/**
 * Determines if the application GUI should be shown based on command line arguments.
 * @todo Add options to use the documentation generators from command line.
 *
 * @param args The command line arguments given.
 * @return True if the GUI should be shown, false otherwise.
 */
#if QT_VERSION >= 0x050000
bool showGUI(QCommandLineParser *parser);
#else
bool showGUI(KCmdLineArgs *args);
#endif

/**
 * Initializes the document used by the application.
 * If a file was specified in command line arguments, opens that file. Else, it
 * opens the last opened file, or a new file if there isn't any "last file used"
 * in the configuration.
 *
 * @param args The command line arguments given.
 */
#if QT_VERSION >= 0x050000
void initDocument(QCommandLineParser *parser);
#else
void initDocument(KCmdLineArgs *args);
#endif

/**
 * Export all the views in the document using the command line args set by the user.
 * Errors that occurred while exporting, if any, are shown using uError().
 *
 * @param args The command line arguments given.
 * @param exportOpt A list containing all the "export" arguments given.
 */
#if QT_VERSION >= 0x050000
void exportAllViews(QCommandLineParser *parser, const QStringList &exportOpt);
#else
void exportAllViews(KCmdLineArgs *args, const QStringList &exportOpt);
#endif

#if QT_VERSION >= 0x050000
static const QString URL            = QStringLiteral("url");
static const QString EXPORT         = QStringLiteral("export");
static const QString EXPORT_FORMATS = QStringLiteral("export-formats");
static const QString IMPORT_FILES   = QStringLiteral("import-files");
static const QString USE_FOLDERS    = QStringLiteral("use-folders");
static const QString DIRECTORY      = QStringLiteral("directory");
static const QString LANGUAGES      = QStringLiteral("languages");
#endif

int main(int argc, char *argv[])
{
#if QT_VERSION >= 0x050000
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("umbrello");

    KAboutData aboutData(QStringLiteral("umbrello"),
                         i18n("Umbrello UML Modeller"),
                         QLatin1String(umbrelloVersion()),
                         i18n("Umbrello – Visual development environment for software, "
                              "based on the industry standard Unified Modelling Language (UML).<br/>"
                              "See also <a href=’http://www.omg.org/spec/’>http://www.omg.org/spec/</a>."),
                         KAboutLicense::GPL,
                         i18n("Copyright © 2001 Paul Hensgen,\nCopyright © 2002-2016 Umbrello UML Modeller Authors"),
                         QString(),
                         QStringLiteral("http://umbrello.kde.org"));
#else
    KAboutData aboutData("umbrello", 0, ki18n("Umbrello UML Modeller"),
                          umbrelloVersion(), ki18n(description), KAboutData::License_GPL,
                          ki18n("(c) 2001 Paul Hensgen, (c) 2002-2016 Umbrello UML Modeller Authors"), KLocalizedString(),
                          "http://umbrello.kde.org/");
#endif
    aboutData.addAuthor(ki18n("Paul Hensgen"), ki18n("Author of initial version."), StringLiteral("phensgen@users.sourceforge.net"));
    aboutData.addAuthor(ki18n("Umbrello UML Modeller Authors"), KLocalizedString(), StringLiteral("umbrello-devel@kde.org"));

    // authors with more than 200 commits: git shortlog -seu | sort -g
    aboutData.addCredit(ki18n("Oliver Kellogg"),
                        ki18n("Bug fixing, porting work, code cleanup, new features."),
                        StringLiteral("okellogg@users.sourceforge.net"));
    aboutData.addCredit(ki18n("Ralf Habacker"),
                        ki18n("Bug fixing, porting work, code cleanup, new features."),
                        StringLiteral("ralf.habacker@freenet.de"));
    aboutData.addCredit(ki18n("Andi Fischer"),
                        ki18n("Porting work, code cleanup, new features."),
                        StringLiteral("andi.fischer@hispeed.ch"));
    aboutData.addCredit(ki18n("Jonathan Riddell"),
                        ki18n("Current maintainer."),
                        StringLiteral("jr@jriddell.org"));
    aboutData.addCredit(ki18n("Brian Thomas"),
                        ki18n("A lot of work for C++ and Java code generation. Codeeditor."),
                        StringLiteral("thomas@mail630.gsfc.nasa.gov"));

#if QT_VERSION >= 0x050000
    KAboutData::setApplicationData(aboutData);
    QCommandLineParser* args = new QCommandLineParser;
    args->addHelpOption();
    args->addVersionOption();
    args->addPositionalArgument(
                URL, i18n("File to open with path."), URL);
    args->addOption(
                QCommandLineOption(EXPORT, i18n("Export diagrams to extension and exit.")));
    args->addOption(
                QCommandLineOption(EXPORT_FORMATS, i18n("List available export extensions.")));
    args->addOption(
                QCommandLineOption(DIRECTORY, i18n("The local directory to save the exported diagrams in.")));
    args->addOption(
                QCommandLineOption(IMPORT_FILES, i18n("Import files.")));
    args->addOption(
                QCommandLineOption(USE_FOLDERS, i18n("Keep the tree structure used to store the views in the document in the target directory.")));
    aboutData.setupCommandLine(args);

    args->process(app);

    aboutData.processCommandLine(args);
    Q_INIT_RESOURCE(icons);
#else
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions options;
    options.add("+[File]", ki18n("File to open"));
    options.add("export <extension>", ki18n("export diagrams to extension and exit"));
    options.add("export-formats", ki18n("list available export extensions"));
    options.add("directory <url>", ki18n("the local directory to save the exported diagrams in"), I18N_NOOP("the directory of the file"));
    options.add("import-files", ki18n("import files"));
    options.add("languages", ki18n("list supported languages"));
    options.add("use-folders", ki18n("keep the tree structure used to store the views in the document in the target directory"));
    KCmdLineArgs::addCmdLineOptions(options); // Add our own options.
    KApplication app;
    Q_INIT_RESOURCE_EXTERN(icons);
#endif

    QPointer<UMLApp> uml;
    if (app.isSessionRestored()) {
        kRestoreMainWindows< UMLApp >();
    } else {
#if QT_VERSION >= 0x050000
        if (args->isSet(EXPORT_FORMATS)) {
#else
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        if (args->isSet("export-formats")) {
#endif
            foreach(const QString& type, UMLViewImageExporterModel::supportedImageTypes())
                fprintf(stdout, "%s\n", qPrintable(type));
            return 0;
#if QT_VERSION >= 0x050000
        } else if (args->isSet(LANGUAGES)) {
#else
        } else if (args->isSet("languages")) {
#endif
            for(int i = Uml::ProgrammingLanguage::ActionScript; i < Uml::ProgrammingLanguage::Reserved; i++) {
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

#if QT_VERSION >= 0x050000
        if (args->isSet(IMPORT_FILES)) {
            QStringList importList = args->values(IMPORT_FILES);
            if (importList.size() > 0) {
                uml->newDocument();
                uml->importFiles(&importList);
            }
        }
#else
        if (args->isSet("import-files") && args->count() > 0) {
            QStringList importList;
            for (int i = 0; i < args->count(); i++)
                importList.append(args->url(i).toLocalFile());
            uml->newDocument();
            uml->importFiles(&importList);
        }
#endif
        else
            initDocument(args);


        // export option
#if QT_VERSION >= 0x050000
        QStringList exportOpt = args->values(EXPORT);
#else
        QStringList exportOpt = args->getOptionList("export");
#endif
        if (exportOpt.size() > 0) {
             exportAllViews(args, exportOpt);
        }
    }
    int result = app.exec();
    delete uml;
    return result;
}

#if QT_VERSION >= 0x050000
bool showGUI(QCommandLineParser *parser)
{
    if (parser->isSet(EXPORT) || parser->isSet(EXPORT_FORMATS)) {
        return false;
    }
    return true;
}

void initDocument(QCommandLineParser *parser)
{
    QStringList urls = parser->positionalArguments();
    if (urls.count() > 0) {
        UMLApp::app()->openDocumentFile(QUrl::fromLocalFile(urls.at(0)));
    } else {
        bool last = UmbrelloSettings::loadlast();
        QString file = UmbrelloSettings::lastFile();
        if (last && !file.isEmpty()) {
            UMLApp::app()->openDocumentFile(QUrl(file));
        } else {
            UMLApp::app()->newDocument();
        }
    }
}

void exportAllViews(QCommandLineParser *parser, const QStringList &exportOpt)
{
    QString extension(exportOpt.last());
    uDebug() << "extension: " << extension;

    // export to the specified directory, or the directory where the file is saved
    // if no directory was specified
    QUrl directory;
    QString directoryOpt = parser->value(DIRECTORY);
    if (directoryOpt.size() > 0) {
        directory = QUrl(directoryOpt);
    } else {
        directory = UMLApp::app()->document()->url().adjusted(QUrl::RemoveFilename);
    }

    bool useFolders = parser->isSet(USE_FOLDERS);

    uDebug() << "directory: " << directory.toDisplayString();

    // the event is posted so when the Qt loop begins it's processed. UMLApp process this event executing
    // the method it provides for exporting the views. Once all the views were exported, a quit event
    // is sent and the app finishes without user interaction
    qApp->postEvent(UMLApp::app(), new CmdLineExportAllViewsEvent(extension, directory, useFolders));
}
#else
bool showGUI(KCmdLineArgs *args)
{
    if (args->getOptionList("export").size() > 0 || args->isSet("export-formats")) {
        return false;
    }
    return true;
}

void initDocument(KCmdLineArgs *args)
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
        }
    }
}

void exportAllViews(KCmdLineArgs *args, const QStringList &exportOpt)
{
    QString extension(exportOpt.last());
    uDebug() << "extension: " << extension;

    // export to the specified directory, or the directory where the file is saved
    // if no directory was specified
    KUrl directory;
    QStringList directoryOpt = args->getOptionList("directory");
    if (directoryOpt.size() > 0) {
        directory = KCmdLineArgs::makeURL(directoryOpt.last().toLocal8Bit());
    } else {
        directory = KUrl(UMLApp::app()->document()->url().directory());
    }

    bool useFolders = args->isSet("use-folders");

    uDebug() << "directory: " << directory.prettyUrl();

    // the event is posted so when the Qt loop begins it's processed. UMLApp process this event executing
    // the method it provides for exporting the views. Once all the views were exported, a quit event
    // is sent and the app finishes without user interaction
    kapp->postEvent(UMLApp::app(), new CmdLineExportAllViewsEvent(extension, directory, useFolders));
}
#endif
