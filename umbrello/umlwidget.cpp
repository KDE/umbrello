/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header file
#include "umlwidget.h"
// system includes
#include <qpainter.h>
#include <qcolor.h>
#include <kdebug.h>
#include <kcolordialog.h>
#include <kfontdialog.h>
#include <kmessagebox.h>
// local includes
#include "umlwidgetcontroller.h"
#include "umlobject.h"
#include "classifier.h"
#include "uniqueid.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlclassifierlistitemlist.h"
#include "codegenerator.h"
#include "codegenerators/simplecodegenerator.h"
#include "listpopupmenu.h"
#include "associationwidget.h"
#include "dialogs/settingsdlg.h"
#include "codedocument.h"
#include "floatingtextwidget.h"
#include "docwindow.h"
#include "dialogs/classpropdlg.h"
#include "clipboard/idchangelog.h"

using namespace Uml;


UMLWidget::UMLWidget( UMLView * view, UMLObject * o, UMLWidgetController *widgetController /* = 0*/ )
        : WidgetBase(view), QCanvasRectangle( view->canvas() ),
        m_pMenu(0)
{
    if (widgetController) {
        m_widgetController = widgetController;
    } else {
        m_widgetController = new UMLWidgetController(this);
    }
    init();
    m_pObject = o;
    if(m_pObject) {
        connect( m_pObject, SIGNAL(modified()), this, SLOT(updateWidget()) );
        m_nId = m_pObject->getID();
    }
}

UMLWidget::UMLWidget(UMLView * view, Uml::IDType id /* = Uml::id_None */, UMLWidgetController *widgetController /* = 0*/)
        : WidgetBase(view), QCanvasRectangle( view->canvas() ),
        m_pMenu(0)
{
    if (widgetController) {
        m_widgetController = widgetController;
    } else {
        m_widgetController = new UMLWidgetController(this);
    }
    init();
    if (id == Uml::id_None)
        m_nId = UniqueID::gen();
    else
        m_nId = id;
}

UMLWidget::~UMLWidget() {
    //slotRemovePopupMenu();
    delete m_widgetController;
    cleanup();
}

UMLWidget& UMLWidget::operator=(const UMLWidget& other) {
    if (this == &other)
        return *this;

    // assign members loaded/saved
    m_bUseFillColour = other.m_bUseFillColour;
    m_nId = other.m_nId;
    m_Type = other.m_Type;
    setX( other.getX() );
    setY( other.getY() );
    m_Assocs = other.m_Assocs;
    m_Font = other.m_Font;
    QCanvasRectangle::setSize( other.width(), other.height() );
    m_bUsesDiagramFillColour = other.m_bUsesDiagramFillColour;
    m_bUsesDiagramLineColour = other.m_bUsesDiagramLineColour;
    m_bUsesDiagramLineWidth  = other.m_bUsesDiagramLineWidth;
    m_bUsesDiagramUseFillColour = other.m_bUsesDiagramUseFillColour;
    m_LineColour = other.m_LineColour;
    m_LineWidth  = other.m_LineWidth;
    m_FillColour = other.m_FillColour;
    m_bIsInstance = other.m_bIsInstance;
    m_instanceName = other.m_instanceName;

    // assign volatile (non-saved) members
    m_bSelected = other.m_bSelected;
    m_bStartMove = other.m_bStartMove;
    m_nPosX = other.m_nPosX;
    m_pObject = other.m_pObject;
    m_pView = other.m_pView;
    m_pMenu = other.m_pMenu;
    m_bResizable = other.m_bResizable;
    for (unsigned i = 0; i < FT_INVALID; i++)
        m_pFontMetrics[i] = other.m_pFontMetrics[i];
    m_bActivated = other.m_bActivated;
    m_bIgnoreSnapToGrid = other.m_bIgnoreSnapToGrid;
    m_bIgnoreSnapComponentSizeToGrid = other.m_bIgnoreSnapComponentSizeToGrid;
    return *this;
}

