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
#ifndef CODEIMPSELECTPAGE_H
#define CODEIMPSELECTPAGE_H

// qt includes
#include <QtGui/QWizardPage>

// app includes
#include "ui_codeimpselectpage.h"

/**
 * This class is used in the code importing wizard.
 *
 * @author Andi Fischer
 */
class CodeImpSelectPage : public QWizardPage, private Ui::CodeImpSelectPage
{
    Q_OBJECT
public:
    CodeImpSelectPage(QWidget *parent = 0);
    ~CodeImpSelectPage();

    QString language();
    void apply();
    bool save();
    bool validatePage();

    QList<QFileInfo> selectedFiles();

private:
    void files(const QString& path, QStringList& filters);
    bool matchFilter(const QFileInfo& path);
    void setSelectionCounter();

    QList<QFileInfo> m_fileList;
    QRegExp          m_fileExtensionPattern;

    static const QString ADA;
    static const QString CPP;
    static const QString IDL;
    static const QString JAVA;
    static const QString PASCAL;
    static const QString PYTHON;

    static QString s_recentPath;

    void setupLanguageBox();
    void setupURLRequester();
    void setupTreeView();

protected slots:
    void languageChanged(int id);
//    void browseClicked();
    void treeClicked(const QModelIndex& index);

private slots:
    void urlSelected(const KUrl& url);
    void urlTextChanged(const QString& text);
    void urlReturnPressed();
    void changeLanguage();
    void subdirStateChanged(int state);
    void selectAll();
    void deselectAll();

signals:
    void applyClicked();
    void languageChanged();
    void syncCodeDocumentsToParent();

};

#endif
