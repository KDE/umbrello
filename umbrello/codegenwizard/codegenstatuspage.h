/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002                                                    *
 *   Luis De la Parra  <luis@delaparra.org>                                *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
#ifndef CODEGENSTATUSPAGE_H
#define CODEGENSTATUSPAGE_H

// app includes
#include "ui_codegenstatuspage.h"
#include "basictypes.h"
#include "classifier.h"
#include "codegenerationpolicy.h"

// kde includes
#include <kled.h>

// qt includes
#include <QtGui/QWizardPage>
#include <QtGui/QSpacerItem>
#include <QtGui/QHBoxLayout>


/**
 * @author Luis De la Parra
 * @author Brian Thomas
 * @author Andi Fischer
 */
class CodeGenStatusPage : public QWizardPage, private Ui::CodeGenStatusPage
{
    Q_OBJECT
public:
    CodeGenStatusPage(QWidget *parent = 0);
    ~CodeGenStatusPage();
    void initializePage();
    bool isComplete() const;

private:
    bool m_generationDone;
      
protected slots:
    void generateCode();
    void classGenerated(UMLClassifier* concept, bool generated);
    void populateStatusList();

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
        };
        ~LedStatus() {};
        void setColor(const QColor& color) { m_led->setColor(color); };
        void setOn(bool isOn) { isOn ? m_led->setState(KLed::On) : m_led->setState(KLed::Off); };
    private:
        KLed *m_led;
};

#endif