bool UMLWidget::operator==(const UMLWidget& other) {
    if( this == &other )
        return true;

    if(m_Type != other.m_Type) {
        return false;
    }

    if (getID() != other.getID())
        return false;

    /* Testing the associations is already an exaggeration, no?
       The type and ID should uniquely identify an UMLWidget.
     */
    if (m_Assocs.count() != other.m_Assocs.count()) {
        return false;
    }

    // if(getBaseType() != wt_Text) // DON'T do this for floatingtext widgets, an infinite loop will result
    // {
    AssociationWidgetListIt assoc_it( m_Assocs );
    AssociationWidgetListIt assoc_it2( other.m_Assocs );
    AssociationWidget * assoc = 0, *assoc2 = 0;
    while ( ((assoc=assoc_it.current()) != 0) &&  ((assoc2=assoc_it2.current()) != 0)) {
        ++assoc_it;
        ++assoc_it2;
        if(!(*assoc == *assoc2)) {
            return false;
        }
    }
    // }
    return true;
    // NOTE:  In the comparison tests we are going to do, we don't need these values.
    // They will actually stop things functioning correctly so if you change these, be aware of that.
    /*
    if(m_bUseFillColour != other.m_bUseFillColour)
        return false;
    if(m_nId != other.m_nId)
        return false;
    if( m_Font != other.m_Font )
        return false;
    if(m_nX  != other.m_nX)
        return false;
    if(m_nY != other.m_nY)
        return false;
     */
}

void UMLWidget::mouseMoveEvent(QMouseEvent* me) {
    m_widgetController->mouseMoveEvent(me);
}

void UMLWidget::mousePressEvent(QMouseEvent *me) {
    m_widgetController->mousePressEvent(me);
}

void UMLWidget::updateWidget()
{
    updateComponentSize();
    adjustAssocs( getX(), getY() ); //adjust assoc lines.
    if (m_Type == Uml::wt_Class) {
        m_pView->createAutoAttributeAssociations(this);
    }
    if(isVisible())
        update();
}

QSize UMLWidget::calculateSize() {
    return QSize(20, 20);
}

void UMLWidget::constrain(int& width, int& height) {
    const QSize minSize = calculateSize();
    if (width < minSize.width())
        width = minSize.width();
    if (height < minSize.height())
        height = minSize.height();
}

void UMLWidget::mouseReleaseEvent(QMouseEvent *me) {
    m_widgetController->mouseReleaseEvent(me);
}

void UMLWidget::init() {
    m_nId = Uml::id_None;
    m_bIsInstance = false;
    if (m_pView) {
        m_bUseFillColour = true;
        m_bUsesDiagramFillColour = true;
        m_bUsesDiagramUseFillColour = true;
        const Settings::OptionState& optionState = m_pView->getOptionState();
        m_FillColour = optionState.uiState.fillColor;
        m_Font       = optionState.uiState.font;
        m_bShowStereotype = optionState.classState.showStereoType;
    } else {
        kError() << "UMLWidget::init: SERIOUS PROBLEM - m_pView is NULL" << endl;
        m_bUseFillColour = false;
        m_bUsesDiagramFillColour = false;
        m_bUsesDiagramUseFillColour = false;
        m_bShowStereotype = false;
    }

    for (int i = 0; i < (int)FT_INVALID; ++i)
        m_pFontMetrics[(UMLWidget::FontType)i] = 0;

    m_bResizable = true;

    m_bSelected = false;
    m_bStartMove = false;
    m_bActivated = false;
    m_bIgnoreSnapToGrid = false;
    m_bIgnoreSnapComponentSizeToGrid = false;
    m_pMenu = 0;
    m_pDoc = UMLApp::app()->getDocument();
    m_nPosX = 0;
    connect( m_pView, SIGNAL( sigRemovePopupMenu() ), this, SLOT( slotRemovePopupMenu() ) );
    connect( m_pView, SIGNAL( sigClearAllSelected() ), this, SLOT( slotClearAllSelected() ) );

    connect( m_pView, SIGNAL(sigColorChanged(Uml::IDType)), this, SLOT(slotColorChanged(Uml::IDType)));
    connect( m_pView, SIGNAL(sigLineWidthChanged(Uml::IDType)), this, SLOT(slotLineWidthChanged(Uml::IDType)));


    // connect( m_pView, SIGNAL(sigColorChanged(int)), this, SLOT(slotColorChanged(int)));
    m_pObject = NULL;
    setZ(m_origZ = 2);  // default for most widgets
}

