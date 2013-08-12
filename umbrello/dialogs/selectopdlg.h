/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef SELECTOPDLG_H
#define SELECTOPDLG_H

#include <kdialog.h>

class KComboBox;
class KLineEdit;
class QGroupBox;
class QLabel;
class QRadioButton;
class UMLClassifier;
class UMLView;

/**
 * A dialog used to select an operation.
 *
 * @short A dialog to select an operation.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class SelectOpDlg : public KDialog
{
    Q_OBJECT
public:
    SelectOpDlg(QWidget * parent, UMLClassifier * c);
    ~SelectOpDlg();

    QString getOpText();

    bool isClassOp() const;

    QString getSeqNumber();

    void setSeqNumber(const QString &num);

    void setCustomOp(const QString &op);

    bool setClassOp(const QString &op);

protected:

    void insertOperation( const QString& type, int index = -1 );

private:
    static const int OP     = 0;
    static const int CUSTOM = 1;

    QGroupBox * m_pOpGB;
    KComboBox * m_pOpCB;
    QLabel * m_pSeqL;
    KLineEdit * m_pOpLE, * m_pSeqLE;
    QRadioButton * m_pCustomRB, * m_pOpRB;
    QString m_Text;
    int m_nOpCount;
    int m_id;  ///< takes on the value OP or CUSTOM according to what the user selected
    UMLView *m_pView;

public slots:
    void slotSelectedOp();
    void slotSelectedCustom();
};

#endif

