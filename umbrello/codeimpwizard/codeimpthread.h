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
#ifndef CODEIMPTHREAD_H
#define CODEIMPTHREAD_H

#include <QtCore/QFileInfo>
#include <QtGui/QMessageBox>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QWaitCondition>

class ClassImport;

/**
 *
 * @author Andi Fischer
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class CodeImpThread : public QThread
{
    Q_OBJECT
public:
    CodeImpThread(QList<QFileInfo> files, QObject* parent = 0);
    virtual ~CodeImpThread();

    virtual void run();

    int emitAskQuestion(const QString& question);

signals:
    void askQuestion(const QString& question, QMessageBox::StandardButton* answer);
    void messageToWiz(const QFileInfo& file, const QString& text);
    void messageToApp(const QString& text);

private slots:
    void questionAsked(const QString& question, QMessageBox::StandardButton* answer);

private:
    QList<QFileInfo>  m_files;
    QWaitCondition    m_waitCondition;
    QMutex            m_mutex;
    ClassImport*      m_importer;
};

#endif
