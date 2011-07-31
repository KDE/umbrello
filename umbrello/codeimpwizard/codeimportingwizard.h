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
#ifndef CODEIMPORTINGWIZARD_H
#define CODEIMPORTINGWIZARD_H

// qt includes
#include <QtGui/QWizard>

class QFileInfo;
class QWizardPage;
class CodeImpSelectPage;
class CodeImpStatusPage;

/**
 * @author Andi Fischer
 */
class CodeImportingWizard : public QWizard 
{
    Q_OBJECT
public:
    enum {SelectionPage, StatusPage};

    CodeImportingWizard();
    ~CodeImportingWizard();

    QList<QFileInfo> selectedFiles();

private:
    QWizardPage* createSelectionPage();
    QWizardPage* createStatusPage();

    CodeImpSelectPage  *m_SelectionPage;
    CodeImpStatusPage  *m_StatusPage;

    static const QSize c_pageSize;
};

#endif
