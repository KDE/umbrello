/*
    Copyright 2018  Ralf Habacker  <ralf.habacker@freenet.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "testlistpopupmenu.h"

#include "associationwidgetpopupmenu.h"
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
    UMLFolder folder(QLatin1String("test"));
    UMLView view(&folder);
    UMLScene scene(&folder, &view);
    QWidget qWidget;
    for(int i = WidgetBase::wt_Min+1; i < WidgetBase::wt_Max; i++) {
        WidgetBase::WidgetType type = static_cast<WidgetBase::WidgetType>(i);
        WidgetBase *widget = 0;
        if (type == WidgetBase::wt_Entity) {
            widget = new EntityWidget(&scene, new UMLEntity("entity"));
        } else if (type == WidgetBase::wt_Category) {
            widget = new CategoryWidget(&scene, new UMLCategory("category"));
        } else if (type == WidgetBase::wt_Class) {
            widget = new ClassifierWidget(&scene, new UMLClassifier("classifier"));
        } else if (type == WidgetBase::wt_Interface) {
            widget = new ClassifierWidget(&scene, new UMLClassifier("instance"));
            widget->setBaseType(type);
        } else
            widget = new WidgetBase(&scene, type);
        WidgetBasePopupMenu popup(&qWidget, widget, false);
        popup.dumpActions(WidgetBase::toString(type));
    }
}

void TestListPopupMenu::test_createWidgetsMultiSelect()
{
    UMLFolder folder(QLatin1String("test"));
    UMLView view(&folder);
    UMLScene scene(&folder, &view);
    QWidget qWidget;
    for(int i = WidgetBase::wt_Min+1; i < WidgetBase::wt_Max; i++) {
        WidgetBase::WidgetType type = static_cast<WidgetBase::WidgetType>(i);
        WidgetBase *widget = 0;
        if (type == WidgetBase::wt_Entity) {
            widget = new EntityWidget(&scene, new UMLEntity("entity"));
        } else if (type == WidgetBase::wt_Category) {
            widget = new CategoryWidget(&scene, new UMLCategory("category"));
        } else if (type == WidgetBase::wt_Class) {
            widget = new ClassifierWidget(&scene, new UMLClassifier("classifier"));
        } else if (type == WidgetBase::wt_Interface) {
            widget = new ClassifierWidget(&scene, new UMLClassifier("instance"));
            widget->setBaseType(type);
        } else
            widget = new WidgetBase(&scene, type);
        WidgetBasePopupMenu popupMulti(&qWidget, widget, true, type);
        popupMulti.dumpActions(WidgetBase::toString(type));
    }
}

void TestListPopupMenu::test_createAssociationWidget()
{
    UMLFolder folder(QLatin1String("test"));
    UMLView view(&folder);
    UMLScene scene(&folder, &view);
    QWidget qWidget;
    UMLWidget widgetA(&scene, WidgetBase::wt_UMLWidget, 0), widgetB(&scene, WidgetBase::wt_UMLWidget, 0);
    for (int i = Uml::AssociationType::Generalization; i < Uml::AssociationType::Reserved; ++i) {
        Uml::AssociationType::Enum type = Uml::AssociationType::fromInt(i);
        AssociationWidget *widget = AssociationWidget::create(&scene, &widgetA, type, &widgetB);
        AssociationWidgetPopupMenu popup(&qWidget,type, widget);
        popup.dumpActions(Uml::AssociationType::toString(type));
    }
}

void TestListPopupMenu::test_createUMLScene()
{
    UMLFolder folder(QLatin1String("test"));
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
        UMLListViewItem item(&view, QLatin1String("test"), type);
        item.setUMLObject(type == UMLListViewItem::ListViewType::lvt_Category ? &category : &object);
        UMLListViewPopupMenu popup(&qWidget, &item);
        popup.dumpActions(UMLListViewItem::toString(type));
    }
}

void TestListPopupMenu::test_createMiscMenu()
{
    QWidget qWidget;
    for(int i = ListPopupMenu::MenuType::mt_Model; i < ListPopupMenu::MenuType::mt_Max; i++) {
        ListPopupMenu::MenuType mt = static_cast<ListPopupMenu::MenuType>(i);
        if (mt == ListPopupMenu::MenuType::mt_Category) {
            qDebug().nospace() << ListPopupMenu::toString(mt);
        } else {
            ListPopupMenu popup(&qWidget, mt);
            popup.dumpActions(ListPopupMenu::toString(mt));
        }
    }
}

QTEST_MAIN(TestListPopupMenu)