void UMLWidget::slotMenuSelection(int sel) {
    QFont font;
    QColor newColour;
    const Uml::Widget_Type wt = m_Type;
    UMLWidget* widget = 0; // use for select the first object properties (fill, line color)

    switch(sel) {
    case ListPopupMenu::mt_Rename:
        m_pDoc -> renameUMLObject(m_pObject);
        // adjustAssocs( getX(), getY() );//adjust assoc lines
        break;

    case ListPopupMenu::mt_Delete:
        //remove self from diagram
        m_pView -> removeWidget(this);
        break;

    //UMLWidgetController::doMouseDoubleClick relies on this implementation
    case ListPopupMenu::mt_Properties:
        if (wt == wt_Actor || wt == wt_UseCase ||
                wt == wt_Package || wt == wt_Interface || wt == wt_Datatype ||
                wt == wt_Component || wt == wt_Artifact ||
                wt == wt_Node || wt == wt_Enum || wt == wt_Entity ||
                (wt == wt_Class && m_pView -> getType() == dt_Class)) {
            showProperties();
        } else if (wt == wt_Object) {
            m_pObject->showProperties();
        } else {
            kWarning() << "making properties dialog for unknown widget type" << endl;
        }
        // adjustAssocs( getX(), getY() );//adjust assoc lines
        break;

    case ListPopupMenu::mt_Line_Color:
    case ListPopupMenu::mt_Line_Color_Selection:
        widget = m_pView->getFirstMultiSelectedWidget();
        if (widget) { newColour = widget->getLineColor(); }
        if( KColorDialog::getColor(newColour) ) {
            m_pView -> selectionSetLineColor( newColour );
            m_pDoc -> setModified(true);
        }
        break;

    case ListPopupMenu::mt_Fill_Color:
    case ListPopupMenu::mt_Fill_Color_Selection:
        widget = m_pView->getFirstMultiSelectedWidget();
        if (widget) { newColour = widget->getFillColour(); }
        if ( KColorDialog::getColor(newColour) ) {
            m_pView -> selectionSetFillColor( newColour );
            m_pDoc -> setModified(true);
        }
        break;

    case ListPopupMenu::mt_Use_Fill_Color:
        m_bUseFillColour = !m_bUseFillColour;
        m_bUsesDiagramUseFillColour = false;
        m_pView->selectionUseFillColor( m_bUseFillColour );
        break;
    case ListPopupMenu::mt_Show_Attributes_Selection:
    case ListPopupMenu::mt_Show_Operations_Selection:
    case ListPopupMenu::mt_Visibility_Selection:
    case ListPopupMenu::mt_DrawAsCircle_Selection:
    case ListPopupMenu::mt_Show_Operation_Signature_Selection:
    case ListPopupMenu::mt_Show_Attribute_Signature_Selection:
    case ListPopupMenu::mt_Show_Packages_Selection:
    case ListPopupMenu::mt_Show_Stereotypes_Selection:
    case ListPopupMenu::mt_Show_Public_Only_Selection:
        m_pView->selectionToggleShow(sel);
        m_pDoc->setModified(true);
        break;

    case ListPopupMenu::mt_ViewCode: {
            UMLClassifier *c = dynamic_cast<UMLClassifier*>(m_pObject);
            if(c)
            {
                UMLApp::app()->viewCodeDocument(c);
            }
            break;
        }

    case ListPopupMenu::mt_Delete_Selection:
        m_pView -> deleteSelection();
        break;

    case ListPopupMenu::mt_Change_Font:
        font = getFont();
        if( KFontDialog::getFont( font, false, m_pView ) )
        {
            setFont( font );
            m_pDoc->setModified(true);
        }
        break;

    case ListPopupMenu::mt_Change_Font_Selection:
        font = getFont();
        if( KFontDialog::getFont( font, false, m_pView ) )
        {
            m_pView -> selectionSetFont( font );
            m_pDoc->setModified(true);
        }
        break;

    case ListPopupMenu::mt_Cut:
        m_pView -> setStartedCut();
        UMLApp::app() -> slotEditCut();
        break;

    case ListPopupMenu::mt_Copy:
        UMLApp::app() -> slotEditCopy();
        break;

    case ListPopupMenu::mt_Paste:
        UMLApp::app() -> slotEditPaste();
        break;

    case ListPopupMenu::mt_Refactoring:
        //check if we are operating on a classifier, or some other kind of UMLObject
        if(dynamic_cast<UMLClassifier*>(m_pObject))
        {
            UMLApp::app()->refactor(static_cast<UMLClassifier*>(m_pObject));
        }
        break;

    case ListPopupMenu::mt_Clone:
        // In principle we clone all the uml objects.
        {
            UMLObject *pClone = m_pObject->clone();
            m_pView->addObject(pClone);
        }
        break;

    case ListPopupMenu::mt_Rename_MultiA:
    case ListPopupMenu::mt_Rename_MultiB:
    case ListPopupMenu::mt_Rename_Name:
    case ListPopupMenu::mt_Rename_RoleAName:
    case ListPopupMenu::mt_Rename_RoleBName:
        {
            FloatingTextWidget *ft = static_cast<FloatingTextWidget*>(this);
            ft->handleRename();
            break;
        }
    }
}

