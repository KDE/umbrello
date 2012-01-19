/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header file
#include "umlwidget.h"
// qt includes
#include <QtCore/QPointer>
#include <QtGui/QPainter>
#include <QtGui/QColor>
#include <QtGui/QMouseEvent>
// kde includes
#include <kcolordialog.h>
#include <kfontdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
// local includes
#include "debug_utils.h"
#include "umlwidgetcontroller.h"
#include "umlobject.h"
#include "classifier.h"
#include "uniqueid.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlclassifierlistitemlist.h"
#include "listpopupmenu.h"
#include "associationwidget.h"
#include "settingsdlg.h"
#include "codedocument.h"
#include "floatingtextwidget.h"
#include "docwindow.h"
#include "classpropdlg.h"
#include "idchangelog.h"
#include "cmds.h"

using namespace Uml;

const UMLSceneSize UMLWidget::DefaultMinimumSize(50, 20);
const UMLSceneSize UMLWidget::DefaultMaximumSize(1000, 1000);

/**
 * Creates a UMLWidget object.
 *
 * @param scene The view to be displayed on.
 * @param o The UMLObject to represent.
 * @param widgetController The UMLWidgetController of this UMLWidget
 */
UMLWidget::UMLWidget(UMLScene * scene, WidgetType type, UMLObject * o, UMLWidgetController *widgetController)
  : WidgetBase(scene, type),
    UMLSceneRectangle(scene->canvas())
{
    if (widgetController) {
        m_widgetController = widgetController;
    } else {
        m_widgetController = new UMLWidgetController(this);
    }
    init();
    m_pObject = o;
    if (m_pObject) {
        connect(m_pObject, SIGNAL(modified()), this, SLOT(updateWidget()));
        m_nId = m_pObject->id();
    }
}

/**
 * Creates a UMLWidget object.
 *
 * @param scene The view to be displayed on.
 * @param id The id of the widget.
 *  The default value (id_None) will prompt generation of a new ID.
 * @param widgetController The UMLWidgetController of this UMLWidget
 */
UMLWidget::UMLWidget(UMLScene *scene, WidgetType type, Uml::IDType id, UMLWidgetController *widgetController)
  : WidgetBase(scene, type),
    UMLSceneRectangle(scene->canvas())
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

/**
 * Standard destructor.
 */
UMLWidget::~UMLWidget()
{
    //slotRemovePopupMenu();
    delete m_widgetController;
    cleanup();
}

/**
 * Assignment operator
 */
UMLWidget& UMLWidget::operator=(const UMLWidget & other)
{
    if (this == &other)
        return *this;

    WidgetBase::operator=(other);

    // assign members loaded/saved
    m_useFillColor = other.m_useFillColor;
    m_usesDiagramFillColor = other.m_usesDiagramFillColor;
    m_usesDiagramUseFillColor = other.m_usesDiagramUseFillColor;
    m_FillColor = other.m_FillColor;
    m_Assocs = other.m_Assocs;
    m_Text = other.m_Text; //new
    m_Font = other.m_Font;
    m_isInstance = other.m_isInstance;
    m_instanceName = other.m_instanceName;
    m_instanceName = other.m_instanceName;
    m_showStereotype = other.m_showStereotype;
    setX(other.getX());
    setY(other.getY());
    UMLSceneRectangle::setSize(other.width(), other.height());

    // assign volatile (non-saved) members
    m_selected = other.m_selected;
    m_startMove = other.m_startMove;
    m_nPosX = other.m_nPosX;
    m_origZ = other.m_origZ;  //new
    m_pMenu = other.m_pMenu;
    m_menuIsEmbedded = other.m_menuIsEmbedded;
    m_doc = other.m_doc;    //new
    m_resizable = other.m_resizable;
    for (unsigned i = 0; i < FT_INVALID; ++i)
        m_pFontMetrics[i] = other.m_pFontMetrics[i];
    m_activated = other.m_activated;
    m_ignoreSnapToGrid = other.m_ignoreSnapToGrid;
    m_ignoreSnapComponentSizeToGrid = other.m_ignoreSnapComponentSizeToGrid;
    // m_widgetController do not have any settings
    return *this;
}

/**
 * Overload '==' operator
 */
