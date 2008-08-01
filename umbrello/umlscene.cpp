/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlscene.h"

// system includes
#include <climits>
#include <math.h>

// include files for Qt
#include <QPainter>
#include <QPrinter>
#include <QPixmap>
#include <QGraphicsSceneMouseEvent>

//kde include files
#include <ktemporaryfile.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kdebug.h>

// application specific includes
#include "umlviewimageexporter.h"
#include "listpopupmenu.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "docwindow.h"
#include "assocrules.h"
#include "umlrole.h"
#include "dialogs/classoptionspage.h"
#include "dialogs/umlviewdialog.h"
#include "clipboard/idchangelog.h"
#include "clipboard/umldragdata.h"
#include "widget_factory.h"
#include "floatingtextwidget.h"
#include "classifierwidget.h"
#include "classifier.h"
#include "packagewidget.h"
#include "package.h"
#include "folder.h"
#include "componentwidget.h"
#include "nodewidget.h"
#include "artifactwidget.h"
#include "datatypewidget.h"
#include "enumwidget.h"
#include "entitywidget.h"
#include "actorwidget.h"
#include "usecasewidget.h"
#include "notewidget.h"
#include "boxwidget.h"
#include "associationwidget.h"
#include "objectwidget.h"
#include "messagewidget.h"
#include "statewidget.h"
#include "signalwidget.h"
#include "forkjoinwidget.h"
#include "activitywidget.h"
#include "objectnodewidget.h"
#include "pinwidget.h"
#include "seqlinewidget.h"
#include "uniqueid.h"
#include "umllistviewitemlist.h"
#include "umllistviewitem.h"
#include "umllistview.h"
#include "umlobjectlist.h"
#include "association.h"
#include "attribute.h"
#include "model_utils.h"
#include "object_factory.h"
#include "umlwidget.h"
#include "toolbarstatefactory.h"
#include "cmds.h"
#include "entity.h"
#include "foreignkeyconstraint.h"

#define TEST 1
#include "test.h"

// static members
const qreal UMLScene::defaultCanvasSize = 1300;

using namespace Uml;

// constructor
UMLScene::UMLScene(UMLFolder *parentFolder) :
    QGraphicsScene(0, 0, defaultCanvasSize, defaultCanvasSize)
{
    // Initialize loaded/saved data
    m_nID = Uml::id_None;
    m_Documentation = "";
    m_Type = dt_Undefined;
    m_bUseSnapToGrid = false;
    m_bUseSnapComponentSizeToGrid = false;
    m_bShowSnapGrid = false;
    m_nSnapX = 10;
    m_nSnapY = 10;
    m_nCollaborationId = 0;

    m_isMouseMovingItems = false;

    // Initialize other data
    //m_AssociationList.setAutoDelete(true);
    //m_WidgetList.setAutoDelete(true);
    //m_MessageList.setAutoDelete(true);

    //Setup up booleans
    m_bChildDisplayedDoc = false;
    m_bPaste = false;
    m_bActivated = false;
    m_bCreateObject = false;
    m_bDrawSelectedOnly = false;
    m_bPopupShowing = false;
    m_bStartedCut = false;
    //clear pointers
    m_pIDChangesLog = 0;
    m_pMenu = 0;

    // [PORT] Port ImageExporter properly
    m_pImageExporter = new UMLViewImageExporter(this->activeView());

    //setup signals
    connect(this, SIGNAL(sigRemovePopupMenu()), this, SLOT(slotRemovePopupMenu()));
    connect(UMLApp::app(), SIGNAL(sigCutSuccessful()),
            this, SLOT(slotCutSuccessful()));

    // Create the ToolBarState factory. This class is not a singleton, because it
    // needs a pointer to this object.
    m_pToolBarStateFactory = new ToolBarStateFactory();
    m_pToolBarState = m_pToolBarStateFactory->getState(WorkToolBar::tbb_Arrow, this);
    m_pDoc = UMLApp::app()->getDocument();
    m_pFolder = parentFolder;

#ifdef TEST
    /*
     * Test code below.
     */
    Test *test = Test::self();
    test->testScene(this);
#endif
}

UMLScene::~UMLScene()
{
    delete m_pImageExporter;

    delete    m_pIDChangesLog;
    m_pIDChangesLog = 0;

    // before we can delete the QCanvas, all widgets must be explicitly
    // removed
    // otherwise the implicit remove of the contained widgets will cause
    // events which would demand a valid connected QCanvas
    // ==> this causes umbrello to crash for some - larger?? - projects
    // first avoid all events, which would cause some update actions
    // on deletion of each removed widget
    blockSignals(true);
    removeAllWidgets();

    delete m_pToolBarStateFactory;
    m_pToolBarStateFactory = NULL;

}

QString UMLScene::getName() const
{
    return m_Name;
}

void UMLScene::setName(const QString &name)
{
    m_Name = name;
}

int UMLScene::generateCollaborationId()
{
    return ++m_nCollaborationId;
}

void UMLScene::print(QPrinter *pPrinter, QPainter & pPainter)
{
    int height, width;
    //get the size of the page
    pPrinter->setFullPage(true);

    QFontMetrics fm = pPainter.fontMetrics(); // use the painter font metrics, not the screen fm!
    int fontHeight  = fm.lineSpacing();
    // fetch printer margins individual for all four page sides, as at least top and bottom are not the same
    uint left, right, top, bottom;
    QRect paper = pPrinter->paperRect();
    QRect page  = pPrinter->pageRect();
    top = paper.top() - page.top();
    left = paper.left() - page.left();
    bottom = paper.bottom() - page.bottom();
    right = paper.right() - page.right();
    uDebug() << "margin: top=" << QString().setNum(top) << " left=" << QString().setNum(left)
             << " bottom=" << QString().setNum(bottom) << " right=" << QString().setNum(right);
    // give a little extra space at each side
    left += 2;
    right += 2;
    top += 2;
    bottom += 2;

    if (pPrinter->orientation() == QPrinter::Landscape) {
        // we are printing in LANDSCAPE -->swap marginX and marginY
        uint right_old = right;
        // the DiagramRight side is printed at PrintersTop
        right = top;
        // the DiagramTop side is printed at PrintersLeft
        top = left;
        // the DiagramLeft side is printed at PrintersBottom
        left = bottom;
        // the DiagramBottom side is printed at PrintersRight
        bottom = right_old;
    }

    // The printer will probably use a different font with different font metrics,
    // force the widgets to update accordingly on paint
    forceUpdateWidgetFontMetrics(&pPainter);

    width = pPrinter->width() - left - right;
    height = pPrinter->height() - top - bottom;

    //get the smallest rect holding the diagram
    QRect rect = getDiagramRect().toRect();
    //now draw to printer

#if 0
    int offsetX = 0, offsetY = 0, widthX = 0, heightY = 0;
    // respect the margin
    pPainter.translate(marginX, marginY);

    // clip away everything outside of the margin
    pPainter.setClipRect(marginX, marginY,
                         width, pPrinter->height() - marginY * 2);

    //loop until all of the picture is printed
    int numPagesX = (int)ceil((double)rect.width() / (double)width);
    int numPagesY = (int)ceil((double)rect.height() / (double)height);
    int page = 0;

    // print the canvas to multiple pages
    for (int pageY = 0; pageY < numPagesY; ++pageY) {
        // tile vertically
        offsetY = pageY * height + rect.y();
        heightY = (pageY + 1) * height > rect.height()
            ? rect.height() - pageY * height
            : height;
        for (int pageX = 0; pageX < numPagesX; ++pageX) {
            // tile horizontally
            offsetX = pageX * width + rect.x();
            widthX = (pageX + 1) * width > rect.width()
                ? rect.width() - pageX * width
                : width;

            // make sure the part of the diagram is painted at the correct
            // place in the printout
            pPainter.translate(-offsetX, -offsetY);
            getDiagram(QRect(offsetX, offsetY, widthX, heightY),
                       pPainter);
            // undo the translation so the coordinates for the painter
            // correspond to the page again
            pPainter.translate(offsetX, offsetY);

            //draw foot note
            QString string = i18n("Diagram: %2 Page %1", page + 1, getName());
            QColor textColor(50, 50, 50);
            pPainter.setPen(textColor);
            pPainter.drawLine(0, height + 2, width, height + 2);
            pPainter.drawText(0, height + 4, width, fontHeight, Qt::AlignLeft, string);

            if (pageX + 1 < numPagesX || pageY + 1 < numPagesY) {
                pPrinter->newPage();
                page++;
            }
        }
    }
#else
    // be gentle - as described in Qt-Doc "The Coordinate System"
    pPainter.save();

    int diagramHeight = rect.height();
    // + 4+fontHeight between diagram and footline as space-buffer
    // + 2            between line and foot-text
    // + 1            for foot-line
    // + fontHeight   for foot-text
    // ==============
    // (2*fontHeight) + 7
    int footHeight;
    int footTop;
    int drawHeight;
    bool isFooter = getOptionState().generalState.footerPrinting;
    if (isFooter) {
        footHeight = (2 * fontHeight) + 7;
        footTop    = rect.y() + diagramHeight  + 4 + fontHeight;
        drawHeight = diagramHeight  + footHeight;
    } else {
        footHeight = 0;
        footTop    = rect.y() + diagramHeight;
        drawHeight = diagramHeight;
    }

    // set window of painter to dimensions of diagram
    // set window to viewport relation so that x:y isn't changed
    double dScaleX = (double)rect.width() / (double)width;
    double dScaleY = (double)drawHeight / (double)height;
    // select the scaling factor so that the larger dimension
    // fits on the printer page->use the larger scaling factor
    //->the virtual diagram window has some additional space at the
    // shorter dimension
    double dScaleUse = (dScaleX > dScaleY) ? dScaleX : dScaleY;

    int windowWidth  = (int)ceil(dScaleUse * width);
    int windowHeight = (int)ceil(dScaleUse * height);
#ifdef DEBUG_PRINTING
    uDebug() << "drawHeight: " << drawHeight << ", width: " << rect.width()
             << "\nPageHeight: " << height << ", PageWidht: " << width
             << "\nScaleY: " << dScaleY << ", ScaleX: " << dScaleX
             << "\ndScaleUse: " << dScaleUse
             << "\nVirtualSize: Width: " << windowWidth << ", Height: " << windowHeight
             << "\nFoot Top: " << footTop
             << endl;
#endif
    // set virtual drawing area window - where diagram fits 100% in
    pPainter.setWindow(rect.x(), rect.y(), windowWidth, windowHeight);

    // set viewport - the physical mapping
    // -->Qt's QPainter will map all drawed elements from diagram area (window)
    //     to printer area (viewport)
    pPainter.setViewport(left, top, width, height);

    // get Diagram
    getDiagram(QRect(rect.x(), rect.y(), windowWidth, diagramHeight), pPainter);

    if (isFooter) {
        //draw foot note
        QString string = i18n("Diagram: %2 Page %1", 1, getName());
        QColor textColor(50, 50, 50);
        pPainter.setPen(textColor);
        pPainter.drawLine(rect.x(), footTop    , windowWidth, footTop);
        pPainter.drawText(rect.x(), footTop + 3, windowWidth, fontHeight, Qt::AlignLeft, string);
    }
    // now restore scaling
    pPainter.restore();

#endif
    // next painting will most probably be to a different device (i.e. the screen)
    forceUpdateWidgetFontMetrics(0);
}

void UMLScene::setupNewWidget(NewUMLRectWidget *w)
{
	if(w->scene() != this) {
		addItem(w);
	}
    w->setX(m_Pos.x());
    w->setY(m_Pos.y());
    w->setVisible(true);
    w->setActivated();
    w->setFont(getFont());

    // [PORT]
    //w->slotColorChanged(getID());
    //w->slotLineWidthChanged(getID());

    resizeCanvasToItems();
    m_WidgetList.append(w);
    m_pDoc->setModified();

    // [PORT]
    // UMLApp::app()->executeCommand(new CmdCreateWidget(this, w));
}

void UMLScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* ome)
{
    m_pToolBarState->mouseRelease(ome);
    m_isMouseMovingItems = false;
}

void UMLScene::slotToolBarChanged(int c)
{
    m_pToolBarState->cleanBeforeChange();
    m_pToolBarState = m_pToolBarStateFactory->getState((WorkToolBar::ToolBar_Buttons)c, this);
    m_pToolBarState->init();

    m_bPaste = false;
}

void UMLScene::slotObjectCreated(UMLObject* o)
{
    m_bPaste = false;
    //check to see if we want the message
    //may be wanted by someone else e.g. list view

    if (!m_bCreateObject)  {
        return;
    }

    NewUMLRectWidget* newWidget = Widget_Factory::createWidget(this, o);

    if (newWidget == NULL)
        return;

    newWidget->setVisible(true);
    newWidget->setActivated();
    newWidget->setFont(getFont());

    // [PORT]
    // newWidget->slotColorChanged(getID());
    // newWidget->slotLineWidthChanged(getID());

    newWidget->updateComponentSize();

    if (m_Type == Uml::dt_Sequence) {
        // Set proper position on the sequence line widget which is
        // attached to the object widget.
        ObjectWidget *ow = dynamic_cast<ObjectWidget*>(newWidget);
        if (ow)
            ow->moveEvent(NULL);
    }
    m_bCreateObject = false;
    m_WidgetList.append(newWidget);

    switch (o->getBaseType()) {
    case ot_Actor:
    case ot_UseCase:
    case ot_Class:
    case ot_Package:
    case ot_Component:
    case ot_Node:
    case ot_Artifact:
    case ot_Interface:
    case ot_Enum:
    case ot_Entity:
    case ot_Datatype:
    case ot_Category:
        createAutoAssociations(newWidget);
        // We need to invoke createAutoAttributeAssociations()
        // on all other widgets again because the newly created
        // widget might saturate some latent attribute assocs.
        foreach(NewUMLRectWidget* w,  m_WidgetList) {
            if (w != newWidget) {
                createAutoAttributeAssociations(w);

                if (o->getBaseType() == ot_Entity)
                    createAutoConstraintAssociations(w);
            }
        }
        break;
    default:
        break;
    }
    resizeCanvasToItems();
}

void UMLScene::slotObjectRemoved(UMLObject * o)
{
    m_bPaste = false;
    Uml::IDType id = o->getID();

    foreach(NewUMLRectWidget* obj, m_WidgetList) {
        if (obj->getID() != id)
            continue;
        removeWidget(obj);
        break;
    }
}

