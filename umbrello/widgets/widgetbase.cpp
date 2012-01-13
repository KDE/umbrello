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
        m_usesDiagramLineColor = true;
        m_usesDiagramLineWidth  = true;
        m_usesDiagramTextColor = true;
        const Settings::OptionState& optionState = m_scene->optionState();
        m_textColor = optionState.uiState.textColor;
        m_LineColor = optionState.uiState.lineColor;
        m_LineWidth  = optionState.uiState.lineWidth;
    } else {
        uError() << "WidgetBase constructor: SERIOUS PROBLEM - m_scene is NULL";
        m_usesDiagramLineColor = false;
        m_usesDiagramLineWidth  = false;
        m_usesDiagramTextColor = false;
        m_textColor =  QColor("black");
        m_LineColor = QColor("black");
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
 * ( needed esp. by event handling of AssociationLine ).
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
 * Returns text color
 *
 * @return currently used text color
 */
QColor WidgetBase::textColor() const
{
    return m_textColor;
}

/**
 * Sets the text color
 *
 * @param color the new text color
 */
void WidgetBase::setTextColor(const QColor &color)
{
    m_textColor = color;
    m_usesDiagramTextColor = false;
}

/**
 * Returns line color
 *
 * @return currently used line color
 */
QColor WidgetBase::lineColor() const
{
    return m_LineColor;
}

/**
 * Sets the line color
 *
 * @param color   The new line color
 */
void WidgetBase::setLineColor(const QColor &color)
{
    m_LineColor = color;
    m_usesDiagramLineColor = false;
}

/**
 * Returns fill color
 *
 * @return currently used fill color
 */
QColor WidgetBase::fillColor() const
{
    return m_FillColor;
}

/**
 * Sets the fill color
 *
 * @param colour   The new fill color
 */
void WidgetBase::setFillColor(const QColor &color)
{
    m_FillColor = color;
    m_usesDiagramFillColor = false;
}

/**
 * Returns line width
 *
 * @return currently used line with
 */
uint WidgetBase::lineWidth() const
{
    return m_LineWidth;
}

/**
 * Sets the line width
 *
 * @param width  The new line width
 */
void WidgetBase::setLineWidth(uint width)
{
    m_LineWidth = width;
    m_usesDiagramLineWidth = false;
}

/**
 * Return state of fill color usage
 *
 * @return True if fill color is used
 */
bool WidgetBase::useFillColor()
{
    return m_useFillColor;
}

/**
 * Set state if fill color is used
 *
 * @param state  The state to set
 */
void WidgetBase::setUseFillColor(bool state)
{
    m_useFillColor = state;
    m_usesDiagramUseFillColor = false;
}

/**
 * Returns state if diagram text color is used
 *
 * @return True means diagram text color is used
 */
bool WidgetBase::usesDiagramTextColor() const
{
    return m_usesDiagramTextColor;
}

/**
 * Set state if diagram text color is used
 *
 * @param state  The state to set
 */
void WidgetBase::setUsesDiagramTextColor(bool state)
{
    if (m_usesDiagramTextColor == state) {
        return;
    }
    m_usesDiagramTextColor = state;
    setTextColor(m_textColor);
}

/**
 * Returns state of diagram line color is used
 *
 * @return True means diagrams line color is used
*/
bool WidgetBase::usesDiagramLineColor() const
{
    return m_usesDiagramLineColor;
}

/**
 * Set state of diagram line color is used
 *
 * @param state  The state to set
 */
void WidgetBase::setUsesDiagramLineColor(bool state)
{
    m_usesDiagramLineColor = state;
}

/**
 * Returns state of diagram fill color is used
 *
 * @return True means diagrams fill color is used
*/
bool WidgetBase::usesDiagramFillColor() const
{
    return m_usesDiagramFillColor;
}

/**
 * Set state if diagram fill color is used
 *
 * @param state  The state to set
 */
void WidgetBase::setUsesDiagramFillColor(bool state)
{
    m_usesDiagramFillColor = state;
}

/**
 * Returns state of diagram use fill color is used
 *
 * @return True means diagrams fill color is used
*/
bool WidgetBase::usesDiagramUseFillColor() const
{
    return m_usesDiagramUseFillColor;
}

/**
 * Set state of diagram use fill color is used
 *
 * @param state  The state to set
 */
void WidgetBase::setUsesDiagramUseFillColor(bool state)
{
    m_usesDiagramUseFillColor = state;
}

/**
 * Returns state of diagram line width is used
 *
 * @return True means diagrams line width is used
 */
bool WidgetBase::usesDiagramLineWidth() const
{
    return m_usesDiagramLineWidth;
}

/**
 * Set state of diagram line width is used
 *
 * @param state  The state to set
 */
void WidgetBase::setUsesDiagramLineWidth(bool state)
{
    m_usesDiagramLineWidth = state;
}

void WidgetBase::saveToXMI( QDomDocument & /*qDoc*/, QDomElement & qElement )
{
    qElement.setAttribute( "textcolor", m_usesDiagramTextColor ? "none" : m_textColor.name() );

    if (m_usesDiagramLineColor) {
        qElement.setAttribute( "linecolor", "none" );
    } else {
        qElement.setAttribute( "linecolor", m_LineColor.name() );
    }
    if (m_usesDiagramLineWidth) {
        qElement.setAttribute( "linewidth", "none" );
    } else {
        qElement.setAttribute( "linewidth", m_LineWidth );
    }
    qElement.setAttribute("usefillcolor", m_useFillColor);
    // for consistency the following attributes now use american spelling for "color"
    qElement.setAttribute("usesdiagramfillcolor", m_usesDiagramFillColor);
    qElement.setAttribute("usesdiagramusefillcolor", m_usesDiagramUseFillColor);
    if (m_usesDiagramFillColor) {
        qElement.setAttribute("fillcolor", "none");
    } else {
        qElement.setAttribute("fillcolor", m_FillColor.name());
    }
}

bool WidgetBase::loadFromXMI( QDomElement & qElement )
{
    // first load from "linecolour" and then overwrite with the "linecolor"
    // attribute if that one is present. The "linecolour" name was a "typo" in
    // earlier versions of Umbrello
    QString lineColor = qElement.attribute( "linecolour", "none" );
    lineColor = qElement.attribute( "linecolor", lineColor );
    if (lineColor != "none") {
        setLineColor( QColor(lineColor) );
        m_usesDiagramLineColor = false;
    } else if (m_Type != WidgetBase::wt_Box && m_scene != NULL) {
        setLineColor( m_scene->lineColor() );
        m_usesDiagramLineColor = true;
    }
    QString lineWidth = qElement.attribute( "linewidth", "none" );
    if (lineWidth != "none") {
        setLineWidth( lineWidth.toInt() );
        m_usesDiagramLineWidth = false;
    } else if ( m_scene ) {
        setLineWidth( m_scene->lineWidth() );
        m_usesDiagramLineWidth = true;
    }
    QString textColor = qElement.attribute( "textcolor", "none" );
    if (textColor != "none") {
        setTextColor( QColor(textColor) );
        m_usesDiagramTextColor = false;
    } else if ( m_scene ) {
        setTextColor( m_scene->textColor() );
        m_usesDiagramTextColor = true;
    }
    QString usefillcolor = qElement.attribute("usefillcolor", "1");
    m_useFillColor = (bool)usefillcolor.toInt();
    /*
      For the next three *color attributes, there was a mixup of american and english spelling for "color".
      So first we need to keep backward compatibility and try to retrieve the *colour attribute.
      Next we overwrite this value if we find a *color, otherwise the former *colour is kept.
    */
    QString fillColor = qElement.attribute("fillcolour", "none");
    fillColor = qElement.attribute("fillcolor", fillColor);
    if (fillColor != "none") {
        m_FillColor = QColor(fillColor);
    }

    QString usesDiagramFillColor = qElement.attribute("usesdiagramfillcolour", "1");
    usesDiagramFillColor = qElement.attribute("usesdiagramfillcolor", usesDiagramFillColor);
    m_usesDiagramFillColor = (bool)usesDiagramFillColor.toInt();

    QString usesDiagramUseFillColor = qElement.attribute("usesdiagramusefillcolour", "1");
    usesDiagramUseFillColor = qElement.attribute("usesdiagramusefillcolor", usesDiagramUseFillColor);
    m_usesDiagramUseFillColor = (bool)usesDiagramUseFillColor.toInt();

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
    m_LineColor = other.m_LineColor;
    m_LineWidth  = other.m_LineWidth;
    m_usesDiagramLineColor = other.m_usesDiagramLineColor;
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