bool UMLWidget::operator==(const UMLWidget& other) const
{
    if (this == &other)
        return true;

    if (m_Type != other.m_Type) {
        return false;
    }

    if (id() != other.id())
        return false;

    /* Testing the associations is already an exaggeration, no?
       The type and ID should uniquely identify an UMLWidget.
     */
    if (m_Assocs.count() != other.m_Assocs.count()) {
        return false;
    }

    // if(getBaseType() != wt_Text) // DON'T do this for floatingtext widgets, an infinite loop will result
    // {
    AssociationWidgetListIt assoc_it(m_Assocs);
    AssociationWidgetListIt assoc_it2(other.m_Assocs);
    AssociationWidget * assoc = 0, *assoc2 = 0;
    while (assoc_it.hasNext() &&  assoc_it2.hasNext()) {
        assoc = assoc_it.next();
        assoc2 = assoc_it2.next();

        if (!(*assoc == *assoc2)) {
            return false;
        }
    }
    // }
    return true;
    // NOTE:  In the comparison tests we are going to do, we don't need these values.
    // They will actually stop things functioning correctly so if you change these, be aware of that.
    /*
    if(m_useFillColor != other.m_useFillColor)
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

/**
 * Calls the method with the same name in UMLWidgetController.
 * @see UMLWidgetController#mouseMoveEvent
 *
 * @param me The QMouseEvent event.
 */
void UMLWidget::mouseMoveEvent(QMouseEvent* me)
{
    m_widgetController->mouseMoveEvent(me);
}

/**
 * Calls the method with the same name in UMLWidgetController.
 * @see UMLWidgetController#mousePressEvent
 *
 * @param me The QMouseEvent event.
 */
void UMLWidget::mousePressEvent(QMouseEvent *me)
{
    m_widgetController->mousePressEvent(me);
}

/**
 * When a widget changes this slot captures that signal.
 */
void UMLWidget::updateWidget()
{
    updateComponentSize();
    adjustAssocs(getX(), getY());   //adjust assoc lines.
    switch (m_Type) {
    case WidgetBase::wt_Class:
        m_scene->createAutoAttributeAssociations(this);
        break;
    case WidgetBase::wt_Entity:
        m_scene->createAutoConstraintAssociations(this);
        break;
    default:
        break;
    }

    if (isVisible())
        update();
}

/**
 * Apply possible constraints to the given candidate width and height.
 * The default implementation limits input values to the bounds returned
 * by minimumSize()/maximumSize().
 *
 * @param width  input value, may be modified by the constraint
 * @param height input value, may be modified by the constraint
 */
void UMLWidget::constrain(int& width, int& height)
{
    UMLSceneSize minSize = minimumSize();
    if (width < minSize.width())
        width = minSize.width();
    if (height < minSize.height())
        height = minSize.height();
    UMLSceneSize maxSize = maximumSize();
    if (width > maxSize.width())
        width = maxSize.width();
    if (height > maxSize.height())
        height = maxSize.height();
}

/**
 * Calls the method with the same name in UMLWidgetController.
 * @see UMLWidgetController#mouseReleaseEvent
 *
 * @param me The QMouseEvent event.
 */
void UMLWidget::mouseReleaseEvent(QMouseEvent *me)
{
    m_widgetController->mouseReleaseEvent(me);
}

/**
 * Initializes key attributes of the class.
 */
void UMLWidget::init()
{
    m_nId = Uml::id_None;
    m_pMenu = 0;
    m_menuIsEmbedded = false;
    m_isInstance = false;
    setMinimumSize(DefaultMinimumSize);
    setMaximumSize(DefaultMaximumSize);
    
    for (int i = 0; i < (int)FT_INVALID; ++i)
        m_pFontMetrics[(UMLWidget::FontType)i] = 0;
    
    if (m_scene) {
        m_useFillColor = true;
        m_usesDiagramFillColor = true;
        m_usesDiagramUseFillColor = true;
        const Settings::OptionState& optionState = m_scene->optionState();
        m_FillColor = optionState.uiState.fillColor;
        // FIXME: using setFont() here let umbrello hang probably because of doc->loading() not set. 
        m_Font = optionState.uiState.font;
        m_showStereotype = optionState.classState.showStereoType;
    } else {
        uError() << "SERIOUS PROBLEM - m_scene is NULL";
        m_useFillColor = false;
        m_usesDiagramFillColor = false;
        m_usesDiagramUseFillColor = false;
        m_showStereotype = false;
    }

    m_resizable = true;

    m_selected = false;
    m_startMove = false;
    m_activated = false;
    m_ignoreSnapToGrid = false;
    m_ignoreSnapComponentSizeToGrid = false;
    m_pMenu = 0;
    m_doc = UMLApp::app()->document();
    m_nPosX = 0;
    connect(m_scene, SIGNAL(sigRemovePopupMenu()), this, SLOT(slotRemovePopupMenu()));
    connect(m_scene, SIGNAL(sigClearAllSelected()), this, SLOT(slotClearAllSelected()));

    connect(m_scene, SIGNAL(sigColorChanged(Uml::IDType)), this, SLOT(slotColorChanged(Uml::IDType)));
    connect(m_scene, SIGNAL(sigLineWidthChanged(Uml::IDType)), this, SLOT(slotLineWidthChanged(Uml::IDType)));

    m_pObject = NULL;
    setZ(m_origZ = 2);  // default for most widgets
}

/**
 * Captures any popup menu signals for menus it created.
 *
 * @param action The action which has to be executed.
 */
void UMLWidget::slotMenuSelection(QAction* action)
{
    QColor newColor;
    const WidgetBase::WidgetType wt = m_Type;
    UMLWidget* widget = 0; // use for select the first object properties (fill, line color)

    ListPopupMenu::MenuType sel = m_pMenu->getMenuType(action);
    switch (sel) {
    case ListPopupMenu::mt_Rename:
        m_doc->renameUMLObject(m_pObject);
        // adjustAssocs( getX(), getY() );//adjust assoc lines
        break;

    case ListPopupMenu::mt_Delete:
        //remove self from diagram
        m_scene->removeWidget(this);
        break;

        //UMLWidgetController::doMouseDoubleClick relies on this implementation
    case ListPopupMenu::mt_Properties:
        if (wt == WidgetBase::wt_Actor || wt == WidgetBase::wt_UseCase ||
                wt == WidgetBase::wt_Package || wt == WidgetBase::wt_Interface || wt == WidgetBase::wt_Datatype ||
                wt == WidgetBase::wt_Component || wt == WidgetBase::wt_Artifact ||
                wt == WidgetBase::wt_Node || wt == WidgetBase::wt_Enum || wt == WidgetBase::wt_Entity ||
                (wt == WidgetBase::wt_Class && m_scene->type() == Uml::DiagramType::Class)) {
            UMLApp::app()->beginMacro(i18n("Change Properties"));
            showPropertiesDialog();
            UMLApp::app()->endMacro();
        } else if (wt == wt_Object) {
            UMLApp::app()->beginMacro(i18n("Change Properties"));
            m_pObject->showPropertiesPagedDialog();
            UMLApp::app()->endMacro();
        } else {
            uWarning() << "making properties dialog for unknown widget type";
        }
        // adjustAssocs( getX(), getY() );//adjust assoc lines
        break;

    case ListPopupMenu::mt_Line_Color:
        widget = m_scene->getFirstMultiSelectedWidget();
        if (widget) {
            newColor = widget->lineColor();
        }
        if (KColorDialog::getColor(newColor)) {
            m_scene->selectionSetLineColor(newColor);
            m_doc->setModified(true);

        }
        break;

    case ListPopupMenu::mt_Fill_Color:
        widget = m_scene->getFirstMultiSelectedWidget();
        if (widget) {
            newColor = widget->fillColor();
        }
        if (KColorDialog::getColor(newColor)) {
            m_scene->selectionSetFillColor(newColor);
            m_doc->setModified(true);
        }
        break;

    case ListPopupMenu::mt_Use_Fill_Color:
        m_useFillColor = !m_useFillColor;
        m_usesDiagramUseFillColor = false;
        m_scene->selectionUseFillColor(m_useFillColor);
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
        m_scene->selectionToggleShow(sel);
        m_doc->setModified(true);
        break;

    case ListPopupMenu::mt_ViewCode: {
        UMLClassifier *c = dynamic_cast<UMLClassifier*>(m_pObject);
        if (c) {
            UMLApp::app()->viewCodeDocument(c);
        }
        break;
    }

    case ListPopupMenu::mt_Delete_Selection:
        m_scene->deleteSelection();
        break;

    case ListPopupMenu::mt_Change_Font:
    case ListPopupMenu::mt_Change_Font_Selection: {
        QFont font = UMLWidget::font();
        if (KFontDialog::getFont(font, KFontChooser::NoDisplayFlags, m_scene)) {
            UMLApp::app()->executeCommand(new CmdChangeFontSelection(m_doc, m_scene, font));
        }
    }
    break;

    case ListPopupMenu::mt_Cut:
        m_scene->setStartedCut();
        UMLApp::app()->slotEditCut();
        break;

    case ListPopupMenu::mt_Copy:
        UMLApp::app()->slotEditCopy();
        break;

    case ListPopupMenu::mt_Paste:
        UMLApp::app()->slotEditPaste();
        break;

    case ListPopupMenu::mt_Refactoring:
        //check if we are operating on a classifier, or some other kind of UMLObject
        if (dynamic_cast<UMLClassifier*>(m_pObject)) {
            UMLApp::app()->refactor(static_cast<UMLClassifier*>(m_pObject));
        }
        break;

    case ListPopupMenu::mt_Clone:
        // In principle we clone all the uml objects.
    {
        UMLObject *pClone = m_pObject->clone();
        m_scene->addObject(pClone);
    }
    break;

    case ListPopupMenu::mt_Rename_MultiA:
    case ListPopupMenu::mt_Rename_MultiB:
    case ListPopupMenu::mt_Rename_Name:
    case ListPopupMenu::mt_Rename_RoleAName:
    case ListPopupMenu::mt_Rename_RoleBName: {
        FloatingTextWidget *ft = static_cast<FloatingTextWidget*>(this);
        ft->handleRename();
        break;
    }

    default:
        uDebug() << "MenuType " << ListPopupMenu::toString(sel) << " not implemented";
    }
}

/**
 * Captures when another widget moves if it is link to it that signal.
 *
 * @param id The id of object behind the widget.
 */
void UMLWidget::slotWidgetMoved(Uml::IDType /*id*/)
{
}

/**
 * Captures a color change signal.
 *
 * @param sceneID The id of the object behind the widget.
 */
void UMLWidget::slotColorChanged(Uml::IDType viewID)
{
    //only change if on the diagram concerned
    if (m_scene->getID() != viewID) {
        return;
    }
    if (m_usesDiagramFillColor) {
        m_FillColor = m_scene->fillColor();
    }
    if (m_usesDiagramLineColor) {
        m_LineColor = m_scene->lineColor();
    }
    if (m_usesDiagramUseFillColor) {
        m_useFillColor = m_scene->useFillColor();
    }
    update();
}

/**
 * Captures a linewidth change signal.
 *
 * @param sceneID The id of the object behind the widget.
 */
void UMLWidget::slotLineWidthChanged(Uml::IDType viewID)
{
    //only change if on the diagram concerned
    if (m_scene->getID() != viewID) {
        return;
    }
    if (m_usesDiagramLineWidth) {
        m_LineWidth = m_scene->lineWidth();
    }
    update();
}

/**
 * Calls the method with the same name in UMLWidgetController.
 * @see UMLWidgetController#mouseDoubleClickEvent
 *
 * @param me The QMouseEvent event.
 */
void UMLWidget::mouseDoubleClickEvent(QMouseEvent * me)
{
    m_widgetController->mouseDoubleClickEvent(me);
}

/**
 * Set the status of using fill color.
 *
 * @param fc the status of using fill color.
 */
void UMLWidget::setUseFillColor(bool fc)
{
    WidgetBase::setUseFillColor(fc);
    update();
}

/**
 * Overrides the method from WidgetBase.
 */
void UMLWidget::setTextColorcmd(const QColor &color)
{
    WidgetBase::setTextColor(color);
    update();
}

/**
 * Overrides the method from WidgetBase.
 */
void UMLWidget::setTextColor(const QColor &color)
{
    UMLApp::app()->executeCommand(new CmdChangeTextColor(this, color));
    update();
}

/**
 * Overrides the method from WidgetBase.
 */
void UMLWidget::setLineColorcmd(const QColor &color)
{
    WidgetBase::setLineColor(color);
    update();
}

/**
 * Overrides the method from WidgetBase.
 */
void UMLWidget::setLineColor(const QColor &color)
{
    UMLApp::app()->executeCommand(new CmdChangeLineColor(this, color));
}

/**
 * Overrides the method from WidgetBase.
 */
void UMLWidget::setLineWidth(uint width)
{
    WidgetBase::setLineWidth(width);
    update();
}

/**
 * Sets the background fill color
 *
 * @param color the new fill color
 */
void UMLWidget::setFillColor(const QColor &color)
{
    UMLApp::app()->executeCommand(new CmdChangeFillColor(this, color));
}

/**
 * Sets the background fill color
 *
 * @param color the new fill color
 */
void UMLWidget::setFillColorcmd(const QColor &color)
{
    WidgetBase::setFillColor(color);
    update();
}

/**
 * Draws that the widget is selected.
 *
 * @param p Device on which is the selection is to be drawn.
 * @param offsetX The x-coordinate for drawing.
 * @param offsetY The y-coordinate for drawing.
 */
void UMLWidget::drawSelected(QPainter * p, int offsetX, int offsetY)
{
    int w = width();
    int h = height();
    int s = 4;
    QBrush brush(Qt::blue);
    p->fillRect(offsetX, offsetY, s,  s, brush);
    p->fillRect(offsetX, offsetY + h - s, s, s, brush);
    p->fillRect(offsetX + w - s, offsetY, s, s, brush);

    // Draw the resize anchor in the lower right corner.
    if (m_resizable) {
        brush.setColor(Qt::red);
        const int right = offsetX + w;
        const int bottom = offsetY + h;
        p->drawLine(right - s, offsetY + h - 1, offsetX + w - 1, offsetY + h - s);
        p->drawLine(right - (s*2), bottom - 1, right - 1, bottom - (s*2));
        p->drawLine(right - (s*3), bottom - 1, right - 1, bottom - (s*3));
    } else {
        p->fillRect(offsetX + w - s, offsetY + h - s, s, s, brush);
    }
}

/**
 * Activate the object after serializing it from a QDataStream
 *
 * @param ChangeLog
 * @return  true for success
 */
bool UMLWidget::activate(IDChangeLog* /*ChangeLog  = 0 */)
{
    if (widgetHasUMLObject(m_Type) && m_pObject == NULL) {
        m_pObject = m_doc->findObjectById(m_nId);
        if (m_pObject == NULL) {
            uError() << "cannot find UMLObject with id=" << ID2STR(m_nId);
            return false;
        }
    }
    setFont(m_Font);
    setSize(getWidth(), getHeight());
    m_activated = true;
    updateComponentSize();
    if (m_scene->getPaste()) {
        FloatingTextWidget * ft = 0;
        QPoint point = m_scene->getPastePoint();
        int x = point.x() + getX();
        int y = point.y() + getY();
        x = x < 0 ? 0 : x;
        y = y < 0 ? 0 : y;
        if (m_scene->type() == Uml::DiagramType::Sequence) {
            switch (baseType()) {
            case WidgetBase::wt_Object:
            case WidgetBase::wt_Precondition :
                setY(getY());
                setX(x);
                break;

            case WidgetBase::wt_Message:
                setY(getY());
                setX(x);
                break;

            case WidgetBase::wt_Text:
                ft = static_cast<FloatingTextWidget *>(this);
                if (ft->textRole() == Uml::TextRole::Seq_Message) {
                    setX(x);
                    setY(getY());
                } else {
                    setX(getX());
                    setY(getY());
                }
                break;

            default:
                setY(y);
                break;
            }//end switch base type
        }//end if sequence
        else {
            setX(x);
            setY(y);
        }
    }//end if pastepoint
    else {
        setX(getX());
        setY(getY());
    }
    if (m_scene->getPaste())
        m_scene->createAutoAssociations(this);
    updateComponentSize();
    return true;
}

/**
 * Returns true if the Activate method has been called for this instance
 *
 * @return The activate status.
 */
bool UMLWidget::isActivated()
{
    return m_activated;
}

/**
 * Set the m_activated flag of a widget but does not perform the Activate method
 *
 * @param Active Status of activation is to be set.
 */
void UMLWidget::setActivated(bool Active /*=true*/)
{
    m_activated = Active;
}

/**
 * Adds an already created association to the list of
 * associations that include this UMLWidget
 */
void UMLWidget::addAssoc(AssociationWidget* pAssoc)
{
    if (pAssoc && !m_Assocs.contains(pAssoc)) {
        m_Assocs.append(pAssoc);
    }
}

/**
 * Removes an already created association from the list of
 * associations that include this UMLWidget
 */
void UMLWidget::removeAssoc(AssociationWidget* pAssoc)
{
    if (pAssoc) {
        m_Assocs.removeAll(pAssoc);
    }
}

/**
 * Adjusts associations with the given co-ordinates
 *
 * @param x The x-coordinate.
 * @param y The y-coordinate.
 */
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
    if (m_doc->loading()) {
        // don't recalculate the assocs during load of XMI
        // -> return immediately without action
        return;
    }
    AssociationWidgetListIt assoc_it(m_Assocs);

    foreach(AssociationWidget* assocwidget , m_Assocs) {
        assocwidget->saveIdealTextPositions();
    }

    foreach(AssociationWidget* assocwidget , m_Assocs) {
        assocwidget->widgetMoved(this, x, y);
    }
}