void UMLScene::dragEnterEvent(QGraphicsSceneDragDropEvent *e)
{
    UMLDragData::LvTypeAndID_List tidList;
    if (!UMLDragData::getClip3TypeAndID(e->mimeData(), tidList)) {
        return;
    }
    UMLDragData::LvTypeAndID_It tidIt(tidList);
    UMLDragData::LvTypeAndID * tid = tidIt.current();
    if (!tid) {
        uDebug() << "UMLDragData::getClip3TypeAndID returned empty list" << endl;
        return;
    }
    ListView_Type lvtype = tid->type;
    Uml::IDType id = tid->id;

    Diagram_Type diagramType = getType();

    UMLObject* temp = 0;
    //if dragging diagram - might be a drag-to-note
    if (Model_Utils::typeIsDiagram(lvtype)) {
        e->accept();
        return;
    }
    //can't drag anything onto state/activity diagrams
    if (diagramType == dt_State || diagramType == dt_Activity) {
        e->ignore();
        return;
    }
    //make sure can find UMLObject
    if (!(temp = m_pDoc->findObjectById(id))) {
        uDebug() << "object " << ID2STR(id) << " not found";
        e->ignore();
        return;
    }
    //make sure dragging item onto correct diagram
    // concept - class,seq,coll diagram
    // actor,usecase - usecase diagram
    Object_Type ot = temp->getBaseType();
    bool bAccept = true;
    switch (diagramType) {
    case dt_UseCase:
        if ((widgetOnDiagram(id) && ot == ot_Actor) ||
            (ot != ot_Actor && ot != ot_UseCase))
            bAccept = false;
        break;
    case dt_Class:
        if (widgetOnDiagram(id) ||
            (ot != ot_Class &&
             ot != ot_Package &&
             ot != ot_Interface &&
             ot != ot_Enum &&
             ot != ot_Datatype)) {
            bAccept = false;
        }
        break;
    case dt_Sequence:
    case dt_Collaboration:
        if (ot != ot_Class &&
            ot != ot_Interface &&
            ot != ot_Actor)
            bAccept = false;
        break;
    case dt_Deployment:
        if (widgetOnDiagram(id))
            bAccept = false;
        else if (ot != ot_Interface &&
                 ot != ot_Package &&
                 ot != ot_Component &&
                 ot != ot_Class &&
                 ot != ot_Node)
            bAccept = false;
        else if (ot == ot_Package &&
                 temp->getStereotype() != "subsystem")
            bAccept = false;
        break;
    case dt_Component:
        if (widgetOnDiagram(id) ||
            (ot != ot_Interface &&
             ot != ot_Package &&
             ot != ot_Component &&
             ot != ot_Artifact &&
             ot != ot_Class))
            bAccept = false;
        if (ot == ot_Class && !temp->getAbstract())
            bAccept = false;
        break;
    case dt_EntityRelationship:
        if (ot != ot_Entity && ot != ot_Category)
            bAccept = false;
        break;
    default:
        break;
    }
    if (bAccept) {
        e->accept();
    } else {
        e->ignore();
    }
}

void UMLScene::dragMoveEvent(QGraphicsSceneDragDropEvent* e)
{
    e->accept();
}


void UMLScene::dropEvent(QGraphicsSceneDragDropEvent *e)
{
    UMLDragData::LvTypeAndID_List tidList;
    if (!UMLDragData::getClip3TypeAndID(e->mimeData(), tidList)) {
        return;
    }
    UMLDragData::LvTypeAndID_It tidIt(tidList);
    UMLDragData::LvTypeAndID * tid = tidIt.current();
    if (!tid) {
        uDebug() << "UMLDragData::getClip3TypeAndID returned empty list" << endl;
        return;
    }
    ListView_Type lvtype = tid->type;
    Uml::IDType id = tid->id;

    if (Model_Utils::typeIsDiagram(lvtype)) {
        bool breakFlag = false;
        NewUMLRectWidget* w = 0;
        foreach(w ,  m_WidgetList) {
            bool isPointOnWidget = w->onWidget(e->scenePos());
            if (w->getBaseType() == Uml::wt_Note && isPointOnWidget) {
                breakFlag = true;
                break;
            }
        }
        if (breakFlag) {
            NoteWidget *note = static_cast<NoteWidget*>(w);
            note->setDiagramLink(id);
        }
        return;
    }
    UMLObject* o = m_pDoc->findObjectById(id);
    if (!o) {
        uDebug() << "object id=" << ID2STR(id) << " not found" << endl;
        return;
    }
    m_bCreateObject = true;
    m_Pos = e->scenePos();

    slotObjectCreated(o);

    m_pDoc->setModified(true);
}

ObjectWidget * UMLScene::onWidgetLine(const QPointF &point) const
{
    foreach(NewUMLRectWidget* obj, m_WidgetList) {
        ObjectWidget *ow = dynamic_cast<ObjectWidget*>(obj);
        if (ow == NULL)
            continue;
        SeqLineWidget *pLine = ow->getSeqLine();
        if (pLine == NULL) {
            uError() << "SeqLineWidget of " << ow->getName()
                     << " (id=" << ID2STR(ow->getLocalID()) << ") is NULL" << endl;
            continue;
        }
        if (pLine->onWidget(point))
            return ow;
    }
    return 0;
}

ObjectWidget * UMLScene::onWidgetDestructionBox(const QPointF &point) const
{

    foreach(NewUMLRectWidget* obj,  m_WidgetList) {
        ObjectWidget *ow = dynamic_cast<ObjectWidget*>(obj);
        if (ow == NULL)
            continue;
        SeqLineWidget *pLine = ow->getSeqLine();
        if (pLine == NULL) {
            uError() << "SeqLineWidget of " << ow->getName()
                     << " (id=" << ID2STR(ow->getLocalID()) << ") is NULL" << endl;
            continue;
        }
        if (pLine->onDestructionBox(point))
            return ow;
    }
    return 0;
}

NewUMLRectWidget *UMLScene::getWidgetAt(const QPointF& p)
{
    int relativeSize = 10000;  // start with an arbitrary large number
    NewUMLRectWidget  *retObj = NULL;
    UMLWidgetListIt it(m_WidgetList);
    foreach(NewUMLRectWidget* obj,  m_WidgetList) {
        const qreal s = obj->onWidget(p);
        if (!s)
            continue;
        if (s < relativeSize) {
            relativeSize = s;
            retObj = obj;
        }
    }
    return retObj;
}

void UMLScene::checkMessages(ObjectWidget * w)
{
    if (getType() != dt_Sequence)
        return;

    MessageWidgetListIt it(m_MessageList);
    foreach(MessageWidget *obj , m_MessageList) {
        if (! obj->contains(w))
            continue;
        //make sure message doesn't have any associations
        removeAssociations(obj);
        obj->cleanup();
        //make sure not in selected list
        m_MessageList.removeAll(obj);
        delete obj;
    }
}

bool UMLScene::widgetOnDiagram(Uml::IDType id)
{

    foreach(NewUMLRectWidget *obj, m_WidgetList) {
        if (id == obj->getID())
            return true;
    }

    foreach(NewUMLRectWidget *obj , m_MessageList) {
        if (id == obj->getID())
            return true;
    }

    return false;
}

void UMLScene::mouseMoveEvent(QGraphicsSceneMouseEvent* ome)
{
    m_pToolBarState->mouseMove(ome);
}

// search both our NewUMLRectWidget AND MessageWidget lists
NewUMLRectWidget * UMLScene::findWidget(Uml::IDType id)
{
    foreach(NewUMLRectWidget* obj, m_WidgetList) {
        // object widgets are special..the widget id is held by 'localId' attribute (crappy!)
        if (obj->getBaseType() == wt_Object) {
            if (static_cast<ObjectWidget *>(obj)->getLocalID() == id)
                return obj;
        } else if (obj->getID() == id) {
            return obj;
        }
    }

    foreach(NewUMLRectWidget* obj, m_MessageList) {
        if (obj->getID() == id)
            return obj;
    }

    return 0;
}



AssociationWidget * UMLScene::findAssocWidget(Uml::IDType id)
{
    foreach(AssociationWidget* obj , m_AssociationList) {
        UMLAssociation* umlassoc = obj->getAssociation();
        if (umlassoc && umlassoc->getID() == id) {
            return obj;
        }
    }
    return 0;
}

AssociationWidget * UMLScene::findAssocWidget(NewUMLRectWidget *pWidgetA,
                                              NewUMLRectWidget *pWidgetB, const QString& roleNameB)
{
    foreach(AssociationWidget* assoc, m_AssociationList) {
        const Association_Type testType = assoc->getAssocType();
        if (testType != Uml::at_Association &&
            testType != Uml::at_UniAssociation &&
            testType != Uml::at_Composition &&
            testType != Uml::at_Aggregation &&
            testType != Uml::at_Relationship)
            continue;
        if (pWidgetA->getID() == assoc->getWidgetID(A) &&
            pWidgetB->getID() == assoc->getWidgetID(B) &&
            assoc->getRoleName(Uml::B) == roleNameB)
            return assoc;
    }
    return 0;
}


AssociationWidget * UMLScene::findAssocWidget(Uml::Association_Type at,
                                              NewUMLRectWidget *pWidgetA, NewUMLRectWidget *pWidgetB)
{

    foreach(AssociationWidget* assoc, m_AssociationList) {
        Association_Type testType = assoc->getAssocType();
        if (testType != at)
            continue;
        if (pWidgetA->getID() == assoc->getWidgetID(A) &&
            pWidgetB->getID() == assoc->getWidgetID(B))
            return assoc;
    }
    return 0;
}

void UMLScene::removeWidget(NewUMLRectWidget * o)
{
    if (!o)
        return;

    emit sigWidgetRemoved(o);

    removeAssociations(o);

    Widget_Type t = o->getBaseType();
    if (getType() == dt_Sequence && t == wt_Object)
        checkMessages(static_cast<ObjectWidget*>(o));

    o->cleanup();
    disconnect(this, SIGNAL(sigRemovePopupMenu()), o, SLOT(slotRemovePopupMenu()));
    disconnect(this, SIGNAL(sigClearAllSelected()), o, SLOT(slotClearAllSelected()));
    disconnect(this, SIGNAL(sigColorChanged(Uml::IDType)), o, SLOT(slotColorChanged(Uml::IDType)));
    if (t == wt_Message) {
        m_MessageList.removeAll(static_cast<MessageWidget*>(o));
    } else
        m_WidgetList.removeAll(o);
    delete o;
    m_pDoc->setModified(true);
}

bool UMLScene::getUseFillColor() const
{
    return m_Options.uiState.useFillColor;
}

void UMLScene::setUseFillColor(bool ufc)
{
    m_Options.uiState.useFillColor = ufc;
}

QColor UMLScene::getFillColor() const
{
    return m_Options.uiState.fillColor;
}

void UMLScene::setFillColor(const QColor &color)
{
    m_Options.uiState.fillColor = color;
    emit sigColorChanged(getID());
}

QColor UMLScene::getLineColor() const
{
    return m_Options.uiState.lineColor;
}

void UMLScene::setLineColor(const QColor &color)
{
    m_Options.uiState.lineColor = color;
    emit sigColorChanged(getID());
}

uint UMLScene::getLineWidth() const
{
    return m_Options.uiState.lineWidth;
}

void UMLScene::setLineWidth(uint width)
{
    m_Options.uiState.lineWidth = width;
    emit sigLineWidthChanged(getID());
}

void UMLScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* ome)
{
    m_pToolBarState->mouseDoubleClick(ome);
}

QRectF UMLScene::getDiagramRect()
{
    return itemsBoundingRect();
}

void UMLScene::setSelected(NewUMLRectWidget * w, QGraphicsSceneMouseEvent * me)
{
    Q_UNUSED(me);
    int count = selectedItems().count();
    //only call once - if we select more, no need to keep clearing  window

    // if count == 1, widget will update the doc window with their data when selected
    if (count == 2)
        updateDocumentation(true);  //clear doc window
    // [PORT] We no more need to maintain a list of selected widgets
    // ourselves as this is done by QGraphicsScene
    w->setSelected(true);

    /* selection changed, we have to make sure the copy and paste items
     * are correctly enabled/disabled */
    UMLApp::app()->slotCopyChanged();
}

UMLWidgetList UMLScene::selectedWidgets() const
{
    UMLWidgetList list;
    foreach(QGraphicsItem *item, selectedItems()) {
        NewUMLRectWidget *wid = dynamic_cast<NewUMLRectWidget*>(item);
        if(wid) {
            list << wid;
        }
    }
    return list;
}

NewUMLRectWidget* UMLScene::getFirstMultiSelectedWidget() const
{
    UMLWidgetList list = selectedWidgets();
    if(list.isEmpty()) {
        return 0;
    }
    return list.first();
}

void UMLScene::clearSelected()
{
    clearSelection();
}

//TODO Only used in UMLApp::handleCursorKeyReleaseEvent
void UMLScene::moveSelectedBy(qreal dX, qreal dY)
{
    // uDebug() << "********** selectedWidgets() count=" << selectedWidgets().count();
    foreach(NewUMLRectWidget *w, selectedWidgets()) {
        w->moveBy(dX, dY);
    }
}

void UMLScene::selectionUseFillColor(bool useFC)
{
    foreach(NewUMLRectWidget* temp, selectedWidgets()) {
        temp->setUseFillColour(useFC);
    }
}

void UMLScene::selectionSetFont(const QFont &font)
{
    foreach(NewUMLRectWidget* temp, selectedWidgets()) {
        temp->setFont(font);
    }
}

void UMLScene::selectionSetLineColor(const QColor &color)
{
    UMLApp::app()->BeginMacro("Change Line Color");
    foreach(NewUMLRectWidget *temp ,  selectedWidgets()) {
        temp->setLineColor(color);
        // [PORT] temp->setUsesDiagramLineColour(false);
    }
    AssociationWidgetList assoclist = getSelectedAssocs();
    foreach(AssociationWidget *aw , assoclist) {
        aw->setLineColor(color);
        // [PORT] aw->setUsesDiagramLineColour(false);
    }
    UMLApp::app()->EndMacro();
}

void UMLScene::selectionSetLineWidth(uint width)
{
    foreach(NewUMLRectWidget* temp , selectedWidgets()) {
        temp->setLineWidth(width);
        // [PORT] temp->setUsesDiagramLineWidth(false);
    }
    AssociationWidgetList assoclist = getSelectedAssocs();
    foreach(AssociationWidget *aw , assoclist) {
        aw->setLineWidth(width);
        // [PORT] aw->setUsesDiagramLineWidth(false);
    }
}

void UMLScene::selectionSetFillColor(const QColor &color)
{
    UMLApp::app()->BeginMacro("Change Fill Color");

    foreach(NewUMLRectWidget* temp ,  selectedWidgets()) {
        temp->setFillColour(color);
        // [PORT] temp->setUsesDiagramFillColour(false);
    }
    UMLApp::app()->EndMacro();
}

