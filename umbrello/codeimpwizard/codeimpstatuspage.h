/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef CODEIMPSTATUSPAGE_H
#define CODEIMPSTATUSPAGE_H

// app includes
#include "ui_codeimpstatuspage.h"
#include "classifier.h"

//kde includes
#include <kled.h>

// qt includes
#include <QFileInfo>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QWizardPage>

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
    explicit CodeImpStatusPage(QWidget *parent = nullptr);
    ~CodeImpStatusPage();
    void initializePage();
    bool isComplete() const;

private:
    QList<QFileInfo> m_files; ///< list of files to import
    bool             m_workDone;
    bool             m_savedUndoEnabled; ///< saved undo enabled state
    int              m_index; ///< index in m_files
    QFileInfo        m_file; ///< current file
    bool             m_savedlistViewVisible; ///< indicates previous state of list view
#ifdef ENABLE_IMPORT_THREAD
    QThread         *m_thread;
#endif

protected slots:
    void importCode(); ///< start importing
    void importCodeFile(bool noError=true); ///< import single file
    void importNextFile(bool noError=true);
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