/**
 * Adjusts all unselected associations with the given co-ordinates
 *
 * @param x The x-coordinate.
 * @param y The y-coordinate.
 */
void UMLWidget::adjustUnselectedAssocs(int x, int y)
{
    foreach(AssociationWidget* assocwidget , m_Assocs) {

        if (!assocwidget->getSelected())
            assocwidget->saveIdealTextPositions();
    }

    foreach(AssociationWidget* assocwidget , m_Assocs) {
        if (!assocwidget->getSelected())
            assocwidget->widgetMoved(this, x, y);
    }
}

/**
 * Show a properties dialog for a UMLWidget.
 */
void UMLWidget::showPropertiesDialog()
{
    // will already be selected so make sure docWindow updates the doc
    // back it the widget
    DocWindow *docwindow = UMLApp::app()->docWindow();
    docwindow->updateDocumentation(false);
    QPointer<ClassPropDlg> dlg = new ClassPropDlg((QWidget*)UMLApp::app(), this);

    if (dlg->exec()) {
        docwindow->showDocumentation(umlObject() , true);
        m_doc->setModified(true);
    }
    dlg->close(); //wipe from memory
    delete dlg;
}

/**
 * Starts the popup menu. If menu is non zero,
 * the widgets popup menu is embedded into another widget. 
 * The another widget is responsible for calling 
 * setupPopupMenu(), slotMenuSelection() and 
 * removePopupMenu() manually.
 *
 * @return pointer to the popup menu object
 */