void UMLScene::selectionToggleShow(int sel)
{
    // loop through all selected items
    foreach(NewUMLRectWidget *temp , selectedWidgets()) {
        Widget_Type type = temp->getBaseType();
        ClassifierWidget *cw = dynamic_cast<ClassifierWidget*>(temp);

        // toggle the show setting sel
        switch (sel) {
            // some setting are only available for class, some for interface and some
            // for both
        case ListPopupMenu::mt_Show_Attributes_Selection:
            if (type == wt_Class)
                cw->toggleVisualProperty(ClassifierWidget::ShowAttributes);
            break;
        case ListPopupMenu::mt_Show_Operations_Selection:
            if (cw)
                cw->toggleVisualProperty(ClassifierWidget::ShowOperations);
            break;
        case ListPopupMenu::mt_Visibility_Selection:
            if (cw)
                cw->toggleVisualProperty(ClassifierWidget::ShowVisibility);
            break;
        case ListPopupMenu::mt_DrawAsCircle_Selection:
            if (type == wt_Interface)
                cw->toggleVisualProperty(ClassifierWidget::DrawAsCircle);
            break;
        case ListPopupMenu::mt_Show_Operation_Signature_Selection:
            if (cw)
                cw->toggleVisualProperty(ClassifierWidget::ShowOperationSignature);
            break;
        case ListPopupMenu::mt_Show_Attribute_Signature_Selection:
            if (type == wt_Class)
                cw->toggleVisualProperty(ClassifierWidget::ShowAttributeSignature);
            break;
        case ListPopupMenu::mt_Show_Packages_Selection:
            if (cw)
                cw->toggleVisualProperty(ClassifierWidget::ShowPackage);
            break;
        case ListPopupMenu::mt_Show_Stereotypes_Selection:
            if (type == wt_Class)
                cw->toggleVisualProperty(ClassifierWidget::ShowStereotype);
            break;
        case ListPopupMenu::mt_Show_Public_Only_Selection:
            if (cw)
                cw->toggleVisualProperty(ClassifierWidget::ShowPublicOnly);
            break;
        default:
            break;
        } // switch (sel)
    }
}

void UMLScene::deleteSelection()
{
    /*
      Don't delete text widget that are connect to associations as these will
      be cleaned up by the associations.
    */

    foreach(NewUMLRectWidget* temp ,  selectedWidgets()) {
        if (temp->getBaseType() == wt_Text &&
            ((FloatingTextWidget *)temp)->getRole() != tr_Floating) {
            temp->hide();

        } else {
            removeWidget(temp);
        }

    }

    // Delete any selected associations.

    foreach(AssociationWidget* assocwidget, m_AssociationList) {
        if (assocwidget->getSelected())
            removeAssoc(assocwidget);
        // MARK
    }

    /* we also have to remove selected messages from sequence diagrams */

    /* loop through all messages and check the selection state */
    foreach(MessageWidget* cur_msgWgt , m_MessageList) {
        if (cur_msgWgt->isSelected()) {
            removeWidget(cur_msgWgt);  // Remove message - it is selected.
        }
    }

    // sometimes we miss one widget, so call this function again to remove it as
    // well
    if (!selectedWidgets().isEmpty())
        deleteSelection();

}

void UMLScene::selectAll()
{
    clearSelection();
    QPainterPath path;
    path.addRect(sceneRect());
    setSelectionArea(path);
}

Uml::IDType UMLScene::getLocalID()
{
    m_nLocalID = UniqueID::gen();
    return m_nLocalID;
}

bool UMLScene::isSavedInSeparateFile()
{
    if (getOptionState().generalState.tabdiagrams) {
        // Umbrello currently does not support external folders
        // when tabbed diagrams are enabled.
        return false;
    }
    const QString msgPrefix("UMLScene::isSavedInSeparateFile(" + getName() + "): ");
    UMLListView *listView = UMLApp::app()->getListView();
    UMLListViewItem *lvItem = listView->findItem(m_nID);
    if (lvItem == NULL) {
        uError() << msgPrefix
                 << "listView->findUMLObject(this) returns false" << endl;
        return false;
    }
    UMLListViewItem *parentItem = dynamic_cast<UMLListViewItem*>(lvItem->parent());
    if (parentItem == NULL) {
        uError() << msgPrefix
                 << "parent item in listview is not a UMLListViewItem (?)" << endl;
        return false;
    }
    const Uml::ListView_Type lvt = parentItem->getType();
    if (! Model_Utils::typeIsFolder(lvt))
        return false;
    UMLFolder *modelFolder = dynamic_cast<UMLFolder*>(parentItem->getUMLObject());
    if (modelFolder == NULL) {
        uError() << msgPrefix
                 << "parent model object is not a UMLFolder (?)" << endl;
        return false;
    }
    QString folderFile = modelFolder->getFolderFile();
    return !folderFile.isEmpty();
}

void UMLScene::mousePressEvent(QGraphicsSceneMouseEvent* ome)
{
    if (isArrowMode() && ome->buttons().testFlag(Qt::LeftButton)) {
        m_isMouseMovingItems = true;
    }

    m_pToolBarState->mousePress(ome);

    //TODO should be managed by widgets when are selected. Right now also has some
    //problems, such as clicking on a widget, and clicking to move that widget shows
    //documentation of the diagram instead of keeping the widget documentation.
    //When should diagram documentation be shown? When clicking on an empty
    //space in the diagram with arrow tool?
    if (!m_bChildDisplayedDoc) {
        // [PORT]
        // UMLApp::app()->getDocWindow()->showDocumentation(this, true);
    }
    m_bChildDisplayedDoc = false;
}

bool UMLScene::isArrowMode()
{
    return m_pToolBarState ==
        m_pToolBarStateFactory->getState(WorkToolBar::tbb_Arrow, this);
}

void UMLScene::makeSelected(NewUMLRectWidget * uw)
{
    if (uw) {
        uw->setSelected(true);
    }
}

void UMLScene::selectWidgetsOfAssoc(AssociationWidget * a)
{
    if (!a)
        return;
    a->setSelected(true);
    //select the two widgets
    makeSelected(a->getWidget(A));
    makeSelected(a->getWidget(B));
    //select all the text
    makeSelected(a->getMultiWidget(A));
    makeSelected(a->getMultiWidget(B));
    makeSelected(a->getRoleWidget(A));
    makeSelected(a->getRoleWidget(B));
    makeSelected(a->getChangeWidget(A));
    makeSelected(a->getChangeWidget(B));
}

void UMLScene::selectWidgets(qreal px, qreal py, qreal qx, qreal qy)
{
    clearSelected();
    QRectF rect = QRectF(QPointF(px, py), QPointF(qx, qy)).normalized();
    QPainterPath path;
    path.addRect(rect);
    setSelectionArea(path);
}

void  UMLScene::getDiagram(const QRectF &rect, QPixmap & diagram)
{
    QPixmap pixmap(rect.x() + rect.width(), rect.y() + rect.height());
    QPainter painter(&pixmap);
    getDiagram(sceneRect(), painter);
    QPainter output(&diagram);
    output.drawPixmap(QPoint(0, 0), pixmap, rect);
}

void  UMLScene::getDiagram(const QRectF &area, QPainter & painter)
{
    //TODO unselecting and selecting later doesn't work now as the selection is
    //cleared in UMLViewImageExporter. Check if the anything else than the
    //following is needed and, if it works, remove the clearSelected in
    //UMLViewImageExporter and UMLViewImageExporterModel

    UMLWidgetList selected = selectedWidgets();
    foreach(NewUMLRectWidget* widget , selected) {
        widget->setSelected(false);
    }
    AssociationWidgetList selectedAssociationsList = getSelectedAssocs();

    foreach(AssociationWidget* association , selectedAssociationsList) {
        association->setSelected(false);
    }

    // we don't want to get the grid
    bool showSnapGrid = getShowSnapGrid();
    setShowSnapGrid(false);

    // TODO: Check if this render method is identical to cavnas()->drawArea()
    // [PORT]
    render(&painter, QRectF(), area, Qt::KeepAspectRatio);

    setShowSnapGrid(showSnapGrid);

    //select again
    foreach(NewUMLRectWidget* widget , selected) {
        widget->setSelected(true);
    }
    foreach(AssociationWidget* association , selectedAssociationsList) {
        association->setSelected(true);
    }
}

UMLViewImageExporter* UMLScene::getImageExporter()
{
    return m_pImageExporter;
}

void UMLScene::slotActivate()
{
    m_pDoc->changeCurrentView(getID());
}

UMLObjectList UMLScene::getUMLObjects()
{
    UMLObjectList list;
    foreach(NewUMLRectWidget* w,  m_WidgetList) {

        switch (w->getBaseType()) { //use switch for easy future expansion
        case wt_Actor:
        case wt_Class:
        case wt_Interface:
        case wt_Package:
        case wt_Component:
        case wt_Node:
        case wt_Artifact:
        case wt_UseCase:
        case wt_Object:
            list.append(w->getUMLObject());
            break;
        default:
            break;
        }
    }
    return list;
}

void UMLScene::activate()
{
    //Activate Regular widgets then activate  messages
    foreach(NewUMLRectWidget* obj , m_WidgetList) {
        //If this NewUMLRectWidget is already activated or is a MessageWidget then skip it
        if (/* [PORT] obj->isActivated() || */ obj->getBaseType() == wt_Message)
            continue;

        if (obj->activate(0)) {
            obj->setVisible(true);
        } else {
            m_WidgetList.removeAll(obj);
            delete obj;
        }
    }//end foreach

    //Activate Message widgets
    foreach(NewUMLRectWidget* obj , m_MessageList) {
        //If this MessageWidget is already activated then skip it
        // [PORT]
        // if (obj->isActivated())
        //     continue;

        obj->activate(m_pDoc->getChangeLog());
        obj->setVisible(true);

    }//end foreach

    // Activate all association widgets

    foreach(AssociationWidget* aw , m_AssociationList) {
        if (aw->activate()) {
            if (m_PastePoint.x() != 0.) {
                qreal x = m_PastePoint.x() - m_Pos.x();
                qreal y = m_PastePoint.y() - m_Pos.y();
                aw->moveEntireAssoc(x, y);
            }
        } else {
            m_AssociationList.removeAll(aw);
            delete  aw;
        }
    }
}

int UMLScene::getSelectCount(bool filterText) const
{
    if (!filterText)
        return selectedWidgets().count();
    int counter = 0;
    const NewUMLRectWidget * temp = 0;
    foreach(temp, selectedWidgets()) {
        if (temp->getBaseType() == wt_Text) {
            const FloatingTextWidget *ft = static_cast<const FloatingTextWidget*>(temp);
            if (ft->getRole() == tr_Floating)
                counter++;
        } else {
            counter++;
        }
    }
    return counter;
}


bool UMLScene::getSelectedWidgets(UMLWidgetList &WidgetList, bool filterText /*= true*/)
{
    foreach(NewUMLRectWidget* temp, selectedWidgets()) {
        if (filterText && temp->getBaseType() == wt_Text) {
            const FloatingTextWidget *ft = static_cast<const FloatingTextWidget*>(temp);
            if (ft->getRole() == tr_Floating)
                WidgetList.append(temp);
        } else {
            WidgetList.append(temp);
        }
    }//end for
    return true;
}

AssociationWidgetList UMLScene::getSelectedAssocs()
{
    AssociationWidgetList assocWidgetList;

    foreach(AssociationWidget* assocwidget, m_AssociationList) {
        if (assocwidget->getSelected())
            assocWidgetList.append(assocwidget);
    }
    return assocWidgetList;
}