void UMLWidget::slotWidgetMoved(Uml::IDType /*id*/) {}

void UMLWidget::slotColorChanged(Uml::IDType viewID) {
    //only change if on the diagram concerned
    if(m_pView->getID() != viewID) {
        return;
    }
    if ( m_bUsesDiagramFillColour ) {
        m_FillColour = m_pView->getFillColor();
    }
    if ( m_bUsesDiagramLineColour ) {
        m_LineColour = m_pView->getLineColor();
    }
    if ( m_bUsesDiagramUseFillColour ) {
        m_bUseFillColour = m_pView->getUseFillColor();
    }
    update();
}

void UMLWidget::slotLineWidthChanged(Uml::IDType viewID) {
    //only change if on the diagram concerned
    if(m_pView->getID() != viewID) {
        return;
    }
    if ( m_bUsesDiagramLineWidth ) {
        m_LineWidth = m_pView->getLineWidth();
    }
    update();
}

void UMLWidget::mouseDoubleClickEvent( QMouseEvent * me ) {
    m_widgetController->mouseDoubleClickEvent(me);
}

void UMLWidget::setUseFillColour(bool fc) {
    m_bUseFillColour = fc;
    m_bUsesDiagramUseFillColour = false;
    update();
}

void UMLWidget::setLineColor(const QColor &colour) {
    WidgetBase::setLineColor(colour);
    update();
}

void UMLWidget::setLineWidth(uint width) {
    WidgetBase::setLineWidth(width);
    update();
}

void UMLWidget::setFillColour(const QColor &colour) {
    m_FillColour = colour;
    m_bUsesDiagramFillColour = false;
    update();
}

void UMLWidget::drawSelected(QPainter * p, int offsetX, int offsetY) {
    int w = width();
    int h = height();
    int s = 4;
    QBrush brush(Qt::blue);
    p -> fillRect(offsetX, offsetY, s,  s, brush);
    p -> fillRect(offsetX, offsetY + h - s, s, s, brush);
    p -> fillRect(offsetX + w - s, offsetY, s, s, brush);

    // Draw the resize anchor in the lower right corner.
    if (m_bResizable) {
        brush.setColor(Qt::red);
        const int right = offsetX + w;
        const int bottom = offsetY + h;
        p->drawLine(right - s, offsetY + h - 1, offsetX + w - 1, offsetY + h - s);
        p->drawLine(right - (s*2), bottom - 1, right - 1, bottom - (s*2) );
        p->drawLine(right - (s*3), bottom - 1, right - 1, bottom - (s*3) );
     } else {
         p->fillRect(offsetX + w - s, offsetY + h - s, s, s, brush);
     }
}

bool UMLWidget::activate(IDChangeLog* /*ChangeLog  = 0 */) {
    if (widgetHasUMLObject(m_Type) && m_pObject == NULL) {
        m_pObject = m_pDoc->findObjectById(m_nId);
        if (m_pObject == NULL) {
            kError() << "UMLWidget::activate: cannot find UMLObject with id="
                << ID2STR(m_nId) << endl;
            return false;
        }
    }
    setFont(m_Font);
    setSize(getWidth(), getHeight());
    m_bActivated = true;
    updateComponentSize();
    if (m_pView->getPaste()) {
        FloatingTextWidget * ft = 0;
        QPoint point = m_pView -> getPastePoint();
        int x = point.x() + getX();
        int y = point.y() + getY();
        x = x < 0?0:x;
        y = y < 0?0:y;
        if( m_pView -> getType() == dt_Sequence ) {
            switch( getBaseType() ) {
            case wt_Object:
            case wt_Message:
                setY( getY() );
                setX( x );
                break;

            case wt_Text:
                ft = static_cast<FloatingTextWidget *>( this );
                if (ft->getRole() == tr_Seq_Message) {
                    setX( x );
                    setY( getY() );
                } else {
                    setX( getX() );
                    setY( getY() );
                }
                break;

            default:
                setY( y );
                break;
            }//end switch base type
        }//end if sequence
        else {
            setX( x );
            setY( y );
        }
    }//end if pastepoint
    else {
        setX( getX() );
        setY( getY() );
    }
    if ( m_pView -> getPaste() )
        m_pView -> createAutoAssociations( this );
    updateComponentSize();
    return true;
}