ListPopupMenu* UMLWidget::setupPopupMenu(ListPopupMenu* menu)
{
    slotRemovePopupMenu();

    if (menu) {
        m_pMenu = menu;
        m_menuIsEmbedded = true;
        return m_pMenu;
    }

    m_menuIsEmbedded = false;
    //if in a multi- selection to a specific m_pMenu for that
    // NEW: ask UMLView to count ONLY the widgets and not their floatingtextwidgets
    int count = m_scene->getSelectCount(true);
    //a MessageWidget when selected will select its text widget and vice versa
    //so take that into account for popup menu.

    // determine multi state
    bool multi = (m_selected && count > 1);

    // if multiple selected items have the same type
    bool unique = false;

    // if multiple items are selected, we have to check if they all have the same
    // base type
    if (multi == true)
        unique = m_scene->checkUniqueSelection();

    // create the right click context menu
    m_pMenu = new ListPopupMenu(m_scene, this, multi, unique);

    // disable the "view code" menu for simple code generators
    if (UMLApp::app()->isSimpleCodeGeneratorActive())
        m_pMenu->setActionEnabled(ListPopupMenu::mt_ViewCode, false);

    connect(m_pMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotMenuSelection(QAction*)));

    return m_pMenu;
}

/**
 * This slot is entered when an event has occurred on the views display,
 * most likely a mouse event.  Before it sends out that mouse event all
 * children should make sure that they don't have a menu active or there
 * could be more than one popup menu displayed.
 */
