/*
    SPDX-FileCopyrightText: 2018 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testlistpopupmenu.h"

#include "associationwidgetpopupmenu.h"
#include "dialogspopupmenu.h"
#include "widgetbasepopupmenu.h"
#include "umllistviewpopupmenu.h"
#include "umlscenepopupmenu.h"

#include "category.h"
#include "entity.h"
#include "classifier.h"
#include "instance.h"

#include "associationwidget.h"
#include "categorywidget.h"
#include "classifierwidget.h"
#include "entitywidget.h"

#include "folder.h"
#include "umllistview.h"
#include "umlscene.h"
#include "umlview.h"

void TestListPopupMenu::test_createWidgetsSingleSelect()
{
    UMLFolder folder(QStringLiteral("test"));
    UMLView view(&folder);
    UMLScene scene(&folder, &view);
    QWidget qWidget;
    for(int i = WidgetBase::wt_Min+1; i < WidgetBase::wt_Max; i++) {
        WidgetBase::WidgetType type = static_cast<WidgetBase::WidgetType>(i);
        WidgetBase  *widget = nullptr;
        if (type == WidgetBase::wt_Entity) {
            widget = new EntityWidget(&scene, new UMLEntity(QStringLiteral("entity")));
        } else if (type == WidgetBase::wt_Category) {
            widget = new CategoryWidget(&scene, new UMLCategory(QStringLiteral("category")));
        } else if (type == WidgetBase::wt_Class) {
            widget = new ClassifierWidget(&scene, new UMLClassifier(QStringLiteral("classifier")));
        } else if (type == WidgetBase::wt_Interface) {
            widget = new ClassifierWidget(&scene, new UMLClassifier(QStringLiteral("instance")));
            widget->setBaseType(type);
        } else
            widget = new WidgetBase(&scene, type);
        WidgetBasePopupMenu popup(&qWidget, widget, false);
        popup.dumpActions(WidgetBase::toString(type));
    }
}

void TestListPopupMenu::test_createWidgetsMultiSelect()
{
    UMLFolder folder(QStringLiteral("test"));
    UMLView view(&folder);
    UMLScene scene(&folder, &view);
    QWidget qWidget;
    for(int i = WidgetBase::wt_Min+1; i < WidgetBase::wt_Max; i++) {
        WidgetBase::WidgetType type = static_cast<WidgetBase::WidgetType>(i);
        WidgetBase  *widget = nullptr;
        if (type == WidgetBase::wt_Entity) {
            widget = new EntityWidget(&scene, new UMLEntity(QStringLiteral("entity")));
        } else if (type == WidgetBase::wt_Category) {
            widget = new CategoryWidget(&scene, new UMLCategory(QStringLiteral("category")));
        } else if (type == WidgetBase::wt_Class) {
            widget = new ClassifierWidget(&scene, new UMLClassifier(QStringLiteral("classifier")));
        } else if (type == WidgetBase::wt_Interface) {
            widget = new ClassifierWidget(&scene, new UMLClassifier(QStringLiteral("instance")));
            widget->setBaseType(type);
        } else
            widget = new WidgetBase(&scene, type);
        WidgetBasePopupMenu popupMulti(&qWidget, widget, true, type);
        popupMulti.dumpActions(WidgetBase::toString(type));
    }
}

void TestListPopupMenu::test_createAssociationWidget()
{
    UMLFolder folder(QStringLiteral("test"));
    UMLView view(&folder);
    UMLScene scene(&folder, &view);
    QWidget qWidget;
    UMLWidget widgetA(&scene, WidgetBase::wt_UMLWidget, nullptr), widgetB(&scene, WidgetBase::wt_UMLWidget, nullptr);
    for (int i = Uml::AssociationType::Generalization; i < Uml::AssociationType::Reserved; ++i) {
        Uml::AssociationType::Enum type = Uml::AssociationType::fromInt(i);
        AssociationWidget *widget = AssociationWidget::create(&scene, &widgetA, type, &widgetB);
        AssociationWidgetPopupMenu popup(&qWidget,type, widget);
        popup.dumpActions(Uml::AssociationType::toString(type));
    }
}

void TestListPopupMenu::test_createUMLScene()
{
    UMLFolder folder(QStringLiteral("test"));
    UMLView view(&folder);
    QWidget qWidget;
    for(int i = Uml::DiagramType::Undefined+1; i < Uml::DiagramType::N_DIAGRAMTYPES; i++) {
        Uml::DiagramType::Enum type = Uml::DiagramType::fromInt(i);
        view.umlScene()->setType(type);
        UMLScenePopupMenu popup(&qWidget, view.umlScene());
        popup.dumpActions(Uml::DiagramType::toString(type));
    }
}

void TestListPopupMenu::test_createUMLListview()
{
    QWidget qWidget;
    UMLObject object;
    UMLCategory category;
    UMLListView view;
    for(int i = UMLListViewItem::ListViewType::lvt_Min+1; i < UMLListViewItem::ListViewType::lvt_Max; i++) {
        UMLListViewItem::ListViewType type = static_cast<UMLListViewItem::ListViewType>(i);
        UMLListViewItem item(&view, QStringLiteral("test"), type);
        item.setUMLObject(type == UMLListViewItem::ListViewType::lvt_Category ? &category : &object);
        UMLListViewPopupMenu popup(&qWidget, &item);
        popup.dumpActions(UMLListViewItem::toString(type));
    }
}

void TestListPopupMenu::test_createMiscMenu()
{
    QWidget qWidget;
    for(int i = DialogsPopupMenu::TriggerType::tt_Min+1; i < DialogsPopupMenu::TriggerType::tt_Max; i++) {
        DialogsPopupMenu::TriggerType tt = static_cast<DialogsPopupMenu::TriggerType>(i);
        DialogsPopupMenu popup(&qWidget, tt);
        popup.dumpActions(DialogsPopupMenu::toString(tt));
    }
}

QTEST_MAIN(TestListPopupMenu)
