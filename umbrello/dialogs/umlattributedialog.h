/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLATTRIBUTEDIALOG_H
#define UMLATTRIBUTEDIALOG_H

#include "singlepagedialogbase.h"
#include "basictypes.h"
#include "ui_umlattributedialog.h"

/**
 * @author Paul Hensgen
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class UMLAttributeDialog : public SinglePageDialogBase
{
    Q_OBJECT
public:
    UMLAttributeDialog(QWidget * pParent, UMLAttribute * pAttribute);
    ~UMLAttributeDialog();

private:
    Ui::UMLAttributeDialog *ui;
    Uml::ProgrammingLanguage::Enum activeLanguage;

protected:
    void setupDialog();
    bool apply();

    /**
     *   The Attribute to represent
     */
    UMLAttribute * m_pAttribute;

public slots:
    void slotNameChanged(const QString &);

};

#endif
