/*
    Copyright 2010  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "codeimpselectpage.h"

// app includes
#include "basictypes.h"
#include "debug_utils.h"
#include "model_utils.h"
#include "uml.h"

#include "treemodel.h"

// kde includes
#include <knuminput.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klocale.h>

// qt includes
//#include <QtGui/QTreeView>

const QString CodeImpSelectPage::ADA    = Uml::ProgrammingLanguage::toString(Uml::ProgrammingLanguage::Ada);
const QString CodeImpSelectPage::CPP    = Uml::ProgrammingLanguage::toString(Uml::ProgrammingLanguage::Cpp);
const QString CodeImpSelectPage::IDL    = Uml::ProgrammingLanguage::toString(Uml::ProgrammingLanguage::IDL);
const QString CodeImpSelectPage::JAVA   = Uml::ProgrammingLanguage::toString(Uml::ProgrammingLanguage::Java);
const QString CodeImpSelectPage::PASCAL = Uml::ProgrammingLanguage::toString(Uml::ProgrammingLanguage::Pascal);
const QString CodeImpSelectPage::PYTHON = Uml::ProgrammingLanguage::toString(Uml::ProgrammingLanguage::Python);

/**
 * Constructor.
 * @param parent   the parent (wizard) of this wizard page
 */
CodeImpSelectPage::CodeImpSelectPage(QWidget *parent)
  : QWizardPage(parent),
    m_fileList(),
    m_fileExtensionPattern(QRegExp("\\.h$"))
{
    setTitle(i18n("Code Importing Path"));
    setSubTitle(i18n("Select the code importing path."));

    setupUi(this);

    setupLanguageBox();
    connect(ui_languageBox, SIGNAL(activated(int)), this, SLOT(languageChanged(int)));
    connect(this, SIGNAL(languageChanged()), this, SLOT(changeLanguage()));

    setupTreeView();
    connect(ui_treeView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(treeClicked(const QModelIndex&)));

    setupURLRequester();
//    connect(ui_kURL, SIGNAL(urlSelected(const KUrl&)), this, SLOT(urlSelected(const KUrl&)));
    connect(ui_kURL, SIGNAL(textChanged(const QString&)), this, SLOT(urlTextChanged(const QString&)));
//    connect(ui_kURL, SIGNAL(returnPressed()), this, SLOT(urlReturnPressed()));

    connect(ui_subdirCheckBox, SIGNAL(stateChanged(int)), this, SLOT(subdirStateChanged(int)));
    connect(ui_selectAllButton, SIGNAL(clicked()), this, SLOT(selectAll()));
    connect(ui_deselectAllButton, SIGNAL(clicked()), this, SLOT(deselectAll()));
}

/**
 * Destructor.
 */
CodeImpSelectPage::~CodeImpSelectPage()
{
}

/**
 * Fills the language combo box with items and
 * sets the currently selected value.
 */
void CodeImpSelectPage::setupLanguageBox()
{
#if true
    QStringList languages;
    languages += CodeImpSelectPage::ADA;
    languages += CodeImpSelectPage::CPP;
    languages += CodeImpSelectPage::IDL;
    languages += CodeImpSelectPage::JAVA;
    languages += CodeImpSelectPage::PASCAL;
    languages += CodeImpSelectPage::PYTHON;

    ui_languageBox->addItems(languages);

    Uml::ProgrammingLanguage pl = UMLApp::app()->activeLanguage();
    QString selectedLanguage = CodeImpSelectPage::CPP;
    if (pl == Uml::ProgrammingLanguage::IDL) {
        selectedLanguage = CodeImpSelectPage::IDL;
    } else if (pl == Uml::ProgrammingLanguage::Python) {
        selectedLanguage = CodeImpSelectPage::PYTHON;
    } else if (pl == Uml::ProgrammingLanguage::Java) {
        selectedLanguage = CodeImpSelectPage::JAVA;
    } else if (pl == Uml::ProgrammingLanguage::Pascal) {
        selectedLanguage = CodeImpSelectPage::PASCAL;
    } else if (pl == Uml::ProgrammingLanguage::Ada) {
        selectedLanguage = CodeImpSelectPage::ADA;
    }

    QRegExp languageRegExp(selectedLanguage, Qt::CaseSensitive, QRegExp::FixedString);
    ui_languageBox->setCurrentIndex(languages.indexOf(languageRegExp));
    ui_languageBox->setEditable(false);
#else
    int indexCounter = 0;
    while (indexCounter < Uml::ProgrammingLanguage::Reserved) {
        QString language = Model_Utils::progLangToString((Uml::Programming_Language) indexCounter);
        ui_languageBox->insertItem(indexCounter, language);
        indexCounter++;
    }
    ui_languageBox->setCurrentIndex(pl);
#endif
}

