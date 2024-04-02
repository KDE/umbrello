/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2002 Luis De la Parra <luis@delaparra.org>
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/
#ifndef CODEGENSTATUSPAGE_H
#define CODEGENSTATUSPAGE_H

// app includes
#include "ui_codegenstatuspage.h"
#include "basictypes.h"
#include "classifier.h"
#include "codegenerationpolicy.h"
#include "codegenerator.h"

// kde includes
#include <kled.h>

// qt includes
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QWizardPage>

/**
 * @author Luis De la Parra
 * @author Brian Thomas
 * @author Andi Fischer
 */
class CodeGenStatusPage : public QWizardPage, private Ui::CodeGenStatusPage
{
    Q_OBJECT
public:
    explicit CodeGenStatusPage(QWidget *parent = nullptr);
    ~CodeGenStatusPage();
    void initializePage();
    bool isComplete() const;

private:
    bool m_generationDone;

protected:
    Q_SLOT void generateCode();
    Q_SLOT void classGenerated(UMLClassifier* classifier, bool generated);
    Q_SLOT void classGenerated(UMLClassifier* classifier, CodeGenerator::GenerationState state);
    Q_SLOT void populateStatusList();
    Q_SLOT void showFileGenerated(const QString& filename);
    Q_SLOT void loggerClear();
    Q_SLOT void loggerExport();

};

class LedStatus : public QWidget
{
        Q_OBJECT
    public:
        LedStatus(int width, int height) {
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
        KLed *m_led;
};

#endif
