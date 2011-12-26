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
#ifndef CODEIMPSTATUSPAGE_H
#define CODEIMPSTATUSPAGE_H

// app includes
#include "ui_codeimpstatuspage.h"
#include "classifier.h"

//kde includes
#include <kled.h>

// qt includes
#include <QtGui/QWizardPage>
#include <QtGui/QSpacerItem>
#include <QtGui/QHBoxLayout>
#include <QtCore/QFileInfo>

class QThread;
/**
 * This class is used in the code importing wizard.
 * It represents the second page where files are listed and imported by parsing.
 * The status of the work and a log of actions is shown.
 * TODO: Make the LedStatus class more private.
 * @author Andi Fischer
 */
class CodeImpStatusPage : public QWizardPage, private Ui::CodeImpStatusPage
{
    Q_OBJECT
public:
    CodeImpStatusPage(QWidget *parent = 0);
    ~CodeImpStatusPage();
    void initializePage();
    bool isComplete() const;

private:
    QList<QFileInfo> m_files; ///< list of files to import
    bool             m_workDone;
    int              m_index; ///< index in m_files
    QFileInfo        m_file; ///< current file
#ifdef ENABLE_IMPORT_THREAD
    QThread         *m_thread;
#endif

protected slots:
    void importCode(); ///< start importing
    void importCodeFile(); ///< import single file
    void importCodeFinish(); ///< finish importing
    void importCodeStop(); ///< cancel importing
    void updateStatus(const QString& file, const QString& text);
    void messageToLog(const QString& file, const QString& text);
    void messageToApp(const QString& text);
    void populateStatusList();
    void loggerClear();
    void loggerExport();
};

namespace CodeImport {

    class LedStatus : public QWidget
    {
        Q_OBJECT
    public:
        LedStatus(int width, int height) : QWidget() {
            setFixedSize(width, height);
            QHBoxLayout* layout = new QHBoxLayout();
            layout->addItem(new QSpacerItem(20, 20));
            m_led = new KLed(QColor(124, 252, 0), KLed::Off, KLed::Sunken, KLed::Circular);
            layout->addWidget(m_led);
            layout->addItem(new QSpacerItem(20, 20));
            setLayout(layout);
        }
        ~LedStatus() {}
        void setColor(const QColor& color) { m_led->setColor(color); }
        void setOn(bool isOn) { isOn ? m_led->setState(KLed::On) : m_led->setState(KLed::Off); }
    private:
        KLed* m_led;
};

}  // namespace

#endif