/**
 *
 */
void CodeImpSelectPage::setupURLRequester()
{
    ui_kURL->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
}

/**
 *
 */
void CodeImpSelectPage::setupTreeView()
{
    QStringList list;
    list << "<empty>\t0\t-\t-";
//    list << "testName2\t456\tfilePath2\tfalse";
    TreeModel* model = new TreeModel(list);
    ui_treeView->setSelectionMode(QAbstractItemView::MultiSelection);
    ui_treeView->setModel(model);
    ui_treeView->setWindowTitle(i18n("Simple Tree Model"));
    ui_treeView->show();
}

/**
 *
 * @param path   ...
 * @return       ...
 */
bool CodeImpSelectPage::matchFilter(const QFileInfo& path)
{
    QString filename = path.fileName();
    return filename.contains(m_fileExtensionPattern);
}

/**
 * Recursively get all the sources files that matches the filters from the given path.
 * @param path      path to the parent directory
 * @param filters   file extensions of the wanted files
 */
void CodeImpSelectPage::files(const QString& path, QStringList& filters)
{
    //uDebug() << "files from path " << path;
    QDir searchDir(path);
    if (searchDir.exists()) {
        foreach (const QFileInfo &file, searchDir.entryInfoList(filters, QDir::Files)) {
            if (matchFilter(file)) {
                m_fileList.append(file);
                uDebug() << "file = " << file.absoluteFilePath();
            }
        }
        if (ui_subdirCheckBox->isChecked()) {
            foreach (const QFileInfo &subDir, searchDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks)) {
                m_fileList.append(subDir);
                //uDebug() << "directory = " << subDir.fileName();
                files(searchDir.absoluteFilePath(subDir.fileName()), filters);
            }
        }
    }
    else {
        uDebug() << "searchDir does not exist: " << searchDir.path();
    }
}

/**
 *
 * @param url   ...
 */
void CodeImpSelectPage::urlSelected(const KUrl& url)
{
    uDebug() << "loading tree view from " << url.url() << " ...";
    QStringList fileNames;
    files(url.toLocalFile(), fileNames);
    QStringList lines;
    foreach (const QFileInfo& file, m_fileList) {
        QString line;
        line = file.fileName() + "\t" + QString::number(file.size()) + "\t" + file.absoluteFilePath() + "\t" + (file.isDir() ? "directory" : "");
        //uDebug() << line;
        lines.append(line);
    }
    QItemSelectionModel* m = ui_treeView->selectionModel();
    TreeModel* model = new TreeModel(lines);
    ui_treeView->setModel(model);
    delete m;
    setSelectionCounter();
}

/**
 *
 * @param text   ...
 */
void CodeImpSelectPage::urlTextChanged(const QString& text)
{
    uDebug() << "text changed in KUrl to " << text;
    QDir searchDir(text);
    if (searchDir.exists()) {
        uDebug() << "building tree with " << text;
        KUrl url(text);
        urlSelected(url);
    }
}

/**
 *
 */
void CodeImpSelectPage::urlReturnPressed()
{
    uDebug() << "return pressed in KUrl";
}

/**
 *
 * @param state   ...
 */
