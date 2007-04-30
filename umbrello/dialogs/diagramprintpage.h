/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DIAGRAMPRINTPAGE_H
#define DIAGRAMPRINTPAGE_H

class QListBox;
class QRadioButton;
class QComboBox;
class QButtonGroup;
class QGroupBox;

#include <kdeprint/kprintdialogpage.h>

#include "../umldoc.h"
#include "../umlview.h"
/**
 * This is a page on the print dialog to select what diagram(s)
 * you wish to print.  You add it to the @ref KPrinter instance.
 *
 * You will then need to get the options as shown in @ref KPrinter.
 *
 * @short  A print dialog page.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see    KPrinter
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class DiagramPrintPage : public KPrintDialogPage {
    Q_OBJECT
public:
    /**
     *  Constructs the diagram print page.
     *
     *  @param parent The parent to the page.
     *  @param doc      The @ref UMLDoc class instance being used.
     */
    DiagramPrintPage(QWidget * parent, UMLDoc *doc);

    /**
     *  Stnadard deconstructor.
     */
    ~DiagramPrintPage();

    /**
     * Overriden method. It builds a list with the diagram name and the diagram
     * IDs.
     */
    void getOptions(QMap<QString,QString>& opts, bool /* incldef=false */);

    /**
     * Overriden method.
     */
    void setOptions( const QMap<QString,QString>& /*opts*/ );

    /**
     * Overriden method.
     */
    bool isValid( QString& msg );

private:
    QButtonGroup * m_pFilterBG;
    QGroupBox * m_pSelectGB;
    QListBox * m_pSelectLB;
    QRadioButton * m_pAllRB, * m_pCurrentRB, * m_pSelectRB, * m_pTypeRB;
    QComboBox * m_pTypeCB;

    UMLDoc * m_pDoc;
    Uml::Diagram_Type m_ViewType;

    /**
     * list containing the IDs of diagrams to print
     */
    QValueList<Uml::IDType> m_nIdList;

    enum FilterType{Current = 0, All, Select, Type};
public slots:

    /**
     * Gets called when the users chooses to print all diagrams, the current
     * diagram, a selection of diagrams or diagrams by type. It will change the
     * listed diagrams in the diagram box.
     */
    void slotClicked(int id);

    /**
     * Gets called when the user chooses another diagram type. Only diagrams of
     * this type will be shown in the diagram box.
     */
    void slotActivated(const QString & text);
};

#endif
