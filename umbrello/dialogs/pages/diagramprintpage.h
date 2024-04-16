/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DIAGRAMPRINTPAGE_H
#define DIAGRAMPRINTPAGE_H

#include "basictypes.h"
#include "dialogpagebase.h"
#include "umlview.h"

#include <QList>

class QListWidget;
class QRadioButton;
class QGroupBox;
class KComboBox;
class UMLDoc;

/**
 * This is a page on the print dialog to select what diagram(s)
 * you wish to print.  You add it to the QPrinter instance.
 *
 * You will then need to get the options as shown in QPrinter.
 *
 * @short  A print dialog page.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see    QPrinter
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class DiagramPrintPage : public DialogPageBase
{
    Q_OBJECT
public:
    DiagramPrintPage(QWidget * parent, UMLDoc *doc);
    ~DiagramPrintPage();

    int printUmlCount();
    QString printUmlDiagram(int sel);

    bool isValid(QString& msg);

private:
    bool isSelected(int index);

    QGroupBox * m_pFilterGB;
    QGroupBox * m_pSelectGB;
    QListWidget * m_pSelectLW;
    QRadioButton * m_pAllRB;
    QRadioButton * m_pCurrentRB;
    QRadioButton * m_pSelectRB;
    QRadioButton * m_pTypeRB;
    KComboBox * m_pTypeCB;

    UMLDoc * m_doc;
    Uml::DiagramType::Enum m_ViewType;

    QList<Uml::ID::Type> m_nIdList;  ///< list containing the IDs of diagrams to print

    enum FilterType{Current = 0, All, Select, Type};

public Q_SLOTS:
    void slotClicked();
    void slotActivated(int index);
};

#endif