/** Read property of bool m_bActivated. */
bool UMLWidget::isActivated() {
    return m_bActivated;
}

void UMLWidget::setActivated(bool Active /*=true*/) {
    m_bActivated = Active;
}

void UMLWidget::addAssoc(AssociationWidget* pAssoc) {
    if (pAssoc && !m_Assocs.contains(pAssoc)) {
        m_Assocs.append(pAssoc);
    }
}

void UMLWidget::removeAssoc(AssociationWidget* pAssoc) {
    if(pAssoc) {
        m_Assocs.remove(pAssoc);
    }
}

void UMLWidget::adjustAssocs(int x, int y)
{
    // 2004-04-30: Achim Spangler
    // don't adjust Assocs on file load, as
    // the original positions, which are stored in XMI
    // should be reproduced exactly
    // ( don't try to reposition assocs as long
    //   as file is only partly loaded -> reposition
    //   could be misguided )
    /// @todo avoid trigger of this event during load
    if ( m_pDoc->loading() ) {
        // don't recalculate the assocs during load of XMI
        // -> return immediately without action
        return;
    }
    AssociationWidgetListIt assoc_it(m_Assocs);
    AssociationWidget* assocwidget = 0;
    while ((assocwidget = assoc_it.current())) {
        ++assoc_it;
        assocwidget->saveIdealTextPositions();
    }
    assoc_it.toFirst();
    while ((assocwidget = assoc_it.current())) {
        ++assoc_it;
        assocwidget->widgetMoved(this, x, y);
    }
}

void UMLWidget::adjustUnselectedAssocs(int x, int y)
{
    AssociationWidgetListIt assoc_it(m_Assocs);
    AssociationWidget* assocwidget = 0;
    while ((assocwidget = assoc_it.current())) {
        ++assoc_it;
        if(!assocwidget->getSelected())
            assocwidget->saveIdealTextPositions();
    }
    assoc_it.toFirst();
    while ((assocwidget = assoc_it.current())) {
        ++assoc_it;
        if(!assocwidget->getSelected())
            assocwidget->widgetMoved(this, x, y);
    }
}

void UMLWidget::showProperties() {
    // will already be selected so make sure docWindow updates the doc
    // back it the widget
    DocWindow *docwindow = UMLApp::app()->getDocWindow();
    docwindow->updateDocumentation( false );
    ClassPropDlg *dlg = new ClassPropDlg((QWidget*)UMLApp::app(), this);

    if (dlg->exec()) {
        docwindow->showDocumentation( getUMLObject() , true );
        m_pDoc->setModified(true);
    }
    dlg->close(true); //wipe from memory
}

void UMLWidget::startPopupMenu( const QPoint &At) {
    slotRemovePopupMenu();

    //if in a multi- selection to a specific m_pMenu for that
    // NEW: ask UMLView to count ONLY the widgets and not their floatingtextwidgets
    int count = m_pView->getSelectCount(true);
    //a MessageWidget when selected will select its text widget and vice versa
    //so take that into account for popup menu.

    // determine multi state
    bool multi = (m_bSelected && count > 1);

    // if multiple selected items have the same type
    bool unique = false;

    // if multiple items are selected, we have to check if they all have the same
    // base type
    if (multi == true)
        unique = m_pView -> checkUniqueSelection();

    // create the right click context menu
    m_pMenu = new ListPopupMenu(m_pView, this, multi, unique);

    // disable the "view code" menu for simple code generators
    CodeGenerator * currentCG = UMLApp::app()->getGenerator();
    if(currentCG && dynamic_cast<SimpleCodeGenerator*>(currentCG))
        m_pMenu->setItemEnabled(ListPopupMenu::mt_ViewCode, false);

    m_pMenu->popup(At);

    connect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotMenuSelection(int)));
}

void UMLWidget::slotRemovePopupMenu() {
    if(m_pMenu) {
        disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotMenuSelection(int)));
        delete m_pMenu;
        m_pMenu = 0;
    }
}

int UMLWidget::onWidget(const QPoint & p) {
    const int w = width();
    const int h = height();
    const int left = getX();
    const int right = left + w;
    const int top = getY();
    const int bottom = top + h;
    if (p.x() < left || p.x() > right ||
        p.y() < top || p.y() > bottom)   // Qt coord.sys. origin in top left corner
        return 0;
    return (w + h) / 2;
}

