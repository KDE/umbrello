/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLOPERATIONDIALOG_H
#define UMLOPERATIONDIALOG_H

//kde includes
#include "singlepagedialogbase.h"
#include "n_stereoattrs.h"

class DocumentationWidget;
class KComboBox;
class DialogsPopupMenu;
class QAbstractButton;
class QGridLayout;
class QGroupBox;
class QListWidget;
class QListWidgetItem;
class QLabel;
class QRadioButton;
class QPushButton;
class QCheckBox;
class QToolButton;
class QLineEdit;
class UMLDoc;
class UMLObject;
class UMLOperation;
class UMLDatatypeWidget;
class UMLStereotypeWidget;
class VisibilityEnumWidget;

/**
 * @author Paul Hensgen
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLOperationDialog : public SinglePageDialogBase
{
    Q_OBJECT
public:

    UMLOperationDialog(QWidget * parent, UMLOperation * pOperation);
    ~UMLOperationDialog();

protected:

    void setupDialog();

    bool apply();

    void insertTypesSorted(const QString& type = QString());

    UMLOperation*     m_operation;  ///< The operation to represent.
    UMLDoc*           m_doc;        ///< The UMLDocument where all objects live.
    DialogsPopupMenu* m_menu;       ///< Menu used in parameter list box.

    //GUI widgets
    QGridLayout*          m_pGenLayout;
    QGroupBox*            m_pParmsGB;
    QGroupBox*            m_pGenGB;
    QListWidget*          m_pParmsLW;
    QGroupBox*            m_pScopeGB;
    QRadioButton*         m_pPublicRB;
    QRadioButton*         m_pPrivateRB;
    QRadioButton*         m_pProtectedRB;
    QRadioButton*         m_pImplementationRB;
    QLabel*               m_pNameL;
    QLabel*               m_pTagL[N_STEREOATTRS];
    UMLDatatypeWidget*    m_datatypeWidget;
    UMLStereotypeWidget*  m_stereotypeWidget;
    QLineEdit*            m_pNameLE;
    QLineEdit*            m_pTagLE[N_STEREOATTRS];
    QCheckBox*            m_pAbstractCB;
    QCheckBox*            m_pStaticCB;
    QCheckBox*            m_pQueryCB;
    QCheckBox*            m_virtualCB;
    QCheckBox*            m_inlineCB;
    QCheckBox*            m_pOverrideCB;
    QPushButton*          m_pDeleteButton;
    QPushButton*          m_pPropertiesButton;
    QToolButton*          m_pUpButton;
    QToolButton*          m_pDownButton;
    DocumentationWidget*  m_docWidget;
    VisibilityEnumWidget* m_visibilityEnumWidget;

public slots:
    void slotParmRightButtonPressed(const QPoint &p);
    void slotParmDoubleClick(QListWidgetItem *item);
    void slotMenuSelection(QAction* action);
    void slotNewParameter();
    void slotDeleteParameter();
    void slotParameterProperties();
    void slotParameterUp();
    void slotParameterDown();
    void slotParamsBoxClicked(QListWidgetItem* parameterItem);
    void slotStereoTextChanged(const QString &);
    void slotNameChanged(const QString &);
};

#endif
