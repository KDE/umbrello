/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef SELECTOPERATIONDIALOG_H
#define SELECTOPERATIONDIALOG_H

#include "singlepagedialogbase.h"

class KComboBox;
class QLineEdit;
class QCheckBox;
class QGroupBox;
class QLabel;
class UMLClassifier;
class UMLView;
class LinkWidget;

/**
 * A dialog used to select an operation.
 *
 * @short A dialog to select an operation.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class SelectOperationDialog : public SinglePageDialogBase
{
    Q_OBJECT
public:
    SelectOperationDialog(UMLView * parent, UMLClassifier * c, LinkWidget *widget, bool enableAutoIncrement = true);
    ~SelectOperationDialog();

    QString getOpText();

    bool isClassOp() const;

    QString getSeqNumber();

    bool autoIncrementSequence();
    void setAutoIncrementSequence(bool state);

    void setSeqNumber(const QString &num);
    void setCustomOp(const QString &op);
    bool setClassOp(const QString &op);

    bool apply();

    Q_SLOT void slotAutoIncrementChecked(bool state);
    Q_SLOT void slotNewOperation();
    Q_SLOT void slotIndexChanged(int index);
    Q_SLOT void slotTextChanged(const QString &text);

protected:
    void setupDialog();
    void setupOperationsList();

private:
    static const int OP     = 0;
    static const int CUSTOM = 1;

    QGroupBox        *m_pOpGB;
    KComboBox        *m_pOpCB;
    QLabel           *m_pSeqL;
    QLineEdit        *m_pOpLE, *m_pSeqLE;
    QLabel           *m_pCustomRB, *m_pOpRB;
    QPushButton      *m_newOperationButton;
    QCheckBox        *m_pOpAS;
    QString           m_Text;
    int               m_nOpCount;
    int               m_id;  ///< takes on the value OP or CUSTOM according to what the user selected
    UMLView          *m_pView;
    UMLClassifier    *m_classifier;
    LinkWidget       *m_widget;
    bool              m_enableAutoIncrement;
};

#endif

