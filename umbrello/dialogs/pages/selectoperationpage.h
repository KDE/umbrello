/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef SELECTOPERATIONPAGE_H
#define SELECTOPERATIONPAGE_H

#include "dialogpagebase.h"

class KComboBox;
class QLineEdit;
class QCheckBox;
class QGroupBox;
class QLabel;
class QPushButton;
class LinkWidget;
class MessageWidget;
class UMLClassifier;
class UMLView;
class DocumentationWidget;

/**
 * A dialog page used to select an operation.
 *
 * @short A dialog page to select an operation.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class SelectOperationPage : public DialogPageBase
{
    Q_OBJECT
public:
    SelectOperationPage(UMLView *parent, UMLClassifier *c, LinkWidget *widget, bool enableAutoIncrement = true);
    ~SelectOperationPage();

    QString getOpText();

    bool isClassOp() const;

    QString getSeqNumber();

    bool autoIncrementSequence();
    void setAutoIncrementSequence(bool state);

    void setSeqNumber(const QString &num);
    void setCustomOp(const QString &op);
    bool setClassOp(const QString &op);

    void setupDialog();
    bool apply();

Q_SIGNALS:
    void enableButtonOk(bool state);

protected:
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
    DocumentationWidget *m_docWidget;
    int               m_nOpCount;
    int               m_id;  ///< takes on the value OP or CUSTOM according to what the user selected
    UMLView          *m_pView;
    UMLClassifier    *m_classifier;
    LinkWidget       *m_widget;
    bool              m_enableAutoIncrement;

public Q_SLOTS:
    void slotAutoIncrementChecked(bool state);
    void slotNewOperation();
    void slotIndexChanged(int index);
    void slotTextChanged(const QString &text);
};

#endif

