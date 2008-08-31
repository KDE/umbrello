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

#ifndef UMLDRAGDATA_H
#define UMLDRAGDATA_H

#include <qobject.h>
#include <q3ptrlist.h>
#include <qmimedata.h>
//Added by qt3to4:
#include <QPixmap>

#include "../umllistviewitemlist.h"
#include "../associationwidgetlist.h"
#include "../umlobjectlist.h"
#include "../umlviewlist.h"
#include "../umlwidgetlist.h"
#include "../umlnamespace.h"

/**
 * This class provides encoding and decoding for the uml data that will be used
 * in a drag and drop operation or in a copy or paste operation.
 *
 * @author Gustavo Madrigal, Jonathan Riddell (XMI conversion)
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLListView;
class UMLClassifier;

class UMLDragData : public QMimeData  {
    Q_OBJECT
public:

    explicit UMLDragData(UMLObjectList& Objects, QWidget* dragSource = 0);

    /**
     * For use when the user selects UML Object and Diagrams
     * from the ListView to be copied, Mime type =
     * "application/x-uml-clip2
     */
    UMLDragData(UMLObjectList &Objects, UMLListViewItemList& UMLListViewItems,
                UMLViewList& Diagrams, QWidget * dragSource = 0);

    explicit UMLDragData(UMLListViewItemList& UMLListViewItems, QWidget* dragSource = 0);

    /*
     * For use when the user selects UMLObjects from a
     * Diagram. The Selected widegets and the relationships *
     * between only selected widgets will be copied and also
     * its respective ListView Items, Mime type =
     * "application/x-uml-clip4
     */
    UMLDragData(UMLObjectList& Objects, UMLWidgetList& Widgets, AssociationWidgetList& Associations,
            QPixmap& PngImage, Uml::Diagram_Type dType, QWidget* dragSource = 0);

    /**
     * For use when the user selects only Operations and/or
     * Attributes from the ListView, Mime type =
     * "application/x-uml-clip5
     */
    UMLDragData(UMLObjectList& Objects, int, QWidget* dragSource = 0);

    explicit UMLDragData(QWidget* dragSource = 0);

    /**
     *  Deconstructor
     */
    ~UMLDragData();

    static bool decodeClip1(const QMimeData* mimeData, UMLObjectList& objects);

    static bool decodeClip2(const QMimeData* mimeData, UMLObjectList& objects,
                            UMLListViewItemList& umlListViewItems,
                            UMLViewList& diagrams);

    static bool decodeClip3(const QMimeData* mimeData,
                            UMLListViewItemList& umlListViewItems,
                            const UMLListView* parentListView=0);

    struct LvTypeAndID {
        Uml::ListView_Type type;
        Uml::IDType id;
    };
    typedef Q3PtrList<LvTypeAndID> LvTypeAndID_List;
    typedef Q3PtrListIterator<LvTypeAndID> LvTypeAndID_It;

    static bool getClip3TypeAndID(const QMimeData* mimeData,
                                  LvTypeAndID_List& typeAndIdList);

    static bool decodeClip4(const QMimeData* mimeData, UMLObjectList& objects,
                            UMLWidgetList& widgets,
                            AssociationWidgetList& associations,
                            Uml::Diagram_Type & dType);

    static bool decodeClip5(const QMimeData* mimeData, UMLObjectList& objects,
                            UMLClassifier *newParent);

    static int getCodingType(const QMimeData* mimeData);

 private:

    void setUMLDataClip1(UMLObjectList& Objects);

    void setUMLDataClip2(UMLObjectList& Objects, UMLListViewItemList& UMLListViewItems,
                         UMLViewList& Diagrams);

    void setUMLDataClip3(UMLListViewItemList& UMLListViewItems);

    void setUMLDataClip4(UMLObjectList& Objects,
                         UMLWidgetList& WidgetDatas,
                         AssociationWidgetList& Associations, QPixmap& PngImage,
                         Uml::Diagram_Type dType);

    void setUMLDataClip5(UMLObjectList& Objects);

};

#endif
