/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLDRAGDATA_H
#define UMLDRAGDATA_H

#include "associationwidgetlist.h"
#include "basictypes.h"
#include "umllistviewitem.h"
#include "umllistviewitemlist.h"
#include "umlobjectlist.h"
#include "umlviewlist.h"
#include "umlwidgetlist.h"

#include <QList>
#include <QMimeData>

class UMLClassifier;
class UMLListView;
class UMLScene;
class QPixmap;

/**
 * This class provides encoding and decoding for the uml data that will be used
 * in a drag and drop operation or in a copy or paste operation.
 *
 * @author Gustavo Madrigal, Jonathan Riddell (XMI conversion)
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLDragData : public QMimeData 
{
    Q_OBJECT
public:

    explicit UMLDragData(UMLObjectList& Objects, QWidget *dragSource = nullptr);

    UMLDragData(UMLObjectList &Objects, UMLViewList& Diagrams, QWidget  *dragSource = nullptr);

    explicit UMLDragData(UMLListViewItemList& UMLListViewItems, QWidget *dragSource = nullptr);

    UMLDragData(UMLObjectList& Objects, UMLWidgetList& Widgets, AssociationWidgetList& Associations,
            QPixmap& PngImage, UMLScene  *scene, QWidget* dragSource = nullptr);

    UMLDragData(UMLObjectList& Objects, int, QWidget *dragSource = nullptr);

    explicit UMLDragData(QWidget *dragSource = nullptr);

    ~UMLDragData();

    static bool decodeClip1(const QMimeData* mimeData, UMLObjectList& objects);

    static bool decodeClip2(const QMimeData* mimeData, UMLObjectList& objects, UMLViewList& diagrams);

    static bool decodeClip3(const QMimeData* mimeData,
                                  UMLListViewItemList& umlListViewItems,
                                  const UMLListView *parentListView= nullptr);

    struct LvTypeAndID {
        UMLListViewItem::ListViewType type;
        Uml::ID::Type id;
    };
    typedef QList<LvTypeAndID*> LvTypeAndID_List;
    typedef QListIterator<LvTypeAndID*> LvTypeAndID_It;

    static bool getClip3TypeAndID(const QMimeData* mimeData,
                                  LvTypeAndID_List& typeAndIdList);

    static bool decodeClip4(const QMimeData* mimeData, UMLObjectList& objects,
                            UMLWidgetList& widgets,
                            AssociationWidgetList& associations,
                            Uml::DiagramType::Enum & dType);

    static bool decodeClip5(const QMimeData* mimeData, UMLObjectList& objects,
                            UMLClassifier *newParent);

    static int getCodingType(const QMimeData* mimeData);

 private:

    void setUMLDataClip1(UMLObjectList& Objects);

    void setUMLDataClip2(UMLObjectList& Objects, UMLViewList& Diagrams);

    void setUMLDataClip3(UMLListViewItemList& UMLListViewItems);

    void setUMLDataClip4(UMLObjectList& Objects,
                         UMLWidgetList& WidgetDatas,
                         AssociationWidgetList& Associations, QPixmap& PngImage,
                         UMLScene *scene);

    void setUMLDataClip5(UMLObjectList& Objects);

    static void executeCreateWidgetCommand(UMLWidget* widget);

    static bool decodeObjects(QDomNode& objectsNode, UMLObjectList& objects,
                              bool skipIfObjectExists = false);

    static bool decodeViews(QDomNode& umlviewsNode, UMLViewList& diagrams);
};

#endif