void UMLWidget::moveBy(int dx, int dy) {
    int newX = getX() + dx;
    int newY = getY() + dy;
    setX(newX);
    setY(newY);
    adjustAssocs(newX, newY);
}

void UMLWidget::setPen(QPainter & p) {
    p.setPen( QPen(m_LineColour, m_LineWidth) );
}

void UMLWidget::drawShape(QPainter &p ) {
    draw( p, getX(), getY() );
}

void UMLWidget::setSelected(bool _select) {
    const Uml::Widget_Type wt = m_Type;
    if( _select ) {
        if( m_pView -> getSelectCount() == 0 ) {
            if ( widgetHasUMLObject(wt) ) {
                m_pView->showDocumentation(m_pObject, false);
            } else {
                m_pView->showDocumentation(this, false);
            }
        }//end if
        /* if (wt != wt_Text && wt != wt_Box) {
            setZ(9);//keep text on top and boxes behind so don't touch Z value
        } */
    } else {
        /* if (wt != wt_Text && wt != wt_Box) {
            setZ(m_origZ);
        } */
        if( m_bSelected )
            m_pView -> updateDocumentation( true );
    }
    m_bSelected = _select;

    const QPoint pos(getX(), getY());
    UMLWidget *bkgnd = m_pView->getWidgetAt(pos);
    if (bkgnd && bkgnd != this && _select) {
        kDebug() << "UMLWidget::setSelected: setting Z to "
            << bkgnd->getZ() + 1 << ", SelectState: " << _select << endl;
        setZ( bkgnd->getZ() + 1 );
    } else {
        setZ( m_origZ );
    }

    update();

    /* selection changed, we have to make sure the copy and paste items
     * are correctly enabled/disabled */
    UMLApp::app()->slotCopyChanged();
}

void UMLWidget::slotClearAllSelected()
{
    setSelected( false );
}

void UMLWidget::setView(UMLView * v) {
    //remove signals from old view - was probably 0 anyway
    disconnect( m_pView, SIGNAL( sigRemovePopupMenu() ), this, SLOT( slotRemovePopupMenu() ) );
    disconnect( m_pView, SIGNAL( sigClearAllSelected() ), this, SLOT( slotClearAllSelected() ) );
    disconnect( m_pView, SIGNAL(sigColorChanged(Uml::IDType)), this, SLOT(slotColorChanged(Uml::IDType)));
    disconnect( m_pView, SIGNAL(sigLineWidthChanged(Uml::IDType)), this, SLOT(slotLineWidthChanged(Uml::IDType)));
    m_pView = v;
    connect( m_pView, SIGNAL( sigRemovePopupMenu() ), this, SLOT( slotRemovePopupMenu() ) );
    connect( m_pView, SIGNAL( sigClearAllSelected() ), this, SLOT( slotClearAllSelected() ) );
    connect( m_pView, SIGNAL(sigColorChanged(Uml::IDType)), this, SLOT(slotColorChanged(Uml::IDType)));
    connect( m_pView, SIGNAL(sigLineWidthChanged(Uml::IDType)), this, SLOT(slotLineWidthChanged(Uml::IDType)));
}

void UMLWidget::setX( int x ) {
    if (!m_bIgnoreSnapToGrid) {
        x = m_pView->snappedX(x);
    }
    QCanvasItem::setX( (double)x );
}

void UMLWidget::setY( int y ) {
    if (!m_bIgnoreSnapToGrid){
        y = m_pView->snappedX(y);
    }
    QCanvasItem::setY( (double)y );
}

void UMLWidget::setZ(int z) {
    m_origZ = getZ();
    QCanvasItem::setZ(z);
}

void UMLWidget::setName(const QString &strName) {
    if (m_pObject)
        m_pObject->setName(strName);
    else
        m_Text = strName;
    updateComponentSize();
    adjustAssocs( getX(), getY() );
}

QString UMLWidget::getName() const {
    if (m_pObject)
        return m_pObject->getName();
    return m_Text;
}

void UMLWidget::cleanup() {
}

void UMLWidget::slotSnapToGrid( ) {
    setX( getX() );
    setY( getY() );
}

