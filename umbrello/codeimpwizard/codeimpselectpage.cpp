/*
    Copyright 2011  Andi Fischer  <andi.fischer@hispeed.ch>

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
#include "debug_utils.h"
#include "model_utils.h"
#include "uml.h"

// kde includes
#include <klocale.h>

// qt includes
#include <QFileSystemModel>

/**
 * Keep the last clicked directory for setting it the next time.
 */
QString CodeImpSelectPage::s_recentPath = "";

/**
 * Constructor.
 * @param parent   the parent (wizard) of this wizard page
 */
CodeImpSelectPage::CodeImpSelectPage(QWidget *parent)
  : QWizardPage(parent),
    m_fileList(),
    m_fileExtensions()
{
    setTitle(i18n("Code Importing Path"));
    setSubTitle(i18n("Select the code importing path."));

    setupUi(this);

    setupLanguageBox();
    connect(ui_languageBox, SIGNAL(activated(int)), this, SLOT(languageChanged(int)));
    connect(this, SIGNAL(languageChanged()), this, SLOT(changeLanguage()));

    setupTreeView();
    connect(ui_treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(treeClicked(QModelIndex)));
    connect(ui_treeView, SIGNAL(entered(QModelIndex)), this, SLOT(treeEntered(QModelIndex)));

    setupFileExtEdit();
    connect(ui_fileExtLineEdit, SIGNAL(editingFinished()), this, SLOT(fileExtChanged()));

    connect(ui_selectFolderCheckBox, SIGNAL(stateChanged(int)), this, SLOT(selectFolderStateChanged(int)));
    connect(ui_clearSelectionButton, SIGNAL(clicked()), this, SLOT(clearSelection()));

    setupToolTips();
    // update file extensions
    changeLanguage();
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
    int indexCounter = 0;
    while (indexCounter < Uml::ProgrammingLanguage::Reserved) {
        QString language = Uml::ProgrammingLanguage::toString(Uml::ProgrammingLanguage::fromInt(indexCounter));
        ui_languageBox->insertItem(indexCounter, language);
        indexCounter++;
    }
    Uml::ProgrammingLanguage::Enum pl = UMLApp::app()->activeLanguage();
    ui_languageBox->setCurrentIndex(pl);
}

/**
 * Setup the tree view widget.
 */
void CodeImpSelectPage::setupTreeView()
{
    QFileSystemModel* model = new QFileSystemModel();
    model->setRootPath("");
    model->setNameFilterDisables(false);

    m_fileExtensions << "*.h" << "*.hpp" << "*.hh" << "*.hxx" << "*.H";  //:TODO set according to the current language!
    model->setNameFilters(m_fileExtensions);

    ui_treeView->setSelectionMode(QAbstractItemView::MultiSelection);
    ui_treeView->setModel(model);
    ui_treeView->setIndentation(20);
    ui_treeView->setColumnWidth(0, 200);

    ui_treeView->setSortingEnabled(true);
    ui_treeView->header()
        ->setSortIndicator(0, Qt::AscendingOrder);

    setupInitialSelection();

    ui_treeView->setWindowTitle(i18n("File System Model"));
    ui_treeView->scrollTo(ui_treeView->currentIndex());
    ui_treeView->setMouseTracking(true);
    ui_treeView->show();
}

/**
 * Setup the tree view selection to an initial state
 */
void CodeImpSelectPage::setupInitialSelection()
{
    QFileSystemModel* model = (QFileSystemModel*)ui_treeView->model();

    if (s_recentPath.isEmpty()) {
        ui_treeView->setCurrentIndex(model->index(QDir::currentPath()));
    } else {
        ui_treeView->setCurrentIndex(model->index(s_recentPath));
    }

    QDir::Filters filters = QDir::AllDirs | QDir::NoDotAndDotDot;

    if (ui_selectFolderCheckBox->checkState() == Qt::Checked) {
        ui_treeView->setSelectionMode(QAbstractItemView::SingleSelection);
        model->setFilter(filters);
    } else {
        ui_treeView->setSelectionMode(QAbstractItemView::MultiSelection);
        model->setFilter(filters | QDir::Files);
    }

    updateSelectionCounter();
}

/**
 * Setup the tree view and file extension widget.
 * Call it after setupTreeView(), because the extensions are set there.
 */
void CodeImpSelectPage::setupFileExtEdit()
{
    ui_fileExtLineEdit->setText(m_fileExtensions.join(", "));
}

/**
 * Setup the tool tips for every widget.
 * TODO: Do it here or in the ui file? 
 */
void CodeImpSelectPage::setupToolTips()
{
    ui_languageBox->setToolTip(i18n("Select the desired language to filter files."));
    ui_selectFolderCheckBox->setToolTip(i18n("Select a single folder and import all files recursively."));
    ui_clearSelectionButton->setToolTip(i18n("Clear all selections."));
    ui_fileExtLineEdit->setToolTip(i18n("Add file extensions like e.g. '*.h *.hpp'."));
}

/**
 * Decide if the given file has one of the set extensions.
 * @param path   file info to be examined
 * @return       status if found or not
 */
bool CodeImpSelectPage::matchFilter(const QFileInfo& path)
{
    bool found = false;
    QString filename = path.fileName();
    foreach (QString extension, m_fileExtensions) {
        extension.remove('*');
        if (filename.endsWith(extension)) {
            found = true;
            break;
        }
    }
    return found;
}

/**
 * Slot for the stateChanged event of the "select folder" check box.
 * @param state   check box state (Qt::Checked / Qt::Unchecked)
 */
