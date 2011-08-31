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
#ifndef CODEIMPSELECTPAGE_H
#define CODEIMPSELECTPAGE_H

// qt includes
#include <QtCore/QFileInfo>
#include <QtGui/QWizardPage>

// app includes
#include "ui_codeimpselectpage.h"

/**
 * This class is used in the code importing wizard.
 * It represents the first page where files are selected for importing.
 * @author Andi Fischer
 */
class CodeImpSelectPage : public QWizardPage, private Ui::CodeImpSelectPage
{
    Q_OBJECT
public:
    CodeImpSelectPage(QWidget *parent = 0);
    ~CodeImpSelectPage();

    QString language();
    bool validatePage();

    QList<QFileInfo> selectedFiles();

private:
    bool matchFilter(const QFileInfo& path);

    QList<QFileInfo> m_fileList;
    QStringList      m_fileExtensions;

    static QString s_recentPath;

    void setupLanguageBox();
    void setupTreeView();
    void setupFileExtEdit();
    void setupToolTips();

protected slots:
    void languageChanged(int id);
    void treeClicked(const QModelIndex& index);

private slots:
    void changeLanguage();
    void subdirStateChanged(int state);
    void fileExtChanged();
    void selectAll();
    void deselectAll();
    void updateSelectionCounter();

signals:
    void applyClicked();
    void languageChanged();
    void syncCodeDocumentsToParent();
    void selectionChanged();
};

#endif
