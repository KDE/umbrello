/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>
    SPDX-FileCopyrightText: 2012-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "codeimpselectpage.h"

// app includes
#include "debug_utils.h"
#include "model_utils.h"
#include "uml.h"

DEBUG_REGISTER(CodeImpSelectPage)

// kde includes
#include <KLocalizedString>

// qt includes
#include <QFileSystemModel>

/**
 * Keep the last clicked directory for setting it the next time.
 */
QString CodeImpSelectPage::s_recentPath;

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

    connect(ui_subdirCheckBox, SIGNAL(stateChanged(int)), this, SLOT(subdirStateChanged(int)));
    connect(ui_selectAllButton, SIGNAL(clicked()), this, SLOT(selectAll()));
    connect(ui_deselectAllButton, SIGNAL(clicked()), this, SLOT(deselectAll()));

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
    model->setRootPath(QString());
    model->setNameFilterDisables(false);

    m_fileExtensions << QStringLiteral("*.h")
                     << QStringLiteral("*.hpp")
                     << QStringLiteral("*.hh")
                     << QStringLiteral("*.hxx")
                     << QStringLiteral("*.H");  //:TODO set according to the current language!
    model->setNameFilters(m_fileExtensions);

    ui_treeView->setSelectionMode(QAbstractItemView::MultiSelection);
    ui_treeView->setModel(model);
    ui_treeView->setIndentation(20);
    ui_treeView->setColumnWidth(0, 200);

    ui_treeView->setSortingEnabled(true);
    ui_treeView->header()
        ->setSortIndicator(0, Qt::AscendingOrder);

    ui_treeView->setWindowTitle(i18n("File System Model"));
    if (s_recentPath.isEmpty()) {
        ui_treeView->setCurrentIndex(model->index(QDir::currentPath()));
    }
    else {
        ui_treeView->setCurrentIndex(model->index(s_recentPath));
    }
    ui_treeView->scrollTo(ui_treeView->currentIndex());
    ui_treeView->setMouseTracking(true);
    ui_treeView->show();
}

/**
 * Setup the tree view and file extension widget.
 * Call it after setupTreeView(), because the extensions are set there.
 */
void CodeImpSelectPage::setupFileExtEdit()
{
    ui_fileExtLineEdit->setText(m_fileExtensions.join(QStringLiteral(", ")));
}

/**
 * Setup the tool tips for every widget.
 * TODO: Do it here or in the ui file? 
 */
void CodeImpSelectPage::setupToolTips()
{
    ui_languageBox->setToolTip(i18n("Select the desired language to filter files."));
    ui_subdirCheckBox->setToolTip(i18n("Select also all the files in the subdirectories."));
    ui_selectAllButton->setToolTip(i18n("Select all the files below the current directory."));
    ui_deselectAllButton->setToolTip(i18n("Clear all selections."));
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
    for(QString& extension: m_fileExtensions) {
        extension.remove(QLatin1Char('*'));
        if (filename.endsWith(extension)) {
            found = true;
            break;
        }
    }
    return found;
}

/**
 * Slot for the stateChanged event of the subdirectory check box.
 * @param state   check box state (Qt::Checked / Qt::Unchecked)
 */
void CodeImpSelectPage::subdirStateChanged(int state)
{
    QString strState;
    switch (state) {
    case Qt::Unchecked:
        strState = QStringLiteral("Unchecked");
        break;
    case Qt::Checked:
        strState = QStringLiteral("Checked");
        break;
    default:
        strState = QStringLiteral("not known");
        break;
    }
    logDebug1("CodeImpSelectPage::subdirStateChanged: state set to %1", strState);
}

/**
 * Slot for the editingFinished event of the line edit widget for the extensions.
 */
void CodeImpSelectPage::fileExtChanged()
{
    QString inputStr = ui_fileExtLineEdit->text();
    m_fileExtensions = inputStr.split(QRegularExpression(QStringLiteral("[,;: ]*")));
    logDebug1("CodeImpSelectPage: editing of file extension line edit finished and set to %1",
              m_fileExtensions.join(QStringLiteral(" ")));

    QFileSystemModel* model = (QFileSystemModel*)ui_treeView->model();
    model->setNameFilters(m_fileExtensions);
}

