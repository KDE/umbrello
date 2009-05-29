/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLOPERATIONDIALOG_H
#define UMLOPERATIONDIALOG_H

//kde includes
#include <kdialog.h>

//qt includes
#include <QtGui/QListWidgetItem>

class UMLOperation;
class ListPopupMenu;
class QGroupBox;
class QListWidget;
class QLabel;
class QRadioButton;
class QPushButton;
class QCheckBox;
class KComboBox;
class KLineEdit;
class UMLDoc;
class KArrowButton;

/**
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLOperationDialog : public KDialog
{
    Q_OBJECT

public:

    /**
     * Constructor.
     */
    UMLOperationDialog( QWidget * parent, UMLOperation * pOperation );

    /**
     * Destructor.
     */
    ~UMLOperationDialog();

protected:

    void setupDialog();

    bool apply();

    void insertTypesSorted( const QString& type = "" );

    /**
     * The operation to represent.
     */
    UMLOperation * m_operation;

    /**
     * The UMLDocument where all objects live.
     */
    UMLDoc * m_doc;

    /**
     * Menu used in parameter list box.
     */
    ListPopupMenu * m_menu;

    void insertStereotypesSorted( const QString& type );

    //GUI widgets
    QGroupBox  * m_pParmsGB, * m_pGenGB;
    QListWidget * m_pParmsLW;
    QGroupBox * m_pScopeGB;
    QRadioButton * m_pPublicRB, * m_pPrivateRB,  * m_pProtectedRB, * m_pImplementationRB;
    QLabel * m_pRtypeL, * m_pNameL, * m_pStereoTypeL;
    KComboBox * m_pRtypeCB, * m_pStereoTypeCB;
    KLineEdit * m_pNameLE;
    QCheckBox * m_pAbstractCB;
    QCheckBox * m_pStaticCB;
    QCheckBox * m_pQueryCB;
    QPushButton* m_pDeleteButton;
    QPushButton* m_pPropertiesButton;
    KArrowButton* m_pUpButton;
    KArrowButton* m_pDownButton;

public slots:
    void slotParmRightButtonPressed(const QPoint &p);
    void slotParmDoubleClick(QListWidgetItem *item);
    void slotParmPopupMenuSel(QAction* action);
    void slotNewParameter();
    void slotDeleteParameter();
    void slotParameterProperties();
    void slotParameterUp();
    void slotParameterDown();

    void slotParamsBoxClicked(QListWidgetItem* parameterItem);

    void slotApply();

    void slotOk();
    void slotNameChanged( const QString & );

};

#endif
