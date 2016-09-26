/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLTEMPLATEDIALOG_H
#define UMLTEMPLATEDIALOG_H

#include "singlepagedialogbase.h"
#include "ui_umltemplatedialog.h"

class DocumentationWidget;
class KComboBox;
class KLineEdit;
class QGroupBox;
class QLabel;
class UMLTemplate;
class UMLDatatypeWidget;
class UMLStereotypeWidget;

/**
 * A dialog to edit the properties of a class template (paramaterised class)
 *
 * @author Jonathan Riddell
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class UMLTemplateDialog : public SinglePageDialogBase
{
    Q_OBJECT
public:
    UMLTemplateDialog(QWidget* pParent, UMLTemplate* pAttribute);
    ~UMLTemplateDialog();
private:
    Ui::UMLTemplateDialog *ui;

protected:

    void setupDialog();

    virtual bool apply();

    /**
     *   The Attribute to represent
     */
    UMLTemplate* m_pTemplate;

protected:

    void insertTypesSorted(const QString& type = QString());

};

#endif