bool UMLScene::addWidget(NewUMLRectWidget * pWidget , bool isPasteOperation)
{
    if (!pWidget) {
        return false;
    }
    Widget_Type type = pWidget->getBaseType();
    if (isPasteOperation) {
        if (type == Uml::wt_Message)
            m_MessageList.append(static_cast<MessageWidget*>(pWidget));
        else
            m_WidgetList.append(pWidget);
        return true;
    }
    if (!isPasteOperation && findWidget(pWidget->getID())) {
        uError() << "Not adding (id=" << ID2STR(pWidget->getID())
                 << "/type=" << type << "/name=" << pWidget->getName()
                 << ") because it is already there" << endl;
        return false;
    }
    IDChangeLog * log = m_pDoc->getChangeLog();
    if (isPasteOperation && (!log || !m_pIDChangesLog)) {
        uError() << " Cant addWidget to view in paste op because a log is not open" << endl;
        return false;
    }
    qreal wX = pWidget->getX();
    qreal wY = pWidget->getY();
    bool xIsOutOfRange = (wX <= 0. || wX >= FloatingTextWidget::restrictPositionMax);
    bool yIsOutOfRange = (wY <= 0. || wY >= FloatingTextWidget::restrictPositionMax);
    if (xIsOutOfRange || yIsOutOfRange) {
        QString name = pWidget->getName();
        if (name.isEmpty()) {
            FloatingTextWidget *ft = dynamic_cast<FloatingTextWidget*>(pWidget);
            if (ft)
                name = ft->getDisplayText();
        }
        uDebug() << name << " type=" << pWidget->getBaseType() << "): position ("
                 << wX << "," << wY << ") is out of range" << endl;
        if (xIsOutOfRange) {
            pWidget->setX(0);
            wX = 0;
        }
        if (yIsOutOfRange) {
            pWidget->setY(0);
            wY = 0;
        }
    }
    if (wX < m_Pos.x())
        m_Pos.setX(wX);
    if (wY < m_Pos.y())
        m_Pos.setY(wY);

    //see if we need a new id to match object
    switch (type) {

    case wt_Class:
    case wt_Package:
    case wt_Component:
    case wt_Node:
    case wt_Artifact:
    case wt_Interface:
    case wt_Enum:
    case wt_Entity:
    case wt_Datatype:
    case wt_Actor:
    case wt_UseCase:
    case wt_Category: {
        Uml::IDType id = pWidget->getID();
        Uml::IDType newID = log->findNewID(id);
        if (newID == Uml::id_None) {   // happens after a cut
            if (id == Uml::id_None)
                return false;
            newID = id; //don't stop paste
        } else
            pWidget->setID(newID);
        UMLObject * pObject = m_pDoc->findObjectById(newID);
        if (!pObject) {
            uDebug() << "addWidget: Can not find UMLObject for id "
                     << ID2STR(newID) << endl;
            return false;
        }
        pWidget->setUMLObject(pObject);
        //make sure it doesn't already exist.
        if (findWidget(newID)) {
            uDebug() << "Not adding (id=" << ID2STR(pWidget->getID())
                     << "/type=" << pWidget->getBaseType()
                     << "/name=" << pWidget->getName()
                     << ") because it is already there" << endl;
            delete pWidget; // Not nice but if _we_ don't do it nobody else will
            return true;//don't stop paste just because widget found.
        }
        m_WidgetList.append(pWidget);
    }
        break;

    case wt_Message:
    case wt_Note:
    case wt_Box:
    case wt_Text:
    case wt_State:
    case wt_Activity:
    case wt_ObjectNode: {
        Uml::IDType newID = m_pDoc->assignNewID(pWidget->getID());
        pWidget->setID(newID);
        if (type != wt_Message) {
            m_WidgetList.append(pWidget);
            return true;
        }
        // CHECK
        // Handling of wt_Message:
        MessageWidget *pMessage = static_cast<MessageWidget *>(pWidget);
        if (pMessage == NULL) {
            uDebug() << "pMessage is NULL";
            return false;
        }
        ObjectWidget *objWidgetA = pMessage->getWidget(A);
        ObjectWidget *objWidgetB = pMessage->getWidget(B);
        Uml::IDType waID = objWidgetA->getLocalID();
        Uml::IDType wbID = objWidgetB->getLocalID();
        Uml::IDType newWAID = m_pIDChangesLog->findNewID(waID);
        Uml::IDType newWBID = m_pIDChangesLog->findNewID(wbID);
        if (newWAID == Uml::id_None || newWBID == Uml::id_None) {
            uDebug() << "Error with ids : " << ID2STR(newWAID)
                     << " " << ID2STR(newWBID) << endl;
            return false;
        }
        // Assumption here is that the A/B objectwidgets and the textwidget
        // are pristine in the sense that we may freely change their local IDs.
        objWidgetA->setLocalID(newWAID);
        objWidgetB->setLocalID(newWBID);
        FloatingTextWidget *ft = pMessage->getFloatingTextWidget();
        if (ft == NULL)
            uDebug() << "FloatingTextWidget of Message is NULL";
        else if (ft->getID() == Uml::id_None)
            ft->setID(UniqueID::gen());
        else {
            Uml::IDType newTextID = m_pDoc->assignNewID(ft->getID());
            ft->setID(newTextID);
        }
        m_MessageList.append(pMessage);
    }
        break;

    case wt_Object: {
        ObjectWidget* pObjectWidget = static_cast<ObjectWidget*>(pWidget);
        if (pObjectWidget == NULL) {
            uDebug() << "pObjectWidget is NULL";
            return false;
        }
        Uml::IDType nNewLocalID = getLocalID();
        Uml::IDType nOldLocalID = pObjectWidget->getLocalID();
        m_pIDChangesLog->addIDChange(nOldLocalID, nNewLocalID);
        pObjectWidget->setLocalID(nNewLocalID);
        UMLObject *pObject = m_pDoc->findObjectById(pWidget->getID());
        if (!pObject) {
            uDebug() << "Cannot find UMLObject";
            return false;
        }
        pWidget->setUMLObject(pObject);
        m_WidgetList.append(pWidget);
    }
        break;

    case wt_Precondition: {
        ObjectWidget* pObjectWidget = static_cast<ObjectWidget*>(pWidget);
        if (pObjectWidget == NULL) {
            uDebug() << "pObjectWidget is NULL";
            return false;
        }
        Uml::IDType newID = log->findNewID(pWidget->getID());
        if (newID == Uml::id_None) {
            return false;
        }
        pObjectWidget->setID(newID);
        Uml::IDType nNewLocalID = getLocalID();
        Uml::IDType nOldLocalID = pObjectWidget->getLocalID();
        m_pIDChangesLog->addIDChange(nOldLocalID, nNewLocalID);
        pObjectWidget->setLocalID(nNewLocalID);
        UMLObject *pObject = m_pDoc->findObjectById(newID);
        if (!pObject) {
            uDebug() << "Cannot find UMLObject";
            return false;
        }
        pWidget->setUMLObject(pObject);
        m_WidgetList.append(pWidget);
    }
        break;

    case wt_Pin:
    case wt_CombinedFragment:
    case wt_Signal: {
        ObjectWidget* pObjectWidget = static_cast<ObjectWidget*>(pWidget);
        if (pObjectWidget == NULL) {
            uDebug() << "pObjectWidget is NULL";
            return false;
        }
        Uml::IDType newID = log->findNewID(pWidget->getID());
        if (newID == Uml::id_None) {
            return false;
        }
        pObjectWidget->setID(newID);
        Uml::IDType nNewLocalID = getLocalID();
        Uml::IDType nOldLocalID = pObjectWidget->getLocalID();
        m_pIDChangesLog->addIDChange(nOldLocalID, nNewLocalID);
        pObjectWidget->setLocalID(nNewLocalID);
        UMLObject *pObject = m_pDoc->findObjectById(newID);
        if (!pObject) {
            uDebug() << "Cannot find UMLObject";
            return false;
        }
        pWidget->setUMLObject(pObject);
        m_WidgetList.append(pWidget);
    }
        break;

    default:
        uDebug() << "Trying to add an invalid widget type";
        return false;
        break;
    }

    return true;
}

// Add the association, and its child widgets to this view
bool UMLScene::addAssociation(AssociationWidget* pAssoc , bool isPasteOperation)
{

    if (!pAssoc) {
        return false;
    }
    const Association_Type type = pAssoc->getAssocType();

    if (isPasteOperation) {
        IDChangeLog * log = m_pDoc->getChangeLog();

        if (!log)
            return false;

        Uml::IDType ida = Uml::id_None, idb = Uml::id_None;
        if (getType() == dt_Collaboration || getType() == dt_Sequence) {
            //check local log first
            ida = m_pIDChangesLog->findNewID(pAssoc->getWidgetID(A));
            idb = m_pIDChangesLog->findNewID(pAssoc->getWidgetID(B));
            //if either is still not found and assoc type is anchor
            //we are probably linking to a notewidet - else an error
            if (ida == Uml::id_None && type == at_Anchor)
                ida = log->findNewID(pAssoc->getWidgetID(A));
            if (idb == Uml::id_None && type == at_Anchor)
                idb = log->findNewID(pAssoc->getWidgetID(B));
        } else {
            Uml::IDType oldIdA = pAssoc->getWidgetID(A);
            Uml::IDType oldIdB = pAssoc->getWidgetID(B);
            ida = log->findNewID(oldIdA);
            if (ida == Uml::id_None) {  // happens after a cut
                if (oldIdA == Uml::id_None)
                    return false;
                ida = oldIdA;
            }
            idb = log->findNewID(oldIdB);
            if (idb == Uml::id_None) {  // happens after a cut
                if (oldIdB == Uml::id_None)
                    return false;
                idb = oldIdB;
            }
        }
        if (ida == Uml::id_None || idb == Uml::id_None) {
            return false;
        }
        // cant do this anymore.. may cause problem for pasting
        //      pAssoc->setWidgetID(ida, A);
        //      pAssoc->setWidgetID(idb, B);
        pAssoc->setWidget(findWidget(ida), A);
        pAssoc->setWidget(findWidget(idb), B);
    }

    NewUMLRectWidget * pWidgetA = findWidget(pAssoc->getWidgetID(A));
    NewUMLRectWidget * pWidgetB = findWidget(pAssoc->getWidgetID(B));
    //make sure valid widget ids
    if (!pWidgetA || !pWidgetB) {
        return false;
    }

    //make sure valid
    if (!isPasteOperation && !m_pDoc->loading() &&
        !AssocRules::allowAssociation(type, pWidgetA, pWidgetB, false)) {
        uWarning() << "allowAssociation returns false " << "for AssocType " << type << endl;
        return false;
    }

    //make sure there isn't already the same assoc

    foreach(AssociationWidget* assocwidget, m_AssociationList) {
        if (*pAssoc == *assocwidget)
            // this is nuts. Paste operation wants to know if 'true'
            // for duplicate, but loadFromXMI needs 'false' value
            return (isPasteOperation ? true : false);
    }

    m_AssociationList.append(pAssoc);

    FloatingTextWidget *ft[5] = { pAssoc->getNameWidget(),
                                  pAssoc->getRoleWidget(A),
                                  pAssoc->getRoleWidget(B),
                                  pAssoc->getMultiWidget(A),
                                  pAssoc->getMultiWidget(B)
    };
    for (int i = 0; i < 5; i++) {
        FloatingTextWidget *flotxt = ft[i];
        if (flotxt) {
            flotxt->updateComponentSize();
            addWidget(flotxt);
        }
    }

    return true;
}

void UMLScene::activateAfterLoad(bool bUseLog)
{
    if (m_bActivated)
        return;
    if (bUseLog) {
        beginPartialWidgetPaste();
    }

    //now activate them all
    activate();

    if (bUseLog) {
        endPartialWidgetPaste();
    }
    resizeCanvasToItems();
    m_bActivated = true;
}

void UMLScene::beginPartialWidgetPaste()
{
    delete m_pIDChangesLog;
    m_pIDChangesLog = 0;

    m_pIDChangesLog = new IDChangeLog();
    m_bPaste = true;
}

void UMLScene::endPartialWidgetPaste()
{
    delete    m_pIDChangesLog;
    m_pIDChangesLog = 0;

    m_bPaste = false;
}

void UMLScene::removeAssoc(AssociationWidget* pAssoc)
{
    if (!pAssoc)
        return;

    emit sigAssociationRemoved(pAssoc);

    pAssoc->cleanup();
    m_AssociationList.removeAll(pAssoc);
    delete pAssoc;
    m_pDoc->setModified();
}

void UMLScene::removeAssocInViewAndDoc(AssociationWidget* a)
{
    // For umbrello 1.2, UMLAssociations can only be removed in two ways:
    // 1. Right click on the assocwidget in the view and select Delete
    // 2. Go to the Class Properties page, select Associations, right click
    //    on the association and select Delete
    if (!a)
        return;
    if (a->getAssocType() == at_Containment) {
        UMLObject *objToBeMoved = a->getWidget(B)->getUMLObject();
        if (objToBeMoved != NULL) {
            UMLListView *lv = UMLApp::app()->getListView();
            lv->moveObject(objToBeMoved->getID(),
                           Model_Utils::convert_OT_LVT(objToBeMoved),
                           lv->theLogicalView());
            // UMLListView::moveObject() will delete the containment
            // AssociationWidget via UMLScene::updateContainment().
        } else {
            uDebug() << "removeAssocInViewAndDoc(containment): "
                     << "objB is NULL" << endl;
        }
    } else {
        // Remove assoc in doc.
        m_pDoc->removeAssociation(a->getAssociation());
        // Remove assoc in view.
        removeAssoc(a);
    }
}

/** Removes all the associations related to Widget */
void UMLScene::removeAssociations(NewUMLRectWidget* Widget)
{

    foreach(AssociationWidget* assocwidget, m_AssociationList) {
        if (assocwidget->contains(Widget)) {
            removeAssoc(assocwidget);
        }
    }
}

void UMLScene::selectAssociations(bool bSelect)
{

    foreach(AssociationWidget* assocwidget, m_AssociationList) {
        if (bSelect &&
            assocwidget->getWidget(A) && assocwidget->getWidget(A)->isSelected() &&
            assocwidget->getWidget(B) && assocwidget->getWidget(B)->isSelected()) {
            assocwidget->setSelected(true);
        } else {
            assocwidget->setSelected(false);
        }
    }//end foreach
}

void UMLScene::getWidgetAssocs(UMLObject* Obj, AssociationWidgetList & Associations)
{
    if (! Obj)
        return;

    foreach(AssociationWidget* assocwidget, m_AssociationList) {
        if (assocwidget->getWidget(A)->getUMLObject() == Obj ||
            assocwidget->getWidget(B)->getUMLObject() == Obj)
            Associations.append(assocwidget);
    }//end foreach

}

void UMLScene::removeAllAssociations()
{
    //Remove All association widgets

    foreach(AssociationWidget* assocwidget, m_AssociationList) {
        removeAssoc(assocwidget);
    }

    // Porting to QList from QPtrList which doesn't support autodelete
    //m_AssociationList.clear();
    qDeleteAll(m_AssociationList);
    m_AssociationList.clear();
}


void UMLScene::removeAllWidgets()
{
    // Remove widgets.

    foreach(NewUMLRectWidget* temp , m_WidgetList) {
        // I had to take this condition back in, else umbrello
        // crashes on exit. Still to be analyzed.  --okellogg
        if (!(temp->getBaseType() == wt_Text &&
              ((FloatingTextWidget *)temp)->getRole() != tr_Floating)) {
            removeWidget(temp);
        }
    }
    // Porting to QList from QPtrList which doesn't support autodelete
    //m_WidgetList.clear();
    qDeleteAll(m_AssociationList);
    m_AssociationList.clear();
}

void UMLScene::showDocumentation(UMLObject * object, bool overwrite)
{
    UMLApp::app()->getDocWindow()->showDocumentation(object, overwrite);
    m_bChildDisplayedDoc = true;
}

void UMLScene::showDocumentation(NewUMLRectWidget * widget, bool overwrite)
{
    UMLApp::app()->getDocWindow()->showDocumentation(widget, overwrite);
    m_bChildDisplayedDoc = true;
}

void UMLScene::showDocumentation(AssociationWidget * widget, bool overwrite)
{
    UMLApp::app()->getDocWindow()->showDocumentation(widget, overwrite);
    m_bChildDisplayedDoc = true;
}

void UMLScene::updateDocumentation(bool clear)
{
    UMLApp::app()->getDocWindow()->updateDocumentation(clear);
}

void UMLScene::updateContainment(UMLCanvasObject *self)
{
    if (self == NULL)
        return;
    // See if the object has a widget representation in this view.
    // While we're at it, also see if the new parent has a widget here.
    NewUMLRectWidget *selfWidget = NULL, *newParentWidget = NULL;
    UMLPackage *newParent = self->getUMLPackage();
    foreach(NewUMLRectWidget* w, m_WidgetList) {
        UMLObject *o = w->getUMLObject();
        if (o == self)
            selfWidget = w;
        else if (newParent != NULL && o == newParent)
            newParentWidget = w;
    }
    if (selfWidget == NULL)
        return;
    // Remove possibly obsoleted containment association.
    foreach(AssociationWidget* a, m_AssociationList) {
        if (a->getAssocType() != Uml::at_Containment)
            continue;
        // Container is at role A, containee at B.
        // We only look at association for which we are B.
        NewUMLRectWidget *wB = a->getWidget(B);
        UMLObject *roleBObj = wB->getUMLObject();
        if (roleBObj != self)
            continue;
        NewUMLRectWidget *wA = a->getWidget(A);
        UMLObject *roleAObj = wA->getUMLObject();
        if (roleAObj == newParent) {
            // Wow, all done. Great!
            return;
        }
        removeAssoc(a);  // AutoDelete is true
        // It's okay to break out because there can only be a single
        // containing object.
        break;
    }
    if (newParentWidget == NULL)
        return;
    // Create the new containment association.
    AssociationWidget *a = new AssociationWidget(this, newParentWidget,
                                                 Uml::at_Containment, selfWidget);
    a->calculateEndingPoints();
    a->setActivated(true);
    m_AssociationList.append(a);
}

