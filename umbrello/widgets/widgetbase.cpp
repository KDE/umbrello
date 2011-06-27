/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "widgetbase.h"

#include "debug_utils.h"
#include "optionstate.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlobject.h"

/**
 * Creates a WidgetBase object.
 *
 * @param view      The view to be displayed on.
 */
WidgetBase::WidgetBase(UMLView *view) : QObject(view)
{
    init(view);
}

/**
 * Initialize members.
 */
void WidgetBase::init(UMLView *view, WidgetType type /* = wt_UMLWidget */)
{
    m_pView = view;
    m_Type = type;
    m_pObject = NULL;
    if (m_pView) {
        m_bUsesDiagramLineColour = true;
        m_bUsesDiagramLineWidth  = true;
        const Settings::OptionState& optionState = m_pView->optionState();
        m_LineColour = optionState.uiState.lineColor;
        m_LineWidth  = optionState.uiState.lineWidth;
    } else {
        uError() << "WidgetBase constructor: SERIOUS PROBLEM - m_pView is NULL";
        m_bUsesDiagramLineColour = false;
        m_bUsesDiagramLineWidth  = false;
        m_LineColour = QColor("black");
        m_LineWidth = 0; // initialize with 0 to have valid start condition
    }
}

/**
 * Destructor.
 */
WidgetBase::~WidgetBase()
{
}

/**
 * Set property m_Type.
 */
void WidgetBase::setBaseType(WidgetBase::WidgetType type)
{
    m_Type = type;
}

/**
 * Read property of m_Type.
 */
WidgetBase::WidgetType WidgetBase::baseType() const
{
    return m_Type;
}

/**
 * @return The type used for rtti as string.
 */
QLatin1String WidgetBase::baseTypeStr() const
{
    return QLatin1String(ENUM_NAME(WidgetBase, WidgetType, m_Type));
}

/**
 * Deliver a pointer to the connected UMLView
 * ( needed esp. by event handling of LinePath ).
 */
UMLView* WidgetBase::umlScene() const
{
    return m_pView;
}

/**
 * This is shortcut method for UMLApp::app()->document().
 *
 * @return Pointer to the UMLDoc object.
 */
UMLDoc* WidgetBase::umlDoc() const
{
    return UMLApp::app()->document();
}

/**
 * Returns the @ref UMLObject set to represent.
 *
 * @return the UMLObject to represent.
 */
UMLObject* WidgetBase::umlObject() const
{
    return m_pObject;
}

/**
 * Sets the @ref UMLObject to represent.
 *
 * @param o The object to represent.
 */
void WidgetBase::setUMLObject(UMLObject * o)
{
    m_pObject = o;
}

/**
 * Write property of m_nId.
 */
void WidgetBase::setID(Uml::IDType id)
{
    if (m_pObject) {
        if (m_pObject->id() != Uml::id_None)
            uWarning() << "changing old UMLObject " << ID2STR(m_pObject->id())
                << " to " << ID2STR(id);
        m_pObject->setID(id);
    }
    m_nId = id;
}

/**
 * Read property of m_nId.
 */
Uml::IDType WidgetBase::id() const
{
    if (m_pObject)
        return m_pObject->id();
    return m_nId;
}

/**
 * Used by some child classes to get documentation.
 *
 * @return  The documentation from the UMLObject (if m_pObject is set.)
 */
QString WidgetBase::documentation() const
{
    if (m_pObject)
        return m_pObject->doc();
    return m_Doc;
}

/**
 * Used by some child classes to set documentation.
 *
 * @param doc   The documentation to be set in the UMLObject
 *              (if m_pObject is set.)
 */
void WidgetBase::setDocumentation( const QString &doc )
{
    if (m_pObject)
        m_pObject->setDoc( doc );
    else
        m_Doc = doc;
}

/**
 * Read property of m_LineColour.
 */
QColor WidgetBase::lineColor() const
{
    return m_LineColour;
}

/**
 * Sets the line colour
 *
 * @param colour the new line colour
 */
void WidgetBase::setLineColor(const QColor &colour)
{
    m_LineColour = colour;
    m_bUsesDiagramLineColour = false;
}

/**
 * Read property of m_LineWidth.
 */
uint WidgetBase::lineWidth() const
{
    return m_LineWidth;
}

/**
 * Sets the line width
 *
 * @param width the new line width
 */
void WidgetBase::setLineWidth(uint width)
{
    m_LineWidth = width;
    m_bUsesDiagramLineWidth = false;
}

/**
 * Returns m_bUsesDiagramLineColour
 */
bool WidgetBase::usesDiagramLineColour() const
{
    return m_bUsesDiagramLineColour;
}

/**
 * Sets m_bUsesDiagramLineColour
 */
void WidgetBase::setUsesDiagramLineColour(bool usesDiagramLineColour)
{
    m_bUsesDiagramLineColour = usesDiagramLineColour;
}

/**
 * Returns m_bUsesDiagramLineWidth
 */
bool WidgetBase::usesDiagramLineWidth() const {
    return m_bUsesDiagramLineWidth;
}

/**
 * Sets m_bUsesDiagramLineWidth
 */
void WidgetBase::setUsesDiagramLineWidth(bool usesDiagramLineWidth)
{
    m_bUsesDiagramLineWidth = usesDiagramLineWidth;
}

void WidgetBase::saveToXMI( QDomDocument & /*qDoc*/, QDomElement & qElement )
{
    if (m_bUsesDiagramLineColour) {
        qElement.setAttribute( "linecolor", "none" );
    } else {
        qElement.setAttribute( "linecolor", m_LineColour.name() );
    }
    if (m_bUsesDiagramLineWidth) {
        qElement.setAttribute( "linewidth", "none" );
    } else {
        qElement.setAttribute( "linewidth", m_LineWidth );
    }
}

bool WidgetBase::loadFromXMI( QDomElement & qElement )
{
    // first load from "linecolour" and then overwrite with the "linecolor"
    // attribute if that one is present. The "linecolour" name was a "typo" in
    // earlier versions of Umbrello
    QString lineColor = qElement.attribute( "linecolour", "none" );
    lineColor = qElement.attribute( "linecolor", lineColor );

    QString lineWidth = qElement.attribute( "linewidth", "none" );
    if (lineColor != "none") {
        setLineColor( QColor(lineColor) );
        m_bUsesDiagramLineColour = false;
    } else if (m_Type != WidgetBase::wt_Box && m_pView != NULL) {
        setLineColor( m_pView->getLineColor() );
        m_bUsesDiagramLineColour = true;
    }
    if (lineWidth != "none") {
        setLineWidth( lineWidth.toInt() );
        m_bUsesDiagramLineWidth = false;
    } else if ( m_pView ) {
        setLineWidth( m_pView->getLineWidth() );
        m_bUsesDiagramLineWidth = true;
    }
    return true;
}

#include "widgetbase.moc"