/**
 * Slot for the clicked event on one of the items of the tree view widget.
 * @param index   the index of the item on which was clicked
 */
void CodeImpSelectPage::treeClicked(const QModelIndex& index)
{
    if (index.isValid()) {
        logDebug2("CodeImpSelectPage::treeClicked: item at row=%1 / column=%2",
                  index.row(), index.column());
        QFileSystemModel* indexModel = (QFileSystemModel*)index.model();
        QFileInfo fileInfo = indexModel->fileInfo(index);
        if (fileInfo.isDir()) {
            int rows = indexModel->rowCount(index);
            logDebug1("CodeImpSelectPage::treeClicked: item has directory and has %1 children",
                      rows);
            QItemSelectionModel* selectionModel = ui_treeView->selectionModel();
            for(int row = 0; row < rows; ++row) {
                QModelIndex childIndex = indexModel->index(row, 0, index);
                if (selectionModel->isSelected(index)) {
                    // uDebug() << "select all children";
                    QFileInfo childInfo = indexModel->fileInfo(childIndex);
                    if (childInfo.isDir() && ui_subdirCheckBox->isChecked()) {
                        treeClicked(childIndex);
                    }
                    else {
                        if (matchFilter(childInfo)) {
                            selectionModel->select(childIndex, QItemSelectionModel::Select);
                        }
                        else {
                            selectionModel->select(childIndex, QItemSelectionModel::Deselect);
                        }
                    }
                }
                else {
                    // uDebug() << "deselect all children";
                    selectionModel->select(childIndex, QItemSelectionModel::Deselect);
                }
            }
            // keep the latest clicked directory
            s_recentPath = fileInfo.filePath();
        }
        updateSelectionCounter();
        Q_EMIT selectionChanged();
    }
    else {
        logWarn0("CodeImpSelectPage::treeClicked: Index not valid!");
    }
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
    logDebug1("CodeImpSelectPage::validatePage is entered... recent root path: %1",
              s_recentPath);

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
    Q_EMIT languageChanged();
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
    m_fileExtensions = Uml::ProgrammingLanguage::toExtensions(pl);
    logDebug1("CodeImpSelectPage::changeLanguage: File extensions %1",
              m_fileExtensions.join(QStringLiteral(" ")));

    QFileSystemModel* model = (QFileSystemModel*)ui_treeView->model();
    model->setNameFilters(m_fileExtensions);

    ui_fileExtLineEdit->setText(m_fileExtensions.join(QStringLiteral(", ")));
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
    for(const QModelIndex &idx: list) {
        if (idx.row() != row && idx.column() == 0) {
            QFileInfo fileInfo = model->fileInfo(idx);
            if (fileInfo.isFile() && matchFilter(fileInfo)) {
                fileList.append(fileInfo);
            }
            row = idx.row();
        }
    }
    return fileList;
}

/**
 * Slot for clicked event on the button widget.
 * Select all items in the current selected directory.
 * If the checkbox 'ui_subdirCheckBox' is selected
 * also all the files in the subdirectories are selected.
 */
void CodeImpSelectPage::selectAll()
{
    QModelIndex currIndex = ui_treeView->selectionModel()->currentIndex();
    if (!currIndex.isValid()) {
        logWarn0("CodeImpSelectPage::selectAll: Invalid index");
        return;
    }
    QFileSystemModel* model = (QFileSystemModel*)ui_treeView->model();
    QFileInfo fileInfo = model->fileInfo(currIndex);
    if (fileInfo.isDir()) {
        QItemSelectionModel* selectionModel = ui_treeView->selectionModel();
        Q_UNUSED(selectionModel);
        //...
        if (ui_subdirCheckBox->isChecked()) {
            //...
            ui_treeView->selectAll();
            updateSelectionCounter();
        }
    }
    else {
        logWarn0("CodeImpSelectPage::selectAll: No directory was selected");
    }
}

/**
 * Slot for clicked event on the button widget.
 * Deselects all items in the entire tree.
 */
void CodeImpSelectPage::deselectAll()
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

