/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/


#ifndef UMLOPERATIONDIALOG_H
#define UMLOPERATIONDIALOG_H

//kde includes
#include <kdialogbase.h>

/**
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLOperation;
class ListPopupMenu;
class QGroupBox;
class QListBox;
class QButtonGroup;
class QRadioButton;
class QPushButton;
class QLabel;
class QCheckBox;
class KComboBox;
class QLineEdit;
class UMLDoc;
class KArrowButton;

class UMLOperationDialog : public KDialogBase {
    Q_OBJECT

public:
    /**
    *   Constructor
    */
    UMLOperationDialog( QWidget * parent, UMLOperation * pOperation );

    /**
    *   Deconstructor
    */
    ~UMLOperationDialog();

protected:
    /**
    *   Sets up the dialog
    */
    void setupDialog();

    /**
     * Checks if changes are valid and applies them if they are,
     * else returns false
     */
    bool apply();

    /**
    * Inserts @p type into the type-combobox as well as its completion object.
    */
    void insertType( const QString& type, int index = -1 );

    /**
    *   The operation to represent.
    */
    UMLOperation * m_pOperation;

    /**
      * The UMLDocument where all objects live
     */
    UMLDoc *m_doc;

    /**
    *   Menu used in paramater list box.
    */
    ListPopupMenu * m_pMenu;

    /**
    * Inserts @p stereotype into the stereotype-combobox as well as its completion object.
    */
    void insertStereotype( const QString& type, int index = -1 );

    //GUI widgets
    QGroupBox  * m_pParmsGB, * m_pGenGB;
    QListBox * m_pParmsLB;
    QButtonGroup * m_pScopeBG;
    QRadioButton * m_pPublicRB, * m_pPrivateRB,  * m_pProtectedRB, *m_pImplementationRB;
    QLabel * m_pRtypeL, * m_pNameL, * m_pStereoTypeL;
    KComboBox * m_pRtypeCB, * m_pStereoTypeCB;
    QLineEdit * m_pNameLE;
    QCheckBox * m_pAbstractCB;
    QCheckBox * m_pStaticCB;
    QCheckBox * m_pQueryCB;
    QPushButton* m_pDeleteButton;
    QPushButton* m_pPropertiesButton;
    KArrowButton* m_pUpButton;
    KArrowButton* m_pDownButton;

public slots:
    void slotParmRightButtonPressed(QListBoxItem *item, const QPoint &p);
    void slotParmRightButtonClicked(QListBoxItem *item, const QPoint &p);
    void slotParmDoubleClick(QListBoxItem *item);
    void slotParmPopupMenuSel(int id);
    void slotNewParameter();
    void slotDeleteParameter();
    void slotParameterProperties();
    void slotParameterUp();
    void slotParameterDown();

    /**
     * enables or disables buttons
     */
    void slotParamsBoxClicked(QListBoxItem* parameterItem);

    /**
     * I don't think this is used, but if we had an apply button
     * it would slot into here
     */
    void slotApply();

    /**
     * Used when the OK button is clicked.  Calls apply()
     */
    void slotOk();
    void slotNameChanged( const QString & );

};

#endif
