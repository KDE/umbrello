/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "widgetbase.h"

#include "debug_utils.h"
#include "optionstate.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlobject.h"

/**
 * Creates a WidgetBase object.
 *
 * @param scene   The view to be displayed on.
 */
WidgetBase::WidgetBase(UMLScene *scene, WidgetType type)
  : m_Type(type),
    m_scene(scene)
{
    init();
}

/**
 * Initialize members.
 */
void WidgetBase::init()
{
    m_pObject = 0;
    if (m_scene) {
        m_usesDiagramLineColour = true;
        m_usesDiagramLineWidth  = true;
        const Settings::OptionState& optionState = m_scene->optionState();
        m_LineColour = optionState.uiState.lineColor;
        m_LineWidth  = optionState.uiState.lineWidth;
    } else {
        uError() << "WidgetBase constructor: SERIOUS PROBLEM - m_scene is NULL";
        m_usesDiagramLineColour = false;
        m_usesDiagramLineWidth  = false;
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
UMLScene* WidgetBase::umlScene() const
{
    return m_scene;
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
    m_usesDiagramLineColour = false;
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
    m_usesDiagramLineWidth = false;
}

/**
 * Returns m_usesDiagramLineColour
 */
bool WidgetBase::usesDiagramLineColour() const
{
    return m_usesDiagramLineColour;
}

/**
 * Sets m_usesDiagramLineColour
 */
void WidgetBase::setUsesDiagramLineColour(bool usesDiagramLineColour)
{
    m_usesDiagramLineColour = usesDiagramLineColour;
}

/**
 * Returns m_usesDiagramLineWidth
 */
bool WidgetBase::usesDiagramLineWidth() const {
    return m_usesDiagramLineWidth;
}

/**
 * Sets m_usesDiagramLineWidth
 */
void WidgetBase::setUsesDiagramLineWidth(bool usesDiagramLineWidth)
{
    m_usesDiagramLineWidth = usesDiagramLineWidth;
}

void WidgetBase::saveToXMI( QDomDocument & /*qDoc*/, QDomElement & qElement )
{
    if (m_usesDiagramLineColour) {
        qElement.setAttribute( "linecolor", "none" );
    } else {
        qElement.setAttribute( "linecolor", m_LineColour.name() );
    }
    if (m_usesDiagramLineWidth) {
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
        m_usesDiagramLineColour = false;
    } else if (m_Type != WidgetBase::wt_Box && m_scene != NULL) {
        setLineColor( m_scene->getLineColor() );
        m_usesDiagramLineColour = true;
    }
    if (lineWidth != "none") {
        setLineWidth( lineWidth.toInt() );
        m_usesDiagramLineWidth = false;
    } else if ( m_scene ) {
        setLineWidth( m_scene->getLineWidth() );
        m_usesDiagramLineWidth = true;
    }
    return true;
}

/**
 * Assignment operator
 */
WidgetBase& WidgetBase::operator=(const WidgetBase& other)
{
    m_Type = other.m_Type;
    m_scene = other.m_scene;
    m_pObject = other.m_pObject;
    m_Doc = other.m_Doc;
    m_nId = other.m_nId;
    m_LineColour = other.m_LineColour;
    m_LineWidth  = other.m_LineWidth;
    m_usesDiagramLineColour = other.m_usesDiagramLineColour;
    m_usesDiagramLineWidth  = other.m_usesDiagramLineWidth;

    return *this;
}

/**
 * Helper function for debug output.
 * Returns the given enum value as string.
 * @param wt   WidgetType of which a string representation is wanted
 * @return   the WidgetType as string
 */
QString WidgetBase::toString(WidgetType wt)
{
    return QLatin1String(ENUM_NAME(WidgetBase, WidgetType, wt));
}

#include "widgetbase.moc"