bool UMLWidget::widgetHasUMLObject(Uml::Widget_Type type) {
    if (type == wt_Actor ||
            type == wt_UseCase ||
            type == wt_Class ||
            type == wt_Interface ||
            type == wt_Enum ||
            type == wt_Datatype ||
            type == wt_Package ||
            type == wt_Component ||
            type == wt_Node ||
            type == wt_Artifact ||
            type == wt_Object) {
        return true;
    } else {
        return false;
    }
}

void UMLWidget::setIgnoreSnapToGrid(bool to) {
    m_bIgnoreSnapToGrid = to;
}

bool UMLWidget::getIgnoreSnapToGrid() const {
    return m_bIgnoreSnapToGrid;
}

void UMLWidget::setSize(int width,int height) {
    // snap to the next larger size that is a multiple of the grid
    if (!m_bIgnoreSnapComponentSizeToGrid
            && m_pView -> getSnapComponentSizeToGrid() )
    {
        // integer divisions
        int numX = width / m_pView->getSnapX();
        int numY = height / m_pView->getSnapY();
        // snap to the next larger valid value
        if (width > numX * m_pView->getSnapX())
            width = (numX + 1) * m_pView->getSnapX();
        if (height > numY * m_pView->getSnapY())
            height = (numY + 1) * m_pView->getSnapY();
    }

    QCanvasRectangle::setSize(width,height);
}

void UMLWidget::updateComponentSize() {
    if (m_pDoc->loading())
        return;
    const QSize minSize = calculateSize();
    const int w = minSize.width();
    const int h = minSize.height();
    setSize(w, h);
    adjustAssocs( getX(), getY() );  // adjust assoc lines
}

void UMLWidget::setDefaultFontMetrics(UMLWidget::FontType fontType) {
    setupFontType(m_Font, fontType);
    setFontMetrics(fontType, QFontMetrics(m_Font));
}

void UMLWidget::setupFontType(QFont &font, UMLWidget::FontType fontType) {
    switch(fontType){
    case FT_NORMAL:
        font.setBold(false);
        font.setItalic(false);
        font.setUnderline(false);
        break;
    case FT_BOLD:
        font.setBold(true);
        font.setItalic(false);
        font.setUnderline(false);
        break;
    case FT_ITALIC:
        font.setBold(false);
        font.setItalic(true);
        font.setUnderline(false);
        break;
    case FT_UNDERLINE:
        font.setBold(false);
        font.setItalic(false);
        font.setUnderline(true);
        break;
    case FT_BOLD_ITALIC:
        font.setBold(true);
        font.setItalic(true);
        font.setUnderline(false);
        break;
    case FT_BOLD_UNDERLINE:
        font.setBold(true);
        font.setItalic(false);
        font.setUnderline(true);
        break;
    case FT_ITALIC_UNDERLINE:
        font.setBold(false);
        font.setItalic(true);
        font.setUnderline(true);
        break;
    case FT_BOLD_ITALIC_UNDERLINE:
        font.setBold(true);
        font.setItalic(true);
        font.setUnderline(true);
        break;
    default: return;
    }
}

void UMLWidget::setDefaultFontMetrics(UMLWidget::FontType fontType, QPainter &painter) {
    setupFontType(m_Font, fontType);
    painter.setFont(m_Font);
    setFontMetrics(fontType, painter.fontMetrics());
}

//FIXME this is probably the source of problems with widgets not being wide enough
QFontMetrics &UMLWidget::getFontMetrics(UMLWidget::FontType fontType) {
    if (m_pFontMetrics[fontType] == 0) {
        setDefaultFontMetrics(fontType);
    }
    return *m_pFontMetrics[fontType];
}

void UMLWidget::setFontMetrics(UMLWidget::FontType fontType, QFontMetrics fm) {
    delete m_pFontMetrics[fontType];
    m_pFontMetrics[fontType] = new QFontMetrics(fm);
}

QFont UMLWidget::getFont() const {
    return m_Font;
}

void UMLWidget::setFont( QFont font ) {
    m_Font = font;
    forceUpdateFontMetrics(0);
    if (m_pDoc->loading())
        return;
    update();
}

void UMLWidget::forceUpdateFontMetrics(QPainter *painter) {
    if (painter == 0) {
        for (int i = 0; i < (int)UMLWidget::FT_INVALID; ++i) {
            if (m_pFontMetrics[(UMLWidget::FontType)i]!=0)
                setDefaultFontMetrics((UMLWidget::FontType)i);
        }
    } else {
        for (int i2 = 0; i2 < (int)UMLWidget::FT_INVALID; ++i2) {
            if (m_pFontMetrics[(UMLWidget::FontType)i2]!=0)
                setDefaultFontMetrics((UMLWidget::FontType)i2,*painter);
        }
    }
    // calculate the size, based on the new font metric
    updateComponentSize();
}

