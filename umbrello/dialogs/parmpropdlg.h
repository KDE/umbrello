/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef PARMPROPDLG_H
#define PARMPROPDLG_H

#include "attribute.h"

#include <kdialog.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <ktextedit.h>

class UMLDoc;
class QComboBox;
class QGroupBox;
class QLabel;
class QRadioButton;

/**
 * Displays a dialog box that displays properties of a parameter.
 * You need to make sure that @ref UMLDoc is made to be the
 * parent.
 *
 * @short A properties dialog box for a parameter.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ParmPropDlg : public KDialog
{
    Q_OBJECT
public:

    ParmPropDlg(QWidget * parent, UMLDoc * doc, UMLAttribute * attr);
    ~ParmPropDlg();

public slots:

    void slotOk();

protected:

    /**
     * Returns the documentation.
     * @return  Returns the documentation.
     */
    QString getDoc() {
        return m_doc->toPlainText();
    }

    QString getName() {
        return m_pNameLE->text();
    }

    QString getInitialValue() {
        return m_pInitialLE->text();
    }

    QString getTypeName() {
        return m_pTypeCB->currentText();
    }

    Uml::Parameter_Direction getParmKind();

    void insertTypesSorted(const QString& type = "");

    void insertStereotypesSorted(const QString& type = "");

    bool validate();

protected slots:

    virtual void slotButtonClicked(int button);

private:
    QGroupBox * m_pParmGB, * m_docGB;
    QGroupBox *m_pKind;
    QRadioButton * m_pIn, * m_pOut, *m_pInOut;
    QLabel * m_pTypeL, * m_pNameL, * m_pInitialL, * m_pStereoTypeL;
    KComboBox * m_pTypeCB, * m_pStereoTypeCB;
    KLineEdit * m_pNameLE, * m_pInitialLE;
    KTextEdit * m_doc;
    UMLDoc * m_pUmldoc;
    UMLAttribute * m_pAtt;
};

#endif