void UMLScene::createAutoAssociations(NewUMLRectWidget * widget)
{
    if (widget == NULL ||
        (m_Type != Uml::dt_Class &&
         m_Type != Uml::dt_Component &&
         m_Type != Uml::dt_Deployment
         && m_Type != Uml::dt_EntityRelationship))
        return;
    // Recipe:
    // If this widget has an underlying UMLCanvasObject then
    //   for each of the UMLCanvasObject's UMLAssociations
    //     if umlassoc's "other" role has a widget representation on this view then
    //       if the AssocWidget does not already exist then
    //         if the assoc type is permitted in the current diagram type then
    //           create the AssocWidget
    //         end if
    //       end if
    //     end if
    //   end loop
    //   Do createAutoAttributeAssociations()
    //   if this object is capable of containing nested objects then
    //     for each of the object's containedObjects
    //       if the containedObject has a widget representation on this view then
    //         if the containedWidget is not physically located inside this widget
    //           create the containment AssocWidget
    //         end if
    //       end if
    //     end loop
    //   end if
    //   if the UMLCanvasObject has a parentPackage then
    //     if the parentPackage has a widget representation on this view then
    //       create the containment AssocWidget
    //     end if
    //   end if
    // end if
    UMLObject *tmpUmlObj = widget->getUMLObject();
    if (tmpUmlObj == NULL)
        return;
    UMLCanvasObject *umlObj = dynamic_cast<UMLCanvasObject*>(tmpUmlObj);
    if (umlObj == NULL)
        return;
    const UMLAssociationList& umlAssocs = umlObj->getAssociations();

    Uml::IDType myID = umlObj->getID();
    foreach(UMLAssociation* assoc , umlAssocs) {
        UMLCanvasObject *other = NULL;
        UMLObject *roleAObj = assoc->getObject(A);
        if (roleAObj == NULL) {
            uDebug() << "roleA object is NULL at UMLAssoc "
                     << ID2STR(assoc->getID());
            continue;
        }
        UMLObject *roleBObj = assoc->getObject(B);
        if (roleBObj == NULL) {
            uDebug() << "roleB object is NULL at UMLAssoc "
                     << ID2STR(assoc->getID());
            continue;
        }
        if (roleAObj->getID() == myID) {
            other = static_cast<UMLCanvasObject*>(roleBObj);
        } else if (roleBObj->getID() == myID) {
            other = static_cast<UMLCanvasObject*>(roleAObj);
        } else {
            uDebug() << "Can not find own object "
                     << ID2STR(myID) << " in UMLAssoc "
                     << ID2STR(assoc->getID());
            continue;
        }
        // Now that we have determined the "other" UMLObject, seek it in
        // this view's UMLWidgets.
        Uml::IDType otherID = other->getID();

        bool breakFlag = false;
        NewUMLRectWidget* pOtherWidget = 0;
        foreach(pOtherWidget ,  m_WidgetList) {
            if (pOtherWidget->getID() == otherID) {
                breakFlag = true;
                break;
            }
        }
        if (!breakFlag)
            continue;
        // Both objects are represented in this view:
        // Assign widget roles as indicated by the UMLAssociation.
        NewUMLRectWidget *widgetA, *widgetB;
        if (myID == roleAObj->getID()) {
            widgetA = widget;
            widgetB = pOtherWidget;
        } else {
            widgetA = pOtherWidget;
            widgetB = widget;
        }
        // Check that the assocwidget does not already exist.
        Uml::Association_Type assocType = assoc->getAssocType();
        AssociationWidget * assocwidget = findAssocWidget(assocType, widgetA, widgetB);
        if (assocwidget) {
            assocwidget->calculateEndingPoints();  // recompute assoc lines
            continue;
        }
        // Check that the assoc is allowed.
        if (!AssocRules::allowAssociation(assocType, widgetA, widgetB, false)) {
            uDebug() << "not transferring assoc "
                     << "of type " << assocType;
            continue;
        }

        // Create the AssociationWidget.
        assocwidget = new AssociationWidget(this);
        assocwidget->setWidget(widgetA, A);
        assocwidget->setWidget(widgetB, B);
        assocwidget->setAssocType(assocType);
        assocwidget->setUMLObject(assoc);
        // Call calculateEndingPoints() before setting the FloatingTexts
        // because their positions are computed according to the
        // assocwidget line positions.
        assocwidget->calculateEndingPoints();
        assocwidget->syncToModel();
        assocwidget->setActivated(true);
        if (! addAssociation(assocwidget))
            delete assocwidget;
    }

    createAutoAttributeAssociations(widget);

    if (m_Type == Uml::dt_EntityRelationship) {
        createAutoConstraintAssociations(widget);
    }

    // if this object is capable of containing nested objects then
    Uml::Object_Type t = umlObj->getBaseType();
    if (t == ot_Package || t == ot_Class || t == ot_Interface || t == ot_Component) {
        // for each of the object's containedObjects
        UMLPackage *umlPkg = static_cast<UMLPackage*>(umlObj);
        UMLObjectList lst = umlPkg->containedObjects();
        foreach(UMLObject* obj,  lst) {
            // if the containedObject has a widget representation on this view then
            Uml::IDType id = obj->getID();
            foreach(NewUMLRectWidget *w , m_WidgetList) {
                if (w->getID() != id)
                    continue;
                // if the containedWidget is not physically located inside this widget
                if (widget->sceneBoundingRect().contains(w->sceneBoundingRect()))
                    continue;
                // create the containment AssocWidget
                AssociationWidget *a = new AssociationWidget(this, widget,
                                                             at_Containment, w);
                a->calculateEndingPoints();
                a->setActivated(true);
                if (! addAssociation(a))
                    delete a;
            }
        }
    }
    // if the UMLCanvasObject has a parentPackage then
    UMLPackage *parent = umlObj->getUMLPackage();
    if (parent == NULL)
        return;
    // if the parentPackage has a widget representation on this view then
    Uml::IDType pkgID = parent->getID();

    bool breakFlag = false;
    NewUMLRectWidget* pWidget = 0;
    foreach(pWidget , m_WidgetList) {
        if (pWidget->getID() == pkgID) {
            breakFlag = true;
            break;
        }
    }
    if (!breakFlag || pWidget->sceneBoundingRect().contains(widget->sceneBoundingRect()))
        return;
    // create the containment AssocWidget
    AssociationWidget *a = new AssociationWidget(this, pWidget, at_Containment, widget);
    a->calculateEndingPoints();
    a->setActivated(true);
    if (! addAssociation(a))
        delete a;
}

void UMLScene::createAutoAttributeAssociations(NewUMLRectWidget *widget)
{
    if (widget == NULL || m_Type != Uml::dt_Class || !m_Options.classState.showAttribAssocs)
        return;

    // Pseudocode:
    //   if the underlying model object is really a UMLClassifier then
    //     for each of the UMLClassifier's UMLAttributes
    //       if the attribute type has a widget representation on this view then
    //         if the AssocWidget does not already exist then
    //           if the current diagram type permits compositions then
    //             create a composition AssocWidget
    //           end if
    //         end if
    //       end if
    //       if the attribute type is a Datatype then
    //         if the Datatype is a reference (pointer) type then
    //           if the referenced type has a widget representation on this view then
    //             if the AssocWidget does not already exist then
    //               if the current diagram type permits aggregations then
    //                 create an aggregation AssocWidget from the ClassifierWidget to the
    //                                                 widget of the referenced type
    //               end if
    //             end if
    //           end if
    //         end if
    //       end if
    //     end loop
    //   end if
    //
    // Implementation:
    UMLObject *tmpUmlObj = widget->getUMLObject();
    if (tmpUmlObj == NULL)
        return;
    // if the underlying model object is really a UMLClassifier then
    if (tmpUmlObj->getBaseType() == Uml::ot_Datatype) {
        UMLClassifier *dt = static_cast<UMLClassifier*>(tmpUmlObj);
        while (dt->originType() != NULL) {
            tmpUmlObj = dt->originType();
            if (tmpUmlObj->getBaseType() != Uml::ot_Datatype)
                break;
            dt = static_cast<UMLClassifier*>(tmpUmlObj);
        }
    }
    if (tmpUmlObj->getBaseType() != Uml::ot_Class)
        return;
    UMLClassifier * klass = static_cast<UMLClassifier*>(tmpUmlObj);
    // for each of the UMLClassifier's UMLAttributes
    UMLAttributeList attrList = klass->getAttributeList();
    foreach(UMLAttribute* attr, attrList) {
        createAutoAttributeAssociation(attr->getType(), attr, widget);
        /*
         * The following code from attachment 19935 of http://bugs.kde.org/140669
         * creates Aggregation/Composition to the template parameters.
         * The current solution uses Dependency instead, see handling of template
         * instantiation at Import_Utils::createUMLObject().
         UMLClassifierList templateList = attr->getTemplateParams();
         for (UMLClassifierListIt it(templateList); it.current(); ++it) {
         createAutoAttributeAssociation(it,attr,widget);
         }
        */
    }
}

void UMLScene::createAutoAttributeAssociation(UMLClassifier *type, UMLAttribute *attr,
                                              NewUMLRectWidget *widget /*, UMLClassifier * klass*/)
{
    if (type == NULL) {
        // uDebug() << klass->getName() << ": type is NULL for "
        //     << "attribute " << attr->getName() << endl;
        return;
    }
    Uml::Association_Type assocType = Uml::at_Composition;
    NewUMLRectWidget *w = findWidget(type->getID());
    AssociationWidget *aw = NULL;
    // if the attribute type has a widget representation on this view
    if (w) {
        aw = findAssocWidget(widget, w, attr->getName());
        if (aw == NULL &&
            // if the current diagram type permits compositions
            AssocRules::allowAssociation(assocType, widget, w, false)) {
            // Create a composition AssocWidget, or, if the attribute type is
            // stereotyped <<CORBAInterface>>, create a UniAssociation widget.
            if (type->getStereotype() == "CORBAInterface")
                assocType = at_UniAssociation;

            AssociationWidget *a = new AssociationWidget(this, widget, assocType, w, attr);
            a->calculateEndingPoints();
            a->setVisibility(attr->getVisibility(), B);
            /*
              if (assocType == at_Aggregation || assocType == at_UniAssociation)
              a->setMulti("0..1", B);
            */
            a->setRoleName(attr->getName(), B);
            a->setActivated(true);
            if (! addAssociation(a))
                delete a;
        }
    }
    // if the attribute type is a Datatype then
    if (type->getBaseType() == ot_Datatype) {
        UMLClassifier *dt = static_cast<UMLClassifier*>(type);
        // if the Datatype is a reference (pointer) type
        if (dt->isReference()) {
            //Uml::Association_Type assocType = Uml::at_Composition;
            UMLClassifier *c = dt->originType();
            NewUMLRectWidget *w = c ? findWidget(c->getID()) : 0;
            // if the referenced type has a widget representation on this view
            if (w) {
                aw = findAssocWidget(widget, w, attr->getName());
                if (aw == NULL &&
                    // if the current diagram type permits aggregations
                    AssocRules::allowAssociation(at_Aggregation, widget, w, false)) {
                    // create an aggregation AssocWidget from the ClassifierWidget
                    // to the widget of the referenced type

                    AssociationWidget *a = new AssociationWidget
                        (this, widget, at_Aggregation, w, attr);
                    a->calculateEndingPoints();
                    a->setVisibility(attr->getVisibility(), B);
                    //a->setChangeability(true, B);
                    a->setMulti("0..1", B);
                    a->setRoleName(attr->getName(), B);
                    a->setActivated(true);
                    if (! addAssociation(a))
                        delete a;
                }
            }
        }
    }
}

void UMLScene::createAutoConstraintAssociations(NewUMLRectWidget *widget)
{
    if (widget == NULL || m_Type != Uml::dt_EntityRelationship)
        return;

    // Pseudocode:
    //   if the underlying model object is really a UMLEntity then
    //     for each of the UMLEntity's UMLForeignKeyConstraint's
    //       if the attribute type has a widget representation on this view then
    //         if the AssocWidget does not already exist then
    //           if the current diagram type permits relationships then
    //             create a relationship AssocWidget
    //           end if
    //         end if
    //       end if

    UMLObject *tmpUmlObj = widget->getUMLObject();
    if (tmpUmlObj == NULL)
        return;
    // check if the underlying model object is really a UMLEntity
    UMLCanvasObject *umlObj = dynamic_cast<UMLCanvasObject*>(tmpUmlObj);
    if (umlObj == NULL)
        return;
    // finished checking whether this widget has a UMLCanvas Object

    if (tmpUmlObj->getBaseType() != Uml::ot_Entity)
        return;
    UMLEntity *entity = static_cast<UMLEntity*>(tmpUmlObj);

    // for each of the UMLEntity's UMLForeignKeyConstraints
    UMLClassifierListItemList constrList = entity->getFilteredList(Uml::ot_ForeignKeyConstraint);

    foreach(UMLClassifierListItem* cli, constrList) {
        UMLEntityConstraint *eConstr = static_cast<UMLEntityConstraint*>(cli);

        UMLForeignKeyConstraint* fkc = static_cast<UMLForeignKeyConstraint*>(eConstr);
        if (fkc == NULL) {
            return;
        }

        UMLEntity* refEntity = fkc->getReferencedEntity();
        if (refEntity == NULL) {
            return;
        }

        createAutoConstraintAssociation(refEntity , fkc , widget);

    }
}

void UMLScene::createAutoConstraintAssociation(UMLEntity* refEntity, UMLForeignKeyConstraint* fkConstraint, NewUMLRectWidget* widget)
{

    if (refEntity == NULL) {
        return;
    }

    Uml::Association_Type assocType = Uml::at_Relationship;
    NewUMLRectWidget *w = findWidget(refEntity->getID());
    AssociationWidget *aw = NULL;

    if (w) {
        aw = findAssocWidget(widget, w, fkConstraint->getName());
        if (aw == NULL &&
            // if the current diagram type permits relationships
            AssocRules::allowAssociation(assocType, widget, w, false)) {

            AssociationWidget *a = new AssociationWidget(this, widget, assocType, w);
            a->setUMLObject(fkConstraint);
            a->calculateEndingPoints();
            //a->setVisibility(attr->getVisibility(), B);
            a->setRoleName(fkConstraint->getName(), B);
            a->setActivated(true);
            if (! addAssociation(a))
                delete a;
        }
    }

}