void UMLWidget::setShowStereotype(bool _status) {
    m_bShowStereotype = _status;
    updateComponentSize();
    update();
}

bool UMLWidget::getShowStereotype() const {
    return m_bShowStereotype;
}

void UMLWidget::moveEvent(QMoveEvent* /*me*/) {
}

void UMLWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    /*
      Call after required actions in child class.
      Type must be set in the child class.
    */
    WidgetBase::saveToXMI(qDoc, qElement);
    qElement.setAttribute( "xmi.id", ID2STR(getID()) );
    qElement.setAttribute( "font", m_Font.toString() );
    qElement.setAttribute( "usefillcolor", m_bUseFillColour );
    qElement.setAttribute( "x", getX() );
    qElement.setAttribute( "y", getY() );
    qElement.setAttribute( "width", getWidth() );
    qElement.setAttribute( "height", getHeight() );
    // for consistency the following attributes now use american spelling for "color"
    qElement.setAttribute( "usesdiagramfillcolor", m_bUsesDiagramFillColour );
    qElement.setAttribute( "usesdiagramusefillcolor", m_bUsesDiagramUseFillColour );
    if (m_bUsesDiagramFillColour) {
        qElement.setAttribute( "fillcolor", "none" );
    } else {
        qElement.setAttribute( "fillcolor", m_FillColour.name() );
    }
    qElement.setAttribute("isinstance", m_bIsInstance);
    if (!m_instanceName.isEmpty())
        qElement.setAttribute("instancename", m_instanceName);
    if (m_bShowStereotype)
        qElement.setAttribute("showstereotype", m_bShowStereotype);
}

bool UMLWidget::loadFromXMI( QDomElement & qElement ) {
    WidgetBase::loadFromXMI(qElement);
    QString id = qElement.attribute( "xmi.id", "-1" );
    QString font = qElement.attribute( "font", "" );
    QString usefillcolor = qElement.attribute( "usefillcolor", "1" );
    QString x = qElement.attribute( "x", "0" );
    QString y = qElement.attribute( "y", "0" );
    QString h = qElement.attribute( "height", "0" );
    QString w = qElement.attribute( "width", "0" );
    /*
      For the next three *color attributes, there was a mixup of american and english spelling for "color".
      So first we need to keep backward compatibility and try to retrieve the *colour attribute.
      Next we overwrite this value if we find a *color, otherwise the former *colour is kept.
    */
    QString fillColour = qElement.attribute( "fillcolour", "none" );
    fillColour = qElement.attribute( "fillcolor", fillColour );
    QString usesDiagramFillColour = qElement.attribute( "usesdiagramfillcolour", "1" );
    usesDiagramFillColour = qElement.attribute( "usesdiagramfillcolor", usesDiagramFillColour );
    QString usesDiagramUseFillColour = qElement.attribute( "usesdiagramusefillcolour", "1" );
    usesDiagramUseFillColour = qElement.attribute( "usesdiagramusefillcolor", usesDiagramUseFillColour );

    m_nId = STR2ID(id);

    if( !font.isEmpty() ) {
        //QFont newFont;
        m_Font.fromString(font);
        //setFont(newFont);
    } else {
        kWarning() << "Using default font " << m_Font.toString()
        << " for widget with xmi.id " << ID2STR(m_nId) << endl;
        //setFont( m_Font );
    }
    m_bUseFillColour = (bool)usefillcolor.toInt();
    m_bUsesDiagramFillColour = (bool)usesDiagramFillColour.toInt();
    m_bUsesDiagramUseFillColour = (bool)usesDiagramUseFillColour.toInt();
    setSize( w.toInt(), h.toInt() );
    setX( x.toInt() );
    setY( y.toInt() );
    if (fillColour != "none") {
        m_FillColour = QColor(fillColour);
    }
    QString isinstance = qElement.attribute("isinstance", "0");
    m_bIsInstance = (bool)isinstance.toInt();
    m_instanceName = qElement.attribute("instancename", "");
    QString showstereo = qElement.attribute("showstereotype", "0");
    m_bShowStereotype = (bool)showstereo.toInt();
    return true;
}

UMLWidgetController* UMLWidget::getWidgetController() {
    return m_widgetController;
}

#include "umlwidget.moc"
