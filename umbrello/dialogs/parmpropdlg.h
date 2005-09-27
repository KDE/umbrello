/*
 *  copyright (C) 2002-2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PARMPROPDLG_H
#define PARMPROPDLG_H

#include <kdialogbase.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qmultilineedit.h>
#include <qcombobox.h>
#include "../attribute.h"

class UMLDoc;
/**
 * Displays a dialog box that displays properties of a paramater.
 * You need to make sure that @ref UMLDoc is made to be the
 * parent.
 *
 * @short A properties dialog box for a parameter.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ParmPropDlg : public KDialogBase {
public:
    /**
     * Constructs a ParmPropDlg.
     *
     * @param parent    The parent of the dialog.
     * @param a The parameter to represent.
     */
    ParmPropDlg(QWidget * parent, UMLDoc * doc, UMLAttribute * a);

    /**
     * Standard deconstructor.
     */
    ~ParmPropDlg();

    /**
     * Returns the documentation.
     *
     * @return  Returns the documentation.
     */
    QString getDoc() {
        return m_pDoc -> text();
    }

    /**
     * Return the name of the parameter.
     *
     * @return  Return the name of the parameter.
     */
    QString getName() {
        return m_pNameLE -> text();
    }

    /**
     * Return the initial value of the parameter.
     *
     * @return  Return the initial value of the parameter.
     */
    QString getInitialValue() {
        return m_pInitialLE -> text();
    }

    /**
     * Return the type name of the parameter.
     *
     * @return Return the type name of the parameter.
     */
    QString getTypeName() {
        return m_pTypeCB -> currentText();
    }

    /**
     * Return the kind of the parameter (in, out, or inout).
     *
     * @return  The Uml::Parameter_Direction corresponding to
     *          the selected "Kind" radiobutton.
     */
    Uml::Parameter_Direction getParmKind();

public slots:
    void slotOk();

private:
    QGroupBox * m_pParmGB, * m_pDocGB;
    QButtonGroup *m_pKind;
    QRadioButton * m_pIn, * m_pOut, *m_pInOut;
    QLabel * m_pTypeL, * m_pNameL, * m_pInitialL, * m_pStereoTypeL;
    QComboBox * m_pTypeCB;
    QLineEdit * m_pNameLE, * m_pInitialLE, * m_pStereoTypeLE;
    QMultiLineEdit * m_pDoc;
    UMLDoc * m_pUmldoc;
    UMLAttribute * m_pAtt;
};

#endif