void UMLScene::findMaxBoundingRectangle(const FloatingTextWidget* ft, qreal& px, qreal& py, qreal& qx, qreal& qy)
{
    if (ft == NULL || !ft->isVisible())
        return;

    qreal x = ft->getX();
    qreal y = ft->getY();
    qreal x1 = x + ft->getWidth() - 1;
    qreal y1 = y + ft->getHeight() - 1;

    if (px == -1 || x < px)
        px = x;
    if (py == -1 || y < py)
        py = y;
    if (qx == -1 || x1 > qx)
        qx = x1;
    if (qy == -1 || y1 > qy)
        qy = y1;
}

void UMLScene::copyAsImage(QPixmap*& pix)
{
    //get the smallest rect holding the diagram
    // [PORT]
    QRect rect = getDiagramRect().toRect();
    QPixmap diagram(rect.width(), rect.height());

    //only draw what is selected
    m_bDrawSelectedOnly = true;
    selectAssociations(true);
    getDiagram(rect, diagram);

    //now get the selection cut
    qreal px = -1, py = -1, qx = -1, qy = -1;

    //first get the smallest rect holding the widgets
    foreach(NewUMLRectWidget* temp , selectedWidgets()) {
        qreal x = temp->getX();
        qreal y = temp->getY();
        qreal x1 = x + temp->getWidth() - 1;
        qreal y1 = y + temp->getHeight() - 1;
        if (px == -1 || x < px) {
            px = x;
        }
        if (py == -1 || y < py) {
            py = y;
        }
        if (qx == -1 || x1 > qx) {
            qx = x1;
        }
        if (qy == -1 || y1 > qy) {
            qy = y1;
        }
    }

    //also take into account any text lines in assocs or messages

    //get each type of associations
    //This needs to be reimplemented to increase the rectangle
    //if a part of any association is not included
    foreach(AssociationWidget *a , m_AssociationList) {
        if (! a->getSelected())
            continue;
        const FloatingTextWidget* multiA = const_cast<FloatingTextWidget*>(a->getMultiWidget(A));
        const FloatingTextWidget* multiB = const_cast<FloatingTextWidget*>(a->getMultiWidget(B));
        const FloatingTextWidget* roleA = const_cast<FloatingTextWidget*>(a->getRoleWidget(A));
        const FloatingTextWidget* roleB = const_cast<FloatingTextWidget*>(a->getRoleWidget(B));
        const FloatingTextWidget* changeA = const_cast<FloatingTextWidget*>(a->getChangeWidget(A));
        const FloatingTextWidget* changeB = const_cast<FloatingTextWidget*>(a->getChangeWidget(B));
        findMaxBoundingRectangle(multiA, px, py, qx, qy);
        findMaxBoundingRectangle(multiB, px, py, qx, qy);
        findMaxBoundingRectangle(roleA, px, py, qx, qy);
        findMaxBoundingRectangle(roleB, px, py, qx, qy);
        findMaxBoundingRectangle(changeA, px, py, qx, qy);
        findMaxBoundingRectangle(changeB, px, py, qx, qy);
    }//end foreach

    QRect imageRect;  //area with respect to getDiagramRect()
    //i.e. all widgets on the canvas.  Was previously with
    //respect to whole canvas

    imageRect.setLeft(px - rect.left());
    imageRect.setTop(py - rect.top());
    imageRect.setRight(qx - rect.left());
    imageRect.setBottom(qy - rect.top());

    pix = new QPixmap(imageRect.width(), imageRect.height());
    QPainter output(pix);
    output.drawPixmap(QPoint(0, 0), diagram, imageRect);
    m_bDrawSelectedOnly = false;
}

UMLView* UMLScene::activeView() const
{
    UMLView *view = 0;
    if(!views().isEmpty()) {
        view = dynamic_cast<UMLView*>(views().first());
    }
    return view;
}

void UMLScene::setMenu()
{
    slotRemovePopupMenu();
    ListPopupMenu::Menu_Type menu = ListPopupMenu::mt_Undefined;
    switch (getType()) {
    case dt_Class:
        menu = ListPopupMenu::mt_On_Class_Diagram;
        break;

    case dt_UseCase:
        menu = ListPopupMenu::mt_On_UseCase_Diagram;
        break;

    case dt_Sequence:
        menu = ListPopupMenu::mt_On_Sequence_Diagram;
        break;

    case dt_Collaboration:
        menu = ListPopupMenu::mt_On_Collaboration_Diagram;
        break;

    case dt_State:
        menu = ListPopupMenu::mt_On_State_Diagram;
        break;

    case dt_Activity:
        menu = ListPopupMenu::mt_On_Activity_Diagram;
        break;

    case dt_Component:
        menu = ListPopupMenu::mt_On_Component_Diagram;
        break;

    case dt_Deployment:
        menu = ListPopupMenu::mt_On_Deployment_Diagram;
        break;

    case dt_EntityRelationship:
        menu = ListPopupMenu::mt_On_EntityRelationship_Diagram;
        break;

    default:
        uWarning() << "unknown diagram type " << getType();
        menu = ListPopupMenu::mt_Undefined;
        break;
    }//end switch
    if (menu != ListPopupMenu::mt_Undefined) {
        // uDebug() << "create popup for Menu_Type " << menu;
        m_pMenu = new ListPopupMenu(activeView(), menu, activeView());
        connect(m_pMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotMenuSelection(QAction*)));

        // [PORT] Calculate using activeView once its implementation is done.
        QPoint point = m_Pos.toPoint();
        m_pMenu->popup(point);
    }
}

void UMLScene::slotRemovePopupMenu()
{
    delete m_pMenu;
    m_pMenu = 0;
}

void UMLScene::slotMenuSelection(QAction* action)
{
    ListPopupMenu::Menu_Type sel = ListPopupMenu::mt_Undefined;
    if (m_pMenu != NULL) {  // popup from this class
        sel = m_pMenu->getMenuType(action);
    } else { // popup from umldoc
        sel = m_pDoc->getPopupMenuSelection(action);
    }
    switch (sel) {
    case ListPopupMenu::mt_Undo:
        UMLApp::app()->undo();
        break;

    case ListPopupMenu::mt_Redo:
        UMLApp::app()->redo();
        break;

    case ListPopupMenu::mt_Clear:
        clearDiagram();
        break;

    case ListPopupMenu::mt_Export_Image:
        m_pImageExporter->exportView();
        break;

    case ListPopupMenu::mt_FloatText: {
        FloatingTextWidget* ft = new FloatingTextWidget(this);
        ft->changeTextDlg();
        //if no text entered delete
        if (!FloatingTextWidget::isTextValid(ft->getText())) {
            delete ft;
        } else {
            ft->setID(UniqueID::gen());
            setupNewWidget(ft);
        }
    }
        break;

    case ListPopupMenu::mt_UseCase:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_UseCase);
        break;

    case ListPopupMenu::mt_Actor:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_Actor);
        break;

    case ListPopupMenu::mt_Class:
    case ListPopupMenu::mt_Object:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_Class);
        break;

    case ListPopupMenu::mt_Package:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_Package);
        break;

    case ListPopupMenu::mt_Component:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_Component);
        break;

    case ListPopupMenu::mt_Node:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_Node);
        break;

    case ListPopupMenu::mt_Artifact:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_Artifact);
        break;

    case ListPopupMenu::mt_Interface:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_Interface);
        break;

    case ListPopupMenu::mt_Enum:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_Enum);
        break;

    case ListPopupMenu::mt_Entity:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_Entity);
        break;

    case ListPopupMenu::mt_Category:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_Category);
        break;

    case ListPopupMenu::mt_Datatype:
        m_bCreateObject = true;
        Object_Factory::createUMLObject(ot_Datatype);
        break;

    case ListPopupMenu::mt_Cut:
        //FIXME make this work for diagram's right click menu
        if (selectedWidgets().count() &&
            UMLApp::app()->editCutCopy(true)) {
            deleteSelection();
            m_pDoc->setModified(true);
        }
        break;

    case ListPopupMenu::mt_Copy:
        //FIXME make this work for diagram's right click menu
        selectedWidgets().count() && UMLApp::app()->editCutCopy(true);
        break;

    case ListPopupMenu::mt_Paste:
        m_PastePoint = m_Pos;
        m_Pos.setX(2000);
        m_Pos.setY(2000);
        UMLApp::app()->slotEditPaste();

        m_PastePoint.setX(0);
        m_PastePoint.setY(0);
        break;

    case ListPopupMenu::mt_Initial_State: {
        StateWidget* state = new StateWidget(StateWidget::Initial);
		addItem(state);
        setupNewWidget(state);
    }
        break;

    case ListPopupMenu::mt_End_State: {
        StateWidget* state = new StateWidget(StateWidget::End);
		addItem(state);
        setupNewWidget(state);
    }
        break;

    case ListPopupMenu::mt_State: {
        bool ok = false;
        QString name = KInputDialog::getText(i18n("Enter State Name"),
                                             i18n("Enter the name of the new state:"),
                                             i18n("new state"), &ok, UMLApp::app());
        if (ok) {
            StateWidget* state = new StateWidget();
            state->setName(name);
			addItem(state);
            setupNewWidget(state);
        }
    }
        break;

    case ListPopupMenu::mt_Initial_Activity: {
        ActivityWidget* activity = new ActivityWidget(ActivityWidget::Initial);
        setupNewWidget(activity);
    }
        break;


    case ListPopupMenu::mt_End_Activity: {
        ActivityWidget* activity = new ActivityWidget(ActivityWidget::End);
        setupNewWidget(activity);
    }
        break;

    case ListPopupMenu::mt_Branch: {
        ActivityWidget* activity = new ActivityWidget(ActivityWidget::Branch);
        setupNewWidget(activity);
    }
        break;

    case ListPopupMenu::mt_Activity: {
        bool ok = false;
        QString name = KInputDialog::getText(i18n("Enter Activity Name"),
                                             i18n("Enter the name of the new activity:"),
                                             i18n("new activity"), &ok, UMLApp::app());
        if (ok) {
            ActivityWidget* activity = new ActivityWidget(ActivityWidget::Normal);
            activity->setName(name);
            setupNewWidget(activity);
        }
    }
        break;

    case ListPopupMenu::mt_SnapToGrid:
        toggleSnapToGrid();
        m_pDoc->setModified();
        break;

    case ListPopupMenu::mt_ShowSnapGrid:
        toggleShowGrid();
        m_pDoc->setModified();
        break;

    case ListPopupMenu::mt_Properties:
        if (showPropDialog() == true)
            m_pDoc->setModified();
        break;

    case ListPopupMenu::mt_Delete:
        m_pDoc->removeDiagram(getID());
        break;

    case ListPopupMenu::mt_Rename: {
        bool ok = false;
        QString name = KInputDialog::getText(i18n("Enter Diagram Name"),
                                             i18n("Enter the new name of the diagram:"),
                                             getName(), &ok, UMLApp::app());
        if (ok) {
            setName(name);
            // [PORT]
            // m_pDoc->signalDiagramRenamed(this);
        }
    }
        break;

    default:
        uWarning() << "unknown ListPopupMenu::Menu_Type " << sel;
        break;
    }
}

void UMLScene::slotCutSuccessful()
{
    if (m_bStartedCut) {
        deleteSelection();
        m_bStartedCut = false;
    }
}

void UMLScene::slotShowView()
{
    m_pDoc->changeCurrentView(getID());
}

QPointF UMLScene::getPastePoint()
{
    return m_PastePoint - m_Pos;
}

void UMLScene::resetPastePoint()
{
    m_PastePoint = m_Pos;
}

qreal UMLScene::snappedX(qreal _x)
{
    int x = (int)_x;
    if (getSnapToGrid()) {
        int gridX = getSnapX();
        int modX = x % gridX;
        x -= modX;
        if (modX >= gridX / 2)
            x += gridX;
    }
    return x;
}

qreal UMLScene::snappedY(qreal _y)
{
    int y = (int)_y;
    if (getSnapToGrid()) {
        int gridY = getSnapY();
        int modY = y % gridY;
        y -= modY;
        if (modY >= gridY / 2)
            y += gridY;
    }
    return y;
}

bool UMLScene::showPropDialog()
{
    // Be explict to avoid confusion
    QWidget *parent = activeView();
    UMLViewDialog dlg(parent, this);
    if (dlg.exec()) {
        return true;
    }
    return false;
}


QFont UMLScene::getFont() const
{
    return m_Options.uiState.font;
}

void UMLScene::setFont(QFont font, bool changeAllWidgets /* = false */)
{
    m_Options.uiState.font = font;
    if (!changeAllWidgets)
        return;
    foreach(NewUMLRectWidget* w, m_WidgetList) {
        w->setFont(font);
    }
}

void UMLScene::setClassWidgetOptions(ClassOptionsPage * page)
{
    foreach(NewUMLRectWidget* pWidget , m_WidgetList) {
        Uml::Widget_Type wt = pWidget->getBaseType();
        if (wt == Uml::wt_Class || wt == Uml::wt_Interface) {
            page->setWidget(static_cast<ClassifierWidget *>(pWidget));
            page->updateUMLWidget();
        }
    }
}


void UMLScene::checkSelections()
{
    NewUMLRectWidget * pWA = 0, * pWB = 0;
    //check messages
    foreach(NewUMLRectWidget *pTemp , selectedWidgets()) {
        if (pTemp->getBaseType() == wt_Message && pTemp->isSelected()) {
            MessageWidget * pMessage = static_cast<MessageWidget *>(pTemp);
            pWA = pMessage->getWidget(A);
            pWB = pMessage->getWidget(B);
            if (!pWA->isSelected()) {
                // [PORT] Was setSelectedFlag before
                pWA->setSelected(true);
            }
            if (!pWB->isSelected()) {
                // [PORT] Was setSelectedFlag before
                pWB->setSelected(true);
            }
        }//end if
    }//end for
    //check Associations

    foreach(AssociationWidget *pAssoc , m_AssociationList) {
        if (pAssoc->getSelected()) {
            pWA = pAssoc->getWidget(A);
            pWB = pAssoc->getWidget(B);
            if (!pWA->isSelected()) {
                // [PORT] Was setSelectedFlag before
                pWA->setSelected(true);
            }
            if (!pWB->isSelected()) {
                // [PORT] Was setSelectedFlag before
                pWB->setSelected(true);
            }
        }//end if
    }//end foreach
}

bool UMLScene::checkUniqueSelection()
{
    // if there are no selected items, we return true
    if (selectedWidgets().isEmpty())
        return true;

    // get the first item and its base type
    NewUMLRectWidget * pTemp = (NewUMLRectWidget *) selectedWidgets().first();
    Widget_Type tmpType = pTemp->getBaseType();

    // check all selected items, if they have the same BaseType
    foreach(pTemp , selectedWidgets()) {
        if (pTemp->getBaseType() != tmpType) {
            return false; // the base types are different, the list is not unique
        }
    } // for (through all selected items)

    return true; // selected items are unique
}