void CodeImpSelectPage::subdirStateChanged(int state)
{
    if (ui_kURL->text().isEmpty()) {
        uDebug() << "no path given yet";
    }
    else {
        QString strState;
        switch (state) {
        case Qt::Unchecked:
            strState = "Unchecked";
            break;
        case Qt::Checked:
            strState = "Checked";
            break;
        default:
            strState = "not known";
            break;
        }
        uDebug() << "state set to " << strState;
        urlSelected(ui_kURL->url());
    }
}

/**
 *
 * @param index   ...
 */
void CodeImpSelectPage::treeClicked(const QModelIndex& index)
{
    //QModelIndexList list = ui_treeView->selectionModel()->selectedRows();

    // if is directory then select all children
    //:TODO: disconnect clicked signal / select children / count children / connect clicked signal
    // update label
    setSelectionCounter();
}

/**
 * Reimplemented QWizardPage method to validate page when user clicks next button.
 * @return   the validation state
 */
bool CodeImpSelectPage::validatePage()
{
    return save();
}

/**
 * Reads the set values from their corresponding widgets, writes them back to 
 * the data structure, and notifies clients.
 */
void CodeImpSelectPage::apply()
{
//    if (m_parentPolicy) {
//        m_parentPolicy->setCodeVerboseDocumentComments(ui_forceDoc->isChecked());
//        m_parentPolicy->setCodeVerboseSectionComments(ui_forceSections->isChecked());
//        m_parentPolicy->setOutputDirectory(QDir(ui_outputDir->text()));
//        m_parentPolicy->setIncludeHeadings(ui_includeHeadings->isChecked());
//        m_parentPolicy->setHeadingFileDir(ui_headingsDir->text());
//        m_parentPolicy->setOverwritePolicy(widgetToOverwrite());
//        m_parentPolicy->setLineEndingType((CodeGenerationPolicy::NewLineType) ui_SelectEndLineCharsBox->currentIndex());
//        m_parentPolicy->setIndentationType((CodeGenerationPolicy::IndentationType) ui_SelectIndentationTypeBox->currentIndex());
//        m_parentPolicy->setIndentationAmount(ui_SelectIndentationNumber->value());
//
//        // Emit in THIS order. The first signal triggers any sub-class to do its apply
//        // slot, THEN, once we are all updated, we may sync the parent generator's code
//        // documents.
//        emit applyClicked();
//        emit syncCodeDocumentsToParent();
//    }
}

/**
 * This function is called when leaving this wizard page.
 * Saves the made settings and checks some values.
 * @return   the success state
 */
bool CodeImpSelectPage::save()
{
    // first save the settings to the selected generator policy
    apply();

    // get the output directory path
//    QFileInfo info(policy->getOutputDirectory().absolutePath());
//    if (info.exists()) {
//        // directory exists... make sure we can write to it
//        if (!info.isWritable()) {
//            KMessageBox::sorry(this,i18n("The output folder exists, but it is not writable.\nPlease set the appropriate permissions or choose another folder."),
//                    i18n("Error Writing to Output Folder"));
//            return false;
//        }
//        // it exits and we can write... make sure it is a directory
//        if (!info.isDir()) {
//            KMessageBox::sorry(this,i18n("%1 does not seem to be a folder. Please choose a valid folder.", info.filePath()),
//                    i18n("Please Choose Valid Folder"));
//            return false;
//        }
//    }
//    else {
//        if (KMessageBox::questionYesNo(this,
//                        i18n("The folder %1 does not exist. Do you want to create it now?", info.filePath()),
//                        i18n("Output Folder Does Not Exist"), KGuiItem(i18n("Create Folder")), KGuiItem(i18n("Do Not Create"))) == KMessageBox::Yes)
//        {
//            QDir dir;
//            if (!dir.mkdir(info.filePath())) {
//                KMessageBox::sorry(this,i18n("The folder could not be created.\nPlease make sure you have write access to its parent folder or select another, valid, folder."),
//                            i18n("Error Creating Folder"));
//                return false;
//            }
//            // else, directory created
//        }
//        else {  // do not create output directory
//            KMessageBox::information(this,i18n("Please select a valid folder."),
//                          i18n("Output Folder Does Not Exist"));
//            return false;
//        }
//    }
    return true;
}