void CodeImpSelectPage::selectFolderStateChanged(int state)
{
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

    setupInitialSelection();
}

/**
 * Slot for the editingFinished event of the line edit widget for the extensions.
 */
void CodeImpSelectPage::fileExtChanged()
{
    QString inputStr = ui_fileExtLineEdit->text();
    m_fileExtensions = inputStr.split(QRegExp("[,;: ]*"));
    uDebug() << "editing of file extension line edit finished and set to "
             << m_fileExtensions;

    QFileSystemModel* model = (QFileSystemModel*)ui_treeView->model();
    model->setNameFilters(m_fileExtensions);
}

/**
 * Slot for the clicked event on one of the items of the tree view widget.
 * @param index   the index of the item on which was clicked
 */
void CodeImpSelectPage::treeClicked(const QModelIndex& index)
{
    QFileSystemModel* indexModel = (QFileSystemModel*)index.model();
    QFileInfo fileInfo = indexModel->fileInfo(index);

    if (fileInfo.isDir()) {
        // keep the latest clicked directory
        s_recentPath = fileInfo.filePath();
    }

    updateSelectionCounter();
    emit selectionChanged();
}

void CodeImpSelectPage::treeEntered(const QModelIndex &index)
{
    Q_UNUSED(index);
    ui_treeView->resizeColumnToContents(0);
}

/**
 * Reimplemented QWizardPage method to validate page when user clicks next button.
 * @return   the validation state
 */
bool CodeImpSelectPage::validatePage()
{
    uDebug() << "entered...";
    uDebug() << "recent root path: " << s_recentPath;

    return true;
}

/**
 * Slot of the activated event of the combo box widget.
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
 */
void CodeImpSelectPage::changeLanguage()
{
    QString plStr = language();
    Uml::ProgrammingLanguage::Enum pl = Uml::ProgrammingLanguage::fromString(plStr);
    UMLApp::app()->setActiveLanguage(pl);
    /* :TODO: When the user changes the language, the codegenoptions page
     * :TODO: language-dependent stuff has to be updated.
     * :TODO: Is this needed? If yes adapt to new scheme.
    m_CodeGenOptionsPage->setCodeGenerator(m_doc->getCurrentCodeGenerator());
    apply();
     */
    // set the file extension pattern with which the files are filtered
    m_fileExtensions.clear();
    switch (pl) {  //:TODO: More languages?
    case Uml::ProgrammingLanguage::Ada:
        m_fileExtensions << "*.ads" << "*.adb" << "*.ada";
        break;
    case Uml::ProgrammingLanguage::Cpp:
        m_fileExtensions << "*.h" << "*.hpp" << "*.hh" << "*.hxx" << "*.H";
        break;
    case Uml::ProgrammingLanguage::IDL:
        m_fileExtensions << "*.idl";
        break;
    case Uml::ProgrammingLanguage::Java:
        m_fileExtensions << "*.java";
        break;
    case Uml::ProgrammingLanguage::Pascal:
        m_fileExtensions << "*.pas";
        break;
    case Uml::ProgrammingLanguage::Python:
        m_fileExtensions << "*.py" << "*.pyw";
        break;
    case Uml::ProgrammingLanguage::CSharp:
        m_fileExtensions << "*.cs";
        break;
    default:
        break;
    }
    uDebug() << "File extensions " << m_fileExtensions;

    QFileSystemModel* model = (QFileSystemModel*)ui_treeView->model();
    model->setNameFilters(m_fileExtensions);

    ui_fileExtLineEdit->setText(m_fileExtensions.join(", "));

    // Clear selection and select initial selected directory
    setupInitialSelection();
}

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
 * @return   the list of the selected files
 */
QList<QFileInfo> CodeImpSelectPage::selectedFiles()
{
    QList<QFileInfo> fileList;
    QFileSystemModel* model = (QFileSystemModel*)ui_treeView->model();
    QModelIndexList list = ui_treeView->selectionModel()->selectedIndexes();
    int row = -1;
    foreach (QModelIndex idx, list) {
        if (idx.row() != row && idx.column() == 0) {
            QFileInfo fileInfo = model->fileInfo(idx);
            if (fileInfo.isFile() && matchFilter(fileInfo)) {
                fileList.append(fileInfo);
            }
            if (fileInfo.isDir() && ui_selectFolderCheckBox->isChecked()) {
                selectRecursive(fileInfo, fileList);
            }
            row = idx.row();
        }
    }
    return fileList;
}

/**
 * Recursively look for files in a directory matching the filter pattern
 */
void CodeImpSelectPage::selectRecursive(QFileInfo parent, QList<QFileInfo>& fileList)
{
    QDirIterator iterator(parent.absoluteFilePath(), QDirIterator::Subdirectories);

    while (iterator.hasNext()) {
        iterator.next();
        if (!iterator.fileInfo().isDir()) {
            QFileInfo fileInfo = iterator.fileInfo();
            if (fileInfo.isFile() && matchFilter(fileInfo)) {
                fileList.append(fileInfo);
            }
        }
    }
}

/**
 * Slot for clicked event on the button widget.
 * Deselects all items in the entire tree.
 */
void CodeImpSelectPage::clearSelection()
{
    ui_treeView->clearSelection();
    updateSelectionCounter();
}

/**
 * Utility method for setting the selection counter.
 */
void CodeImpSelectPage::updateSelectionCounter()
{
    QList<QFileInfo> files = selectedFiles();
    ui_filesNumLabel->setText(QString::number(files.size()));
}

#include "codeimpselectpage.moc"