void UMLWidget::slotRemovePopupMenu()
{
    if (m_pMenu) {
        if (!m_menuIsEmbedded) {
            disconnect(m_pMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotMenuSelection(QAction*)));
            delete m_pMenu;
        }
        m_pMenu = 0;
    }
}

/**
 * Returns 0 if the given point is not in the boundaries of the widget,
 * else returns a number which is proportional to the size of the widget.
 *
 * @param p Point to be checked.
 *
 * @return 0 if the given point is not in the boundaries of the widget;
 *         (width()+height())/2 if the point is within the boundaries.
 */
int UMLWidget::onWidget(const QPoint & p)
{
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

/**
 * Move the widget by an X and Y offset relative to
 * the current position.
 */
void UMLWidget::moveByLocal(int dx, int dy)
{
    int newX = getX() + dx;
    int newY = getY() + dy;
    setX(newX);
    setY(newY);
    // uDebug() << "********** x=" << newX << " / y=" << newY;
    adjustAssocs(newX, newY);
}

/**
 * Set the pen.
 */
void UMLWidget::setPenFromSettings(QPainter & p)
{
    p.setPen(QPen(m_LineColor, m_LineWidth));
}

/**
 * Overrides default method.
 *
 * @param p Device on which the shape has to be drawn.
 */
void UMLWidget::drawShape(QPainter &p)
{
    paint(p, getX(), getY());
}

/**
 * Compute the minimum possible width and height.
 *
 * @return UMLSceneSize(mininum_width, minimum_height)
 */
UMLSceneSize UMLWidget::minimumSize()
{
    return m_minimumSize;
}

/**
 * This method is used to set the minimum size variable for this
 * widget.
 *
 * @param newSize The size being set as minimum.
 */
void UMLWidget::setMinimumSize(const UMLSceneSize& newSize)
{
    m_minimumSize = newSize;
}

void UMLWidget::setMinimumSize(UMLSceneValue width, UMLSceneValue height)
{
    m_minimumSize.setWidth(width);
    m_minimumSize.setHeight(height);
}

/**
 * Compute the maximum possible width and height.
 *
 * @return maximum size
 */
UMLSceneSize UMLWidget::maximumSize()
{
    return m_maximumSize;
}

/**
 * This method is used to set the maximum size variable for this
 * widget.
 *
 * @param newSize The size being set as maximum.
 */
void UMLWidget::setMaximumSize(const UMLSceneSize& newSize)
{
    m_maximumSize = newSize;
}

/**
 * This method is used to set the maximum size variable for this
 * widget.
 *
 * @param width The width being set as maximum.
 * @param height The height being set as maximum.
 */
void UMLWidget::setMaximumSize(UMLSceneValue width, UMLSceneValue height)
{
    m_maximumSize.setWidth(width);
    m_maximumSize.setHeight(height);
}

/**
 * Sets the state of whether the widget is selected.
 *
 * @param _select The state of whether the widget is selected.
 */
void UMLWidget::setSelected(bool _select)
{
    const WidgetBase::WidgetType wt = m_Type;
    if (_select) {
        if (m_scene->getSelectCount() == 0) {
            if (widgetHasUMLObject(wt)) {
                m_scene->showDocumentation(m_pObject, false);
            } else {
                m_scene->showDocumentation(this, false);
            }
        }//end if
        /* if (wt != wt_Text && wt != wt_Box) {
            setZ(9);//keep text on top and boxes behind so don't touch Z value
        } */
    } else {
        /* if (wt != wt_Text && wt != wt_Box) {
            setZ(m_origZ);
        } */
        if (m_selected)
            m_scene->updateDocumentation(true);
    }
    m_selected = _select;

    const QPoint pos(getX(), getY());
    UMLWidget *bkgnd = m_scene->widgetAt(pos);
    if (bkgnd && bkgnd != this && _select) {
        uDebug() << "setting Z to " << bkgnd->getZ() + 1 << ", SelectState: " << _select;
        setZ(bkgnd->getZ() + 1);
    } else {
        setZ(m_origZ);
    }

    update();

    /* selection changed, we have to make sure the copy and paste items
     * are correctly enabled/disabled */
    UMLApp::app()->slotCopyChanged();
}

/**
 *   Captures a sigClearAllSelected signal sent by @ref UMLView
 */
void UMLWidget::slotClearAllSelected()
{
    setSelected(false);
}

/**
 * Sets the view the widget is on.
 *
 * @param v The view the widget is on.
 */
void UMLWidget::setScene(UMLScene * v)
{
    //remove signals from old view - was probably 0 anyway
    disconnect(m_scene, SIGNAL(sigRemovePopupMenu()), this, SLOT(slotRemovePopupMenu()));
    disconnect(m_scene, SIGNAL(sigClearAllSelected()), this, SLOT(slotClearAllSelected()));
    disconnect(m_scene, SIGNAL(sigColorChanged(Uml::IDType)), this, SLOT(slotColorChanged(Uml::IDType)));
    disconnect(m_scene, SIGNAL(sigLineWidthChanged(Uml::IDType)), this, SLOT(slotLineWidthChanged(Uml::IDType)));
    m_scene = v;
    connect(m_scene, SIGNAL(sigRemovePopupMenu()), this, SLOT(slotRemovePopupMenu()));
    connect(m_scene, SIGNAL(sigClearAllSelected()), this, SLOT(slotClearAllSelected()));
    connect(m_scene, SIGNAL(sigColorChanged(Uml::IDType)), this, SLOT(slotColorChanged(Uml::IDType)));
    connect(m_scene, SIGNAL(sigLineWidthChanged(Uml::IDType)), this, SLOT(slotLineWidthChanged(Uml::IDType)));
}

/**
 * Sets the x-coordinate.
 * Currently, the only class that reimplements this method is
 * ObjectWidget.
 *
 * @param x The x-coordinate to be set.
 */
void UMLWidget::setX(int x)
{
    if (!m_ignoreSnapToGrid) {
        x = m_scene->snappedX(x);
    }
    UMLSceneItem::setX(x);
}

/**
 * Sets the y-coordinate.
 * Currently, the only class that reimplements this method is
 * ObjectWidget.
 *
 * @param y The y-coordinate to be set.
 */
void UMLWidget::setY(int y)
{
    if (!m_ignoreSnapToGrid) {
        y = m_scene->snappedX(y);
    }
    UMLSceneItem::setY(y);
}

/**
 * Sets the z-coordinate.
 *
 * @param z The z-coordinate to be set.
 */
void UMLWidget::setZ(int z)
{
    m_origZ = getZ();
    UMLSceneItem::setZ(z);
}

/**
 * Sets the name in the corresponding UMLObject.
 * Sets the local m_Text if m_pObject is NULL.
 *
 * @param strName The name to be set.
 */
void UMLWidget::setName(const QString &strName)
{
    if (m_pObject)
        m_pObject->setName(strName);
    else
        m_Text = strName;
    updateComponentSize();
    adjustAssocs(getX(), getY());
}

/**
 * Gets the name from the corresponding UMLObject.
 * Returns the local m_Text if m_pObject is NULL.
 *
 * @return The currently set name.
 */
QString UMLWidget::name() const
{
    if (m_pObject)
        return m_pObject->name();
    return m_Text;
}

/**
 * Used to cleanup any other widget it may need to delete.
 * Used by child classes.  This should be called before deleting a widget of a diagram.
 */
void UMLWidget::cleanup()
{
}

/**
 * Tells the widget to snap to grid.
 * Will use the grid settings of the @ref UMLView it belongs to.
 */
void UMLWidget::slotSnapToGrid()
{
    setX(getX());
    setY(getY());
}

/**
 * Returns whether the widget type has an associated UMLObject
 */
bool UMLWidget::widgetHasUMLObject(WidgetBase::WidgetType type)
{
    if (type == WidgetBase::wt_Actor         ||
            type == WidgetBase::wt_UseCase   ||
            type == WidgetBase::wt_Class     ||
            type == WidgetBase::wt_Interface ||
            type == WidgetBase::wt_Enum      ||
            type == WidgetBase::wt_Datatype  ||
            type == WidgetBase::wt_Package   ||
            type == WidgetBase::wt_Component ||
            type == WidgetBase::wt_Node      ||
            type == WidgetBase::wt_Artifact  ||
            type == WidgetBase::wt_Object) {
        return true;
    } else {
        return false;
    }
}

/**
 * Set m_ignoreSnapToGrid.
 */
void UMLWidget::setIgnoreSnapToGrid(bool to)
{
    m_ignoreSnapToGrid = to;
}

/**
 * Return the value of m_ignoreSnapToGrid.
 */
bool UMLWidget::getIgnoreSnapToGrid() const
{
    return m_ignoreSnapToGrid;
}

/**
 * Sets the size.
 * If m_scene->getSnapComponentSizeToGrid() is true, then
 * set the next larger size that snaps to the grid.
 */
void UMLWidget::setSize(int width, int height)
{
    // snap to the next larger size that is a multiple of the grid
    if (!m_ignoreSnapComponentSizeToGrid
            && m_scene->getSnapComponentSizeToGrid()) {
        // integer divisions
        int numX = width / m_scene->getSnapX();
        int numY = height / m_scene->getSnapY();
        // snap to the next larger valid value
        if (width > numX * m_scene->getSnapX())
            width = (numX + 1) * m_scene->getSnapX();
        if (height > numY * m_scene->getSnapY())
            height = (numY + 1) * m_scene->getSnapY();
    }

    UMLSceneRectangle::setSize(width, height);
}

/**
 * Update the size of this widget.
 */
void UMLWidget::updateComponentSize()
{
    if (m_doc->loading())
        return;
    QSize size = minimumSize();
    setSize(size.width(), size.height());
    adjustAssocs(getX(), getY());    // adjust assoc lines
}

/**
 * clip the size of this widget against the
 * minimal and maximal limits.
 */
void UMLWidget::clipSize()
{
    int width = getWidth();
    int height = getHeight();
    constrain(width, height);
    setSize(width, height);
}

void UMLWidget::setDefaultFontMetrics(UMLWidget::FontType fontType)
{
    setupFontType(m_Font, fontType);
    setFontMetrics(fontType, QFontMetrics(m_Font));
}

void UMLWidget::setupFontType(QFont &font, UMLWidget::FontType fontType)
{
    switch (fontType) {
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

void UMLWidget::setDefaultFontMetrics(UMLWidget::FontType fontType, QPainter &painter)
{
    setupFontType(m_Font, fontType);
    painter.setFont(m_Font);
    setFontMetrics(fontType, painter.fontMetrics());
}

QFontMetrics &UMLWidget::getFontMetrics(UMLWidget::FontType fontType)
{
    if (m_pFontMetrics[fontType] == 0) {
        setDefaultFontMetrics(fontType);
    }
    return *m_pFontMetrics[fontType];
}

void UMLWidget::setFontMetrics(UMLWidget::FontType fontType, QFontMetrics fm)
{
    delete m_pFontMetrics[fontType];
    m_pFontMetrics[fontType] = new QFontMetrics(fm);
}

/**
 *  Returns the font the widget is to use.
 */
QFont UMLWidget::font() const
{
    return m_Font;
}

/**
 * Sets the font the widget is to use.
 *
 * @param font Font to be set.
 */
void UMLWidget::setFont(QFont font)
{
    m_Font = font;
    forceUpdateFontMetrics(0);
    if (m_doc->loading())
        return;
    update();
}

/**
 * @note For performance Reasons, only FontMetrics for already used
 *  font types are updated. Not yet used font types will not get a font metric
 *  and will get the same font metric as if painter was zero.
 *  This behaviour is acceptable, because diagrams will always be showed on Display
 *  first before a special painter like a printer device is used.
 */
void UMLWidget::forceUpdateFontMetrics(QPainter *painter)
{
    if (painter == 0) {
        for (int i = 0; i < (int)UMLWidget::FT_INVALID; ++i) {
            if (m_pFontMetrics[(UMLWidget::FontType)i] != 0)
                setDefaultFontMetrics((UMLWidget::FontType)i);
        }
    } else {
        for (int i2 = 0; i2 < (int)UMLWidget::FT_INVALID; ++i2) {
            if (m_pFontMetrics[(UMLWidget::FontType)i2] != 0)
                setDefaultFontMetrics((UMLWidget::FontType)i2, *painter);
        }
    }
    if (m_doc->loading())
        return;
    // calculate the size, based on the new font metric
    updateComponentSize();
}

/**
 * Set the status of whether to show Stereotype.
 *
 * @param _status   True if stereotype shall be shown.
 */
void UMLWidget::setShowStereotype(bool _status)
{
    m_showStereotype = _status;
    updateComponentSize();
    update();
}

/**
 * Returns the status of whether to show Stereotype.
 *
 * @return  True if stereotype is shown.
 */
bool UMLWidget::getShowStereotype() const
{
    return m_showStereotype;
}

/**
 * Overrides the standard operation.
 *
 * @param me The move event.
 */
void UMLWidget::moveEvent(QMoveEvent* /*me*/)
{
}

void UMLWidget::saveToXMI(QDomDocument & qDoc, QDomElement & qElement)
{
    /*
      Call after required actions in child class.
      Type must be set in the child class.
    */
    WidgetBase::saveToXMI(qDoc, qElement);
    qElement.setAttribute("xmi.id", ID2STR(id()));
    qElement.setAttribute("font", m_Font.toString());
    qElement.setAttribute("x", getX());
    qElement.setAttribute("y", getY());
    qElement.setAttribute("width", getWidth());
    qElement.setAttribute("height", getHeight());
    qElement.setAttribute("isinstance", m_isInstance);
    if (!m_instanceName.isEmpty())
        qElement.setAttribute("instancename", m_instanceName);
    if (m_showStereotype)
        qElement.setAttribute("showstereotype", m_showStereotype);
}

bool UMLWidget::loadFromXMI(QDomElement & qElement)
{
    WidgetBase::loadFromXMI(qElement);
    QString id = qElement.attribute("xmi.id", "-1");
    QString font = qElement.attribute("font", "");
    QString x = qElement.attribute("x", "0");
    QString y = qElement.attribute("y", "0");
    QString h = qElement.attribute("height", "0");
    QString w = qElement.attribute("width", "0");

    m_nId = STR2ID(id);

    if (!font.isEmpty()) {
        QFont newFont;
        newFont.fromString(font);
        setFont(newFont);
    } else {
        uWarning() << "Using default font " << m_Font.toString()
        << " for widget with xmi.id " << ID2STR(m_nId) << endl;
    }

    setSize(w.toInt(), h.toInt());
    setX(x.toInt());
    setY(y.toInt());
    QString isinstance = qElement.attribute("isinstance", "0");
    m_isInstance = (bool)isinstance.toInt();
    m_instanceName = qElement.attribute("instancename", "");
    QString showstereo = qElement.attribute("showstereotype", "0");
    m_showStereotype = (bool)showstereo.toInt();
    return true;
}

/**
 * Returns the UMLWdigetController for this widget.
 */
UMLWidgetController* UMLWidget::getWidgetController()
{
    return m_widgetController;
}

#include "umlwidget.moc"
