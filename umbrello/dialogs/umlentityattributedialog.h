/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLENTITYATTRIBUTEDIALOG_H
#define UMLENTITYATTRIBUTEDIALOG_H

#include "singlepagedialogbase.h"
#include "ui_umlentityattributedialog.h"

class QCheckBox;
class QGroupBox;
class QLabel;
class QRadioButton;
class UMLDatatypeWidget;
class UMLEntityAttribute;
class UMLStereotypeWidget;
class KComboBox;
class KLineEdit;

/**
 * @author Jonathan Riddell
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class UMLEntityAttributeDialog : public SinglePageDialogBase
{
    Q_OBJECT
public:
    UMLEntityAttributeDialog(QWidget* pParent, UMLEntityAttribute* pEntityAttribute);
    ~UMLEntityAttributeDialog();

private:
    Ui::UMLEntityAttributeDialog *ui;

protected:

    void setupDialog();

    virtual bool apply();

    void insertAttribute(const QString& type, int index = -1);

    /**
     * The EntityAttribute to represent.
     */
    UMLEntityAttribute * m_pEntityAttribute;

public slots:
    void slotAutoIncrementStateChanged(bool checked);
    void slotNameChanged(const QString &);
};

#endif