void UMLScene::callBaseMouseMethod(QGraphicsSceneMouseEvent *event)
{
    switch(event->type())
    {
    case QEvent::GraphicsSceneMousePress: QGraphicsScene::mousePressEvent(event); break;
    case QEvent::GraphicsSceneMouseMove: QGraphicsScene::mouseMoveEvent(event); break;
    case QEvent::GraphicsSceneMouseRelease: QGraphicsScene::mouseReleaseEvent(event); break;
    case QEvent::GraphicsSceneMouseDoubleClick: QGraphicsScene::mouseDoubleClickEvent(event); break;

    default: ;
    }
}

void UMLScene::drawBackground(QPainter *p, const QRectF &rect)
{
    QGraphicsScene::drawBackground(p, rect);
    if(!getShowSnapGrid()) {
        return;
    }
    //TODO : Optimize by drawing only contents within rect.
    p->setPen(Qt::gray);
    int gridX = getSnapX();
    int gridY = getSnapY();
    int numX = width() / gridX;
    int numY = height() / gridY;
    for( int x = 0; x <= numX; x++ )
        for( int y = 0; y < numY; y++ )
            p->drawPoint( x * gridX, y * gridY );
}
void UMLScene::clearDiagram()
{
    if (KMessageBox::Continue == KMessageBox::warningContinueCancel(activeView(),
                                                                    i18n("You are about to delete "
                                                                         "the entire diagram.\nAre you sure?"),
                                                                    i18n("Delete Diagram?"),
                                                                    KGuiItem(i18n("&Delete"), "edit-delete"))) {
        removeAllWidgets();
    }
}

void UMLScene::toggleSnapToGrid()
{
    setSnapToGrid(!getSnapToGrid());
}

void UMLScene::toggleSnapComponentSizeToGrid()
{
    setSnapComponentSizeToGrid(!getSnapComponentSizeToGrid());
}

void UMLScene::toggleShowGrid()
{
    setShowSnapGrid(!getShowSnapGrid());
}

void UMLScene::setSnapToGrid(bool bSnap)
{
    m_bUseSnapToGrid = bSnap;
    emit sigSnapToGridToggled(getSnapToGrid());
}

void UMLScene::setSnapComponentSizeToGrid(bool bSnap)
{
    m_bUseSnapComponentSizeToGrid = bSnap;
    updateComponentSizes();
    emit sigSnapComponentSizeToGridToggled(getSnapComponentSizeToGrid());
}

bool UMLScene::getShowSnapGrid() const
{
    return m_bShowSnapGrid;
}

void UMLScene::setShowSnapGrid(bool bShow)
{
    m_bShowSnapGrid = bShow;
    update();
    emit sigShowGridToggled(getShowSnapGrid());
}

bool UMLScene::getShowOpSig() const
{
    return m_Options.classState.showOpSig;
}

void UMLScene::setShowOpSig(bool bShowOpSig)
{
    m_Options.classState.showOpSig = bShowOpSig;
}

void UMLScene::fileLoaded()
{
    resizeCanvasToItems();
}

void UMLScene::resizeCanvasToItems()
{
    QRectF rect = itemsBoundingRect();
    //Make sure (0,0) is in the topLeft
    rect.setTopLeft(QPointF(0, 0));
    setSceneRect(rect);
}

void UMLScene::updateComponentSizes()
{
    // update sizes of all components

    foreach(NewUMLRectWidget *obj , m_WidgetList) {
        obj->updateComponentSize();
    }
}

/**
 * Force the widget font metrics to be updated next time
 * the widgets are drawn.
 * This is necessary because the widget size might depend on the
 * font metrics and the font metrics might change for different
 * QPainter, i.e. font metrics for Display font and Printer font are
 * usually different.
 * Call this when you change the QPainter.
 */
void UMLScene::forceUpdateWidgetFontMetrics(QPainter * painter)
{
    foreach(NewUMLRectWidget *obj , m_WidgetList) {
        obj->forceUpdateFontMetrics(painter);
    }
}

void UMLScene::saveToXMI(QDomDocument & qDoc, QDomElement & qElement)
{
    QDomElement viewElement = qDoc.createElement("diagram");
    viewElement.setAttribute("xmi.id", ID2STR(m_nID));
    viewElement.setAttribute("name", getName());
    viewElement.setAttribute("type", m_Type);
    viewElement.setAttribute("documentation", m_Documentation);
    //optionstate uistate
    viewElement.setAttribute("fillcolor", m_Options.uiState.fillColor.name());
    viewElement.setAttribute("linecolor", m_Options.uiState.lineColor.name());
    viewElement.setAttribute("linewidth", m_Options.uiState.lineWidth);
    viewElement.setAttribute("usefillcolor", m_Options.uiState.useFillColor);
    viewElement.setAttribute("font", m_Options.uiState.font.toString());
    //optionstate classstate
    viewElement.setAttribute("showattsig", m_Options.classState.showAttSig);
    viewElement.setAttribute("showatts", m_Options.classState.showAtts);
    viewElement.setAttribute("showopsig", m_Options.classState.showOpSig);
    viewElement.setAttribute("showops", m_Options.classState.showOps);
    viewElement.setAttribute("showpackage", m_Options.classState.showPackage);
    viewElement.setAttribute("showattribassocs", m_Options.classState.showAttribAssocs);
    viewElement.setAttribute("showscope", m_Options.classState.showVisibility);
    viewElement.setAttribute("showstereotype", m_Options.classState.showStereoType);
    //misc
    viewElement.setAttribute("localid", ID2STR(m_nLocalID));
    viewElement.setAttribute("showgrid", m_bShowSnapGrid);
    viewElement.setAttribute("snapgrid", m_bUseSnapToGrid);
    viewElement.setAttribute("snapcsgrid", m_bUseSnapComponentSizeToGrid);
    viewElement.setAttribute("snapx", m_nSnapX);
    viewElement.setAttribute("snapy", m_nSnapY);
    viewElement.setAttribute("canvasheight", canvasHeight());
    viewElement.setAttribute("canvaswidth", canvasWidth());
    //now save all the widgets


    QDomElement widgetElement = qDoc.createElement("widgets");
    foreach(NewUMLRectWidget *widget , m_WidgetList) {
        // Having an exception is bad I know, but gotta work with
        // system we are given.
        // We DON'T want to record any text widgets which are belonging
        // to associations as they are recorded later in the "associations"
        // section when each owning association is dumped. -b.t.
        if ((widget->getBaseType() != wt_Text && widget->getBaseType() != wt_FloatingDashLine) ||
            static_cast<FloatingTextWidget*>(widget)->getLink() == NULL)
            widget->saveToXMI(qDoc, widgetElement);
    }
    viewElement.appendChild(widgetElement);
    //now save the message widgets
    QDomElement messageElement = qDoc.createElement("messages");
    foreach(NewUMLRectWidget* widget , m_MessageList) {
        widget->saveToXMI(qDoc, messageElement);
    }
    viewElement.appendChild(messageElement);
    //now save the associations
    QDomElement assocElement = qDoc.createElement("associations");
    if (m_AssociationList.count()) {
        // We guard against (m_AssociationList.count() == 0) because
        // this code could be reached as follows:
        //  ^  UMLScene::saveToXMI()
        //  ^  UMLDoc::saveToXMI()
        //  ^  UMLDoc::addToUndoStack()
        //  ^  UMLDoc::setModified()
        //  ^  UMLDoc::createDiagram()
        //  ^  UMLDoc::newDocument()
        //  ^  UMLApp::newDocument()
        //  ^  main()
        //
        AssociationWidgetListIt a_it(m_AssociationList);
        AssociationWidget * assoc = 0;
        foreach(assoc , m_AssociationList) {
            assoc->saveToXMI(qDoc, assocElement);
        }
    }
    viewElement.appendChild(assocElement);
    qElement.appendChild(viewElement);
}

bool UMLScene::loadFromXMI(QDomElement & qElement)
{
    QString id = qElement.attribute("xmi.id", "-1");
    m_nID = STR2ID(id);
    if (m_nID == Uml::id_None)
        return false;
    setName(qElement.attribute("name", ""));
    QString type = qElement.attribute("type", "0");
    m_Documentation = qElement.attribute("documentation", "");
    QString localid = qElement.attribute("localid", "0");
    //optionstate uistate
    QString font = qElement.attribute("font", "");
    if (!font.isEmpty()) {
        m_Options.uiState.font.fromString(font);
        m_Options.uiState.font.setUnderline(false);
    }
    QString fillcolor = qElement.attribute("fillcolor", "");
    QString linecolor = qElement.attribute("linecolor", "");
    QString linewidth = qElement.attribute("linewidth", "");
    QString usefillcolor = qElement.attribute("usefillcolor", "0");
    m_Options.uiState.useFillColor = (bool)usefillcolor.toInt();
    //optionstate classstate
    QString temp = qElement.attribute("showattsig", "0");
    m_Options.classState.showAttSig = (bool)temp.toInt();
    temp = qElement.attribute("showatts", "0");
    m_Options.classState.showAtts = (bool)temp.toInt();
    temp = qElement.attribute("showopsig", "0");
    m_Options.classState.showOpSig = (bool)temp.toInt();
    temp = qElement.attribute("showops", "0");
    m_Options.classState.showOps = (bool)temp.toInt();
    temp = qElement.attribute("showpackage", "0");
    m_Options.classState.showPackage = (bool)temp.toInt();
    temp = qElement.attribute("showattribassocs", "0");
    m_Options.classState.showAttribAssocs = (bool)temp.toInt();
    temp = qElement.attribute("showscope", "0");
    m_Options.classState.showVisibility = (bool)temp.toInt();
    temp = qElement.attribute("showstereotype", "0");
    m_Options.classState.showStereoType = (bool)temp.toInt();
    //misc
    QString showgrid = qElement.attribute("showgrid", "0");
    m_bShowSnapGrid = (bool)showgrid.toInt();

    QString snapgrid = qElement.attribute("snapgrid", "0");
    m_bUseSnapToGrid = (bool)snapgrid.toInt();

    QString snapcsgrid = qElement.attribute("snapcsgrid", "0");
    m_bUseSnapComponentSizeToGrid = (bool)snapcsgrid.toInt();

    QString snapx = qElement.attribute("snapx", "10");
    m_nSnapX = snapx.toInt();

    QString snapy = qElement.attribute("snapy", "10");
    m_nSnapY = snapy.toInt();

    QString height = qElement.attribute("canvasheight", QString("%1").arg(UMLScene::defaultCanvasSize));
    qreal canvasHeight = height.toDouble();

    QString width = qElement.attribute("canvaswidth", QString("%1").arg(UMLScene::defaultCanvasSize));
    qreal canvasWidth = width.toDouble();
    setSceneRect(0, 0, canvasWidth, canvasHeight);

    int nType = type.toInt();
    if (nType == -1 || nType >= 400) {
        // Pre 1.5.5 numeric values
        // Values of "type" were changed in 1.5.5 to merge with Settings::Diagram
        switch (nType) {
        case 400:
            m_Type = Uml::dt_UseCase;
            break;
        case 401:
            m_Type = Uml::dt_Collaboration;
            break;
        case 402:
            m_Type = Uml::dt_Class;
            break;
        case 403:
            m_Type = Uml::dt_Sequence;
            break;
        case 404:
            m_Type = Uml::dt_State;
            break;
        case 405:
            m_Type = Uml::dt_Activity;
            break;
        case 406:
            m_Type = Uml::dt_Component;
            break;
        case 407:
            m_Type = Uml::dt_Deployment;
            break;
        case 408:
            m_Type = Uml::dt_EntityRelationship;
            break;
        default:
            m_Type = Uml::dt_Undefined;
            break;
        }
    } else {
        m_Type = (Uml::Diagram_Type)nType;
    }
    if (!fillcolor.isEmpty())
        m_Options.uiState.fillColor = QColor(fillcolor);
    if (!linecolor.isEmpty())
        m_Options.uiState.lineColor = QColor(linecolor);
    if (!linewidth.isEmpty())
        m_Options.uiState.lineWidth = linewidth.toInt();
    m_nLocalID = STR2ID(localid);

    QDomNode node = qElement.firstChild();
    bool widgetsLoaded = false, messagesLoaded = false, associationsLoaded = false;
    while (!node.isNull()) {
        QDomElement element = node.toElement();
        if (!element.isNull()) {
            if (element.tagName() == "widgets")
                widgetsLoaded = loadWidgetsFromXMI(element);
            else if (element.tagName() == "messages")
                messagesLoaded = loadMessagesFromXMI(element);
            else if (element.tagName() == "associations")
                associationsLoaded = loadAssociationsFromXMI(element);
        }
        node = node.nextSibling();
    }

    if (!widgetsLoaded) {
        uWarning() << "failed umlview load on widgets";
        return false;
    }
    if (!messagesLoaded) {
        uWarning() << "failed umlview load on messages";
        return false;
    }
    if (!associationsLoaded) {
        uWarning() << "failed umlview load on associations";
        return false;
    }
    return true;
}

bool UMLScene::loadWidgetsFromXMI(QDomElement & qElement)
{
    NewUMLRectWidget* widget = 0;
    QDomNode node = qElement.firstChild();
    QDomElement widgetElement = node.toElement();
    while (!widgetElement.isNull()) {
        widget = loadWidgetFromXMI(widgetElement);
        if (widget) {
            m_WidgetList.append(widget);
            // In the interest of best-effort loading, in case of a
            // (widget == NULL) we still go on.
            // The individual widget's loadFromXMI method should
            // already have generated an error message to tell the
            // user that something went wrong.
        }
        node = widgetElement.nextSibling();
        widgetElement = node.toElement();
    }

    return true;
}

NewUMLRectWidget* UMLScene::loadWidgetFromXMI(QDomElement& widgetElement)
{

    if (!m_pDoc) {
        uWarning() << "m_pDoc is NULL";
        return 0L;
    }

    QString tag  = widgetElement.tagName();
    QString idstr  = widgetElement.attribute("xmi.id", "-1");
    NewUMLRectWidget* widget = Widget_Factory::makeWidgetFromXMI(tag, idstr, this);

    if (widget == NULL)
        return NULL;
    if (!widget->loadFromXMI(widgetElement)) {
        widget->cleanup();
        delete widget;
        return 0;
    }
    return widget;
}

bool UMLScene::loadMessagesFromXMI(QDomElement & qElement)
{
    MessageWidget * message = 0;
    QDomNode node = qElement.firstChild();
    QDomElement messageElement = node.toElement();
    while (!messageElement.isNull()) {
        QString tag = messageElement.tagName();
        if (tag == "messagewidget" ||
            tag == "UML:MessageWidget") {   // for bkwd compatibility
            message = new MessageWidget(this, sequence_message_asynchronous,
                                        Uml::id_Reserved);
            if (!message->loadFromXMI(messageElement)) {
                delete message;
                return false;
            }
            m_MessageList.append(message);
            FloatingTextWidget *ft = message->getFloatingTextWidget();
            if (ft)
                m_WidgetList.append(ft);
            else if (message->getSequenceMessageType() != sequence_message_creation)
                uDebug() << "ft is NULL for message " << ID2STR(message->getID()) << endl;
        }
        node = messageElement.nextSibling();
        messageElement = node.toElement();
    }
    return true;
}

