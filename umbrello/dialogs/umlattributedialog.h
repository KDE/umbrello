/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLATTRIBUTEDIALOG_H
#define UMLATTRIBUTEDIALOG_H

#include "singlepagedialogbase.h"

class DocumentationWidget;
class QCheckBox;
class QGroupBox;
class QRadioButton;
class QLabel;
class UMLAttribute;
class KLineEdit;
class UMLDatatypeWidget;
class UMLStereotypeWidget;
class VisibilityEnumWidget;

/**
 * @author Paul Hensgen
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLAttributeDialog : public SinglePageDialogBase
{
    Q_OBJECT
public:
    UMLAttributeDialog(QWidget * pParent, UMLAttribute * pAttribute);
    ~UMLAttributeDialog();

protected:
    void setupDialog();
    bool apply();

    /**
     *   The Attribute to represent
     */
    UMLAttribute * m_pAttribute;

    //GUI Widgets
    QGroupBox * m_pValuesGB;
    QLabel * m_pNameL, * m_pInitialL;
    KLineEdit * m_pNameLE, * m_pInitialLE;
    QCheckBox* m_pStaticCB;
    UMLDatatypeWidget *m_datatypeWidget;
    UMLStereotypeWidget *m_stereotypeWidget;
    VisibilityEnumWidget *m_visibilityEnumWidget;
    DocumentationWidget *m_docWidget;

public slots:
    void slotNameChanged(const QString &);
};

#endif
