/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef CODEIMPSELECTPAGE_H
#define CODEIMPSELECTPAGE_H

// qt includes
#include <QFileInfo>
#include <QWizardPage>

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
    explicit CodeImpSelectPage(QWidget *parent = nullptr);
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
protected:
    Q_SLOT void languageChanged(int id);
    Q_SLOT void treeClicked(const QModelIndex& index);
    Q_SLOT void treeEntered(const QModelIndex& index);

private:
    Q_SLOT void changeLanguage();
    Q_SLOT void subdirStateChanged(int state);
    Q_SLOT void fileExtChanged();
    Q_SLOT void selectAll();
    Q_SLOT void deselectAll();
    Q_SLOT void updateSelectionCounter();

protected:
    Q_SIGNAL void applyClicked();
    Q_SIGNAL void languageChanged();
    Q_SIGNAL void syncCodeDocumentsToParent();
    Q_SIGNAL void selectionChanged();
};

#endif