/**
 * Transform signal.
 * @param id   position in combo box
 */
void CodeImpSelectPage::languageChanged(int id)
{
    Q_UNUSED(id);
    emit languageChanged();
}

/**
 * When the user changes the language, the codegenoptions page
 * language-dependent stuff has to be updated.
 * The way to do this is to call its "apply" method.
 */
void CodeImpSelectPage::changeLanguage()
{
    QString plStr = language();
    Uml::ProgrammingLanguage pl = Uml::ProgrammingLanguage::fromString(plStr);
    UMLApp::app()->setActiveLanguage(pl);
    /* @todo is this needed? if yes adapt to new scheme
     m_CodeGenOptionsPage->setCodeGenerator(m_doc->getCurrentCodeGenerator());
     */
    apply();
    // set the file extension pattern with which the files are filtered
    switch (pl) {  //:TODO: more languages?
    case Uml::ProgrammingLanguage::Ada:
        m_fileExtensionPattern.setPattern("\\.ad[sba]$");
        break;
    case Uml::ProgrammingLanguage::Cpp:
        m_fileExtensionPattern.setPattern("\\.h$");
        break;
    case Uml::ProgrammingLanguage::IDL:
        m_fileExtensionPattern.setPattern("\\.idl$");
        break;
    case Uml::ProgrammingLanguage::Java:
        m_fileExtensionPattern.setPattern("\\.java$");
        break;
    case Uml::ProgrammingLanguage::Pascal:
        m_fileExtensionPattern.setPattern("\\.pas$");
        break;
    case Uml::ProgrammingLanguage::Python:
        m_fileExtensionPattern.setPattern("\\.pyw?$");
        break;
    default:
        break;
    }
    uDebug() << "File extension pattern set to " << m_fileExtensionPattern.pattern();
}

/**
 * Slot for clicked events of the browse buttons.
 * The selected directory is written to its corresponding text field.
 */
// void CodeImpSelectPage::browseClicked()
// {
//     QString dir = KFileDialog::getExistingDirectory();
//     if (dir.isEmpty()) {
//         return;
//     }
// 
//     QString button = sender()->objectName();
//     if (button == "ui_browseOutput") {
// //        ui_outputDir->setText(dir);
//     }
//     else if (button == "ui_browseHeadings") {
// //        ui_headingsDir->setText(dir);
//     }
// }

/**
 * Returns the user selected language used for code generation.
 * @return   the programming language name 
 */
QString CodeImpSelectPage::language()
{
    return ui_languageBox->currentText();
}

/**
 * Returns the list of files, which will be used for code import.
 * @return   the file info list
 */
QList<QFileInfo> CodeImpSelectPage::selectedFiles()
{
    QModelIndexList indexList = ui_treeView->selectionModel()->selectedRows();
    TreeModel* model = (TreeModel*)ui_treeView->model();
    QList<QFileInfo> fileList;
    foreach(QModelIndex index, indexList) {
        QMap<int, QVariant> map = model->itemData(index);
        QString fileName = map.value(0).toString();
        foreach(const QFileInfo& fileInfo, m_fileList) {
            if (fileInfo.fileName() == fileName) {
                fileList.append(fileInfo);
            }
        }
    }
    return fileList;
}

/**
 * Slot for select all signal in the tree view.
 */
void CodeImpSelectPage::selectAll()
{
    ui_treeView->selectAll();
    setSelectionCounter();
}

/**
 * Slot for deselect all signal in the tree view.
 */
void CodeImpSelectPage::deselectAll()
{
    ui_treeView->clearSelection();
    setSelectionCounter();
}

/**
 * Utility method for setting the selection counter.
 */
void CodeImpSelectPage::setSelectionCounter()
{
    QModelIndexList list = ui_treeView->selectionModel()->selectedRows();
    ui_filesNumLabel->setText(QString::number(list.size()));
}

#include "codeimpselectpage.moc"
