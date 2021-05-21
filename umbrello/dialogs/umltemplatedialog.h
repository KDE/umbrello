/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLTEMPLATEDIALOG_H
#define UMLTEMPLATEDIALOG_H

#include "singlepagedialogbase.h"

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
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLTemplateDialog : public SinglePageDialogBase
{
    Q_OBJECT
public:
    UMLTemplateDialog(QWidget* pParent, UMLTemplate* pAttribute);
    ~UMLTemplateDialog();

protected:

    void setupDialog();

    virtual bool apply();

    /**
     *   The Attribute to represent
     */
    UMLTemplate* m_pTemplate;

    // GUI Widgets
    QGroupBox *m_pValuesGB;
    QLabel *m_pNameL;
    KLineEdit *m_pNameLE;
    UMLDatatypeWidget *m_datatypeWidget;
    UMLStereotypeWidget *m_stereotypeWidget;
    DocumentationWidget *m_docWidget;

protected:

    void insertTypesSorted(const QString& type = QString());

};

#endif