bool UMLScene::loadAssociationsFromXMI(QDomElement & qElement)
{
    QDomNode node = qElement.firstChild();
    QDomElement assocElement = node.toElement();
    int countr = 0;
    while (!assocElement.isNull()) {
        QString tag = assocElement.tagName();
        if (tag == "assocwidget" ||
            tag == "UML:AssocWidget") {  // for bkwd compatibility
            countr++;
            AssociationWidget *assoc = new AssociationWidget(this);
            if (!assoc->loadFromXMI(assocElement)) {
                uError() << "could not loadFromXMI association widget:"
                         << (void*)assoc << ", bad XMI file? Deleting from umlview.";
                delete assoc;
                /* return false;
                   Returning false here is a little harsh when the
                   rest of the diagram might load okay.
                */
            } else {
                if (!addAssociation(assoc, false)) {
                    uError() << "Could not addAssociation(" << (void*)assoc << ") to umlview, deleting.";
                    //               assoc->cleanup();
                    delete assoc;
                    //return false; // soften error.. may not be that bad
                }
            }
        }
        node = assocElement.nextSibling();
        assocElement = node.toElement();
    }
    return true;
}

void UMLScene::addObject(UMLObject *object)
{
    m_bCreateObject = true;
    if (m_pDoc->addUMLObject(object))
        m_pDoc->signalUMLObjectCreated(object);  // m_bCreateObject is reset by slotObjectCreated()
    else
        m_bCreateObject = false;
}

bool UMLScene::loadUisDiagramPresentation(QDomElement & qElement)
{
    for (QDomNode node = qElement.firstChild(); !node.isNull(); node = node.nextSibling()) {
        QDomElement elem = node.toElement();
        QString tag = elem.tagName();
        if (! Uml::tagEq(tag, "Presentation")) {
            uError() << "ignoring unknown UisDiagramPresentation tag " << tag;
            continue;
        }
        QDomNode n = elem.firstChild();
        QDomElement e = n.toElement();
        QString idStr;
        int x = 0, y = 0, w = 0, h = 0;
        while (!e.isNull()) {
            tag = e.tagName();
            uDebug() << "Presentation: tag = " << tag;
            if (Uml::tagEq(tag, "Presentation.geometry")) {
                QDomNode gnode = e.firstChild();
                QDomElement gelem = gnode.toElement();
                QString csv = gelem.text();
                QStringList dim = csv.split(",");
                x = dim[0].toInt();
                y = dim[1].toInt();
                w = dim[2].toInt();
                h = dim[3].toInt();
            } else if (Uml::tagEq(tag, "Presentation.style")) {
                // TBD
            } else if (Uml::tagEq(tag, "Presentation.model")) {
                QDomNode mnode = e.firstChild();
                QDomElement melem = mnode.toElement();
                idStr = melem.attribute("xmi.idref", "");
            } else {
                uDebug() << "ignoring tag " << tag;
            }
            n = n.nextSibling();
            e = n.toElement();
        }
        Uml::IDType id = STR2ID(idStr);
        UMLObject *o = m_pDoc->findObjectById(id);
        if (o == NULL) {
            uError() << "Cannot find object for id " << idStr << endl;
        } else {
            Uml::Object_Type ot = o->getBaseType();
            uDebug() << "Create widget for model object of type " << ot;
            NewUMLRectWidget *widget = NULL;
            switch (ot) {
            case Uml::ot_Class:
                widget = new ClassifierWidget(static_cast<UMLClassifier*>(o));
                break;
            case Uml::ot_Association: {
                UMLAssociation *umla = static_cast<UMLAssociation*>(o);
                Uml::Association_Type at = umla->getAssocType();
                UMLObject* objA = umla->getObject(Uml::A);
                UMLObject* objB = umla->getObject(Uml::B);
                if (objA == NULL || objB == NULL) {
                    uError() << "intern err 1" << endl;
                    return false;
                }
                NewUMLRectWidget *wA = findWidget(objA->getID());
                NewUMLRectWidget *wB = findWidget(objB->getID());
                if (wA != NULL && wB != NULL) {
                    AssociationWidget *aw =
                        new AssociationWidget(this, wA, at, wB, umla);
                    aw->syncToModel();
                    m_AssociationList.append(aw);
                } else {
                    uError() << "cannot create assocwidget from ("
                             << (int)wA << ", " << (int)wB << ")" << endl;
                }
                break;
            }
            case Uml::ot_Role: {
                //UMLRole *robj = static_cast<UMLRole*>(o);
                //UMLAssociation *umla = robj->getParentAssociation();
                // @todo properly display role names.
                //       For now, in order to get the role names displayed
                //       simply delete the participating diagram objects
                //       and drag them from the list view to the diagram.
                break;
            }
            default:
                uError() << "Cannot create widget of type " << ot << endl;
            }
            if (widget) {
                uDebug() << "Widget: x=" << x << ", y=" << y
                         << ", w=" << w << ", h=" << h << endl;
                widget->setX(x);
                widget->setY(y);
                widget->setSize(w, h);
                addItem(widget);
                m_WidgetList.append(widget);
            }
        }
    }
    return true;
}

bool UMLScene::loadUISDiagram(QDomElement & qElement)
{
    QString idStr = qElement.attribute("xmi.id", "");
    if (idStr.isEmpty())
        return false;
    m_nID = STR2ID(idStr);
    UMLListViewItem *ulvi = NULL;
    for (QDomNode node = qElement.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isComment())
            continue;
        QDomElement elem = node.toElement();
        QString tag = elem.tagName();
        if (tag == "uisDiagramName") {
            setName(elem.text());
            if (ulvi)
                ulvi->setText(getName());
        } else if (tag == "uisDiagramStyle") {
            QString diagramStyle = elem.text();
            if (diagramStyle != "ClassDiagram") {
                uError() << "diagram style " << diagramStyle << " is not yet implemented" << endl;
                continue;
            }
            m_pDoc->setMainViewID(m_nID);
            m_Type = Uml::dt_Class;
            UMLListView *lv = UMLApp::app()->getListView();
            ulvi = new UMLListViewItem(lv->theLogicalView(), getName(),
                                       Uml::lvt_Class_Diagram, m_nID);
        } else if (tag == "uisDiagramPresentation") {
            loadUisDiagramPresentation(elem);
        } else if (tag != "uisToolName") {
            uDebug() << "ignoring tag " << tag;
        }
    }
    return true;
}


void UMLScene::alignLeft()
{
    UMLWidgetList widgetList;
    getSelectedWidgets(widgetList);
    if (widgetList.isEmpty())
        return;

    qreal smallestX = getSmallestX(widgetList);

    foreach(NewUMLRectWidget *widget , widgetList) {
        widget->setX(smallestX);
        widget->adjustAssocs(widget->getX(), widget->getY());
    }
}

void UMLScene::alignRight()
{
    UMLWidgetList widgetList;
    getSelectedWidgets(widgetList);
    if (widgetList.isEmpty())
        return;
    qreal biggestX = getBiggestX(widgetList);

    foreach(NewUMLRectWidget *widget , widgetList) {
        widget->setX(biggestX - widget->getWidth());
        widget->adjustAssocs(widget->getX(), widget->getY());
    }
}

void UMLScene::alignTop()
{
    UMLWidgetList widgetList;
    getSelectedWidgets(widgetList);
    if (widgetList.isEmpty())
        return;

    qreal smallestY = getSmallestY(widgetList);

    foreach(NewUMLRectWidget *widget , widgetList) {
        widget->setY(smallestY);
        widget->adjustAssocs(widget->getX(), widget->getY());
    }
}

void UMLScene::alignBottom()
{
    UMLWidgetList widgetList;
    getSelectedWidgets(widgetList);
    if (widgetList.isEmpty())
        return;
    qreal biggestY = getBiggestY(widgetList);

    foreach(NewUMLRectWidget *widget , widgetList) {
        widget->setY(biggestY - widget->getHeight());
        widget->adjustAssocs(widget->getX(), widget->getY());
    }
}

void UMLScene::alignVerticalMiddle()
{
    UMLWidgetList widgetList;
    getSelectedWidgets(widgetList);
    if (widgetList.isEmpty())
        return;

    qreal smallestX = getSmallestX(widgetList);
    qreal biggestX = getBiggestX(widgetList);
    qreal middle = int((biggestX - smallestX) / 2) + smallestX;

    foreach(NewUMLRectWidget *widget , widgetList) {
        widget->setX(middle - int(widget->getWidth() / 2));
        widget->adjustAssocs(widget->getX(), widget->getY());
    }
}

void UMLScene::alignHorizontalMiddle()
{
    UMLWidgetList widgetList;
    getSelectedWidgets(widgetList);
    if (widgetList.isEmpty())
        return;

    qreal smallestY = getSmallestY(widgetList);
    qreal biggestY = getBiggestY(widgetList);
    qreal middle = int((biggestY - smallestY) / 2) + smallestY;

    foreach(NewUMLRectWidget *widget , widgetList) {
        widget->setY(middle - int(widget->getHeight() / 2));
        widget->adjustAssocs(widget->getX(), widget->getY());
    }
}

void UMLScene::alignVerticalDistribute()
{
    UMLWidgetList widgetList;
    getSelectedWidgets(widgetList);
    if (widgetList.isEmpty())
        return;

    qreal smallestY = getSmallestY(widgetList);
    qreal biggestY = getBiggestY(widgetList);
    qreal heightsSum = getHeightsSum(widgetList);
    qreal distance = int(((biggestY - smallestY) - heightsSum) / (widgetList.count() - 1.0) + 0.5);

    sortWidgetList(widgetList, hasWidgetSmallerY);

    int i = 1;
    NewUMLRectWidget* widgetPrev = NULL;
    foreach(NewUMLRectWidget *widget , widgetList) {
        if (i == 1) {
            widgetPrev = widget;
        } else {
            widget->setY(widgetPrev->getY() + widgetPrev->getHeight() + distance);
            widget->adjustAssocs(widget->getX(), widget->getY());
            widgetPrev = widget;
        }
        i++;
    }
}

void UMLScene::alignHorizontalDistribute()
{
    UMLWidgetList widgetList;
    getSelectedWidgets(widgetList);
    if (widgetList.isEmpty())
        return;

    qreal smallestX = getSmallestX(widgetList);
    qreal biggestX = getBiggestX(widgetList);
    qreal widthsSum = getWidthsSum(widgetList);
    qreal distance = int(((biggestX - smallestX) - widthsSum) / (widgetList.count() - 1.0) + 0.5);

    sortWidgetList(widgetList, hasWidgetSmallerX);

    int i = 1;
    NewUMLRectWidget* widgetPrev = NULL;
    foreach(NewUMLRectWidget *widget ,  widgetList) {
        if (i == 1) {
            widgetPrev = widget;
        } else {
            widget->setX(widgetPrev->getX() + widgetPrev->getWidth() + distance);
            widget->adjustAssocs(widget->getX(), widget->getY());
            widgetPrev = widget;
        }
        i++;
    }

}

bool UMLScene::hasWidgetSmallerX(const NewUMLRectWidget* widget1, const NewUMLRectWidget* widget2)
{
    return widget1->getX() < widget2->getX();
}

bool UMLScene::hasWidgetSmallerY(const NewUMLRectWidget* widget1, const NewUMLRectWidget* widget2)
{
    return widget1->getY() < widget2->getY();
}

qreal UMLScene::getSmallestX(const UMLWidgetList &widgetList)
{
    UMLWidgetListIt it(widgetList);

    qreal smallestX = 0;

    int i = 1;
    foreach(NewUMLRectWidget *widget ,  widgetList) {
        if (i == 1) {
            smallestX = widget->getX();
        } else {
            if (smallestX > widget->getX())
                smallestX = widget->getX();
        }
        i++;
    }

    return smallestX;
}

qreal UMLScene::getSmallestY(const UMLWidgetList &widgetList)
{

    if (widgetList.isEmpty())
        return -1;

    qreal smallestY = 0;

    int i = 1;
    foreach(NewUMLRectWidget *widget ,  widgetList) {
        if (i == 1) {
            smallestY = widget->getY();
        } else {
            if (smallestY > widget->getY())
                smallestY = widget->getY();
        }
        i++;
    }

    return smallestY;
}

qreal UMLScene::getBiggestX(const UMLWidgetList &widgetList)
{
    if (widgetList.isEmpty())
        return -1;

    qreal biggestX = 0;

    int i = 1;
    foreach(NewUMLRectWidget *widget , widgetList) {
        if (i == 1) {
            biggestX = widget->getX();
            biggestX += widget->getWidth();
        } else {
            if (biggestX < widget->getX() + widget->getWidth())
                biggestX = widget->getX() + widget->getWidth();
        }
        i++;
    }

    return biggestX;
}

qreal UMLScene::getBiggestY(const UMLWidgetList &widgetList)
{

    if (widgetList.isEmpty())
        return -1;

    qreal biggestY = 0;

    int i = 1;
    foreach(NewUMLRectWidget *widget , widgetList) {
        if (i == 1) {
            biggestY = widget->getY();
            biggestY += widget->getHeight();
        } else {
            if (biggestY < widget->getY() + widget->getHeight())
                biggestY = widget->getY() + widget->getHeight();
        }
        i++;
    }

    return biggestY;
}

qreal UMLScene::getHeightsSum(const UMLWidgetList &widgetList)
{
    qreal heightsSum = 0;


    foreach(NewUMLRectWidget *widget , widgetList) {
        heightsSum += widget->getHeight();
    }

    return heightsSum;
}

qreal UMLScene::getWidthsSum(const UMLWidgetList &widgetList)
{
    qreal widthsSum = 0;

    foreach(NewUMLRectWidget *widget , widgetList) {
        widthsSum += widget->getWidth();
    }

    return widthsSum;
}

template<typename Compare>
void UMLScene::sortWidgetList(UMLWidgetList &widgetList, Compare comp)
{
    QVector<NewUMLRectWidget*> widgetVector;

    for (UMLWidgetList::iterator it = widgetList.begin(); it != widgetList.end(); ++it) {
        widgetVector.push_back(*it);
    }
    qSort(widgetVector.begin(), widgetVector.end(), comp);

    widgetList.clear();

    for (QVector<NewUMLRectWidget*>::iterator it = widgetVector.begin(); it != widgetVector.end(); ++it) {
        widgetList.append(*it);
    }
}

#include "umlscene.moc"
