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

#ifndef UMLDRAG_H
#define UMLDRAG_H

#include <qdragobject.h>
#include <qobject.h>
#include <qptrlist.h>

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
class UMLDragPrivate;
class UMLClassifier;

class Q_EXPORT UMLDrag : public QDragObject  {
    Q_OBJECT
    UMLDragPrivate* data;
public:

    /**
     * For use when the user selects only UML Objects from
     * the ListView but no diagrams to be copied, Mime type =
     * "application/x-uml-clip1
     */
    explicit UMLDrag(UMLObjectList& Objects,QWidget* dragSource = 0, const char* name = 0 );

    /**
     * For use when the user selects UML Object and Diagrams
     * from the ListView to be copied, Mime type =
     * "application/x-uml-clip2
     */
    UMLDrag(UMLObjectList &Objects, UMLListViewItemList& UMLListViewItems,
            UMLViewList& Diagrams, QWidget * dragSource = 0, const char * name = 0 );

    /**
     * For use when the user selects only empty folders from
     * the ListView to be copied, Mime type =
     * "application/x-uml-clip3
     */
    explicit UMLDrag(UMLListViewItemList& UMLListViewItems, QWidget* dragSource = 0,
            const char* name = 0 );

    /*
     * For use when the user selects UMLObjects from a
     * Diagram. The Selected widegets and the relationships *
     * between only selected widgets will be copied and also
     * its respective ListView Items, Mime type =
     * "application/x-uml-clip4
     */
    UMLDrag(UMLObjectList& Objects, UMLWidgetList& Widgets, AssociationWidgetList& Associations,
            QPixmap& PngImage, Uml::Diagram_Type dType, QWidget* dragSource = 0,
            const char* name = 0 );

    /**
     * For use when the user selects only Operations and/or
     * Attributes from the ListView, Mime type =
     * "application/x-uml-clip5
     */
    UMLDrag(UMLObjectList& Objects, int, QWidget* dragSource = 0, const char* name = 0);

    /**
     *  Constructor
     */
    explicit UMLDrag(QWidget* dragSource = 0, const char* name = 0);

    /**
     *  Deconstructor
     */
    ~UMLDrag();

    /**
     * For use when the user selects only UMLObjects from the
     * ListView but no diagrams to be copied
     */
    void setUMLDataClip1(UMLObjectList& Objects);

    /**
     * For use when the user selects UML Object and Diagrams
     * from the ListView to be copied
     */
    void setUMLDataClip2(UMLObjectList& Objects, UMLListViewItemList& UMLListViewItems,
                         UMLViewList& Diagrams);

    /**
     * For use when the user selects only empty folders from the ListView
     * to be copied.
     */
    void setUMLDataClip3(UMLListViewItemList& UMLListViewItems);

    /**
     * For use when the user selects UML Objects from a
     * Diagram. The Selected widegets and the relationships
     * between only selected widgets will be copied and also
     * its respective ListView Items
     */
    void setUMLDataClip4(UMLObjectList& Objects,
                         UMLWidgetList& WidgetDatas,
                         AssociationWidgetList& Associations, QPixmap& PngImage,
                         Uml::Diagram_Type dType);

    /**
     * For use when the user selects only Attirbutes and/or
     * Operation from the ListView
     */
    void setUMLDataClip5(UMLObjectList& Objects);

    /**
     * Sets the type of the clip to "application/x-uml-" + sub
     *  sub should be clip[1-5]
     */
    virtual void setSubType(const QCString& sub, int index);

    /**
     * Sets the data in the clip
     */
    virtual void setEncodedData(const QByteArray&, int index);

    /**
     * Returns the type set by setSubType
     */
    const char* format(int index) const;

    /**
     * Returns the encoded data of the given type
     *
     * @param dataName the name of the data type to return
     */
    virtual QByteArray encodedData(const char* dataName) const;

    /**
     * For use when the user selects only UML Objects
     * from the ListView but no diagrams to be
     * copied, decodes Mime type =
     * "application/x-uml-clip1
     */
    static bool decodeClip1(const QMimeSource* mimeSource, UMLObjectList& objects);

    /**
     * For use when the user selects UML Object and Diagrams
     * from the ListView to be copied, decodes Mime type =
     * "application/x-uml-clip2
     */
    static bool decodeClip2(const QMimeSource* mimeSource, UMLObjectList& objects,
                            UMLListViewItemList& umlListViewItems,
                            UMLViewList& diagrams);

    /**
     * For use when the user selects UMLObjects from
     * the ListView to be copied, decodes Mime * type =
     * "application/x-uml-clip3
     */
    static bool decodeClip3(const QMimeSource* mimeSource,
                            UMLListViewItemList& umlListViewItems,
                            const UMLListView* parentListView=0);

    struct LvTypeAndID {
        Uml::ListView_Type type;
        Uml::IDType id;
    };
    typedef QPtrList<LvTypeAndID> LvTypeAndID_List;
    typedef QPtrListIterator<LvTypeAndID> LvTypeAndID_It;

    /**
     * Return just the LvTypeAndID of a Clip3.
     *
     * @param mimeSource        The encoded source.
     * @param typeAndIdList     The LvTypeAndID_List decoded from the source.
     * @return  True if decoding was successful.
     */
    static bool getClip3TypeAndID(const QMimeSource* mimeSource,
                                  LvTypeAndID_List& typeAndIdList);

    /**
     * For use when the user selects UML Objects from a
     * Diagram. The Selected widegets and the relationships
     * between only * selected widgets will be copied and
     * also its respective ListView Items, * decodes Mime
     * type = "application/x-uml-clip4
     */
    static bool decodeClip4(const QMimeSource* mimeSource, UMLObjectList& objects,
                            UMLWidgetList& widgets,
                            AssociationWidgetList& associations,
                            Uml::Diagram_Type & dType);

    /**
     * For use when the user selects only Attributes and/or
     * Operations from the ListView * copied, decodes Mime
     * type = "application/x-uml-clip5
     */
    static bool decodeClip5(const QMimeSource* mimeSource, UMLObjectList& objects,
                            UMLClassifier *newParent);

    /**
     * Converts application/x-uml-clip[1-5] clip type to an integer
     */
    static int getCodingType(const QMimeSource* mimeSource);

};

#endif
