/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DIAGRAMPRINTPAGE_H
#define DIAGRAMPRINTPAGE_H

#include "basictypes.h"
#include "umlview.h"

#include <QtCore/QList>

class QListWidget;
class QRadioButton;
class QGroupBox;
class KComboBox;
class UMLDoc;

/**
 * This is a page on the print dialog to select what diagram(s)
 * you wish to print.  You add it to the @ref QPrinter instance.
 *
 * You will then need to get the options as shown in @ref QPrinter.
 *
 * @short  A print dialog page.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see    KPrinter
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class DiagramPrintPage : public QWidget
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
    Uml::DiagramType m_ViewType;

    QList<Uml::IDType> m_nIdList;  //< list containing the IDs of diagrams to print

    enum FilterType{Current = 0, All, Select, Type};

public slots:
    void slotClicked();
    void slotActivated(int index);
};

#endif
