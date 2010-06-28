/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2010                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "widgetbase.h"

#include <kdebug.h>
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlobject.h"
#include "optionstate.h"

/**
 * Creates a WidgetBase object.
 *
 * @param view      The view to be displayed on.
 */
WidgetBase::WidgetBase(UMLView *view) : QObject(view)
{
    init(view);
}

void WidgetBase::init(UMLView *view, Uml::Widget_Type type /* = Uml::wt_UMLWidget */)
{
    m_pView = view;
    m_Type = type;
    m_pObject = NULL;
    if (m_pView) {
        m_bUsesDiagramLineColour = true;
        m_bUsesDiagramLineWidth  = true;
        const Settings::OptionState& optionState = m_pView->getOptionState();
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

void WidgetBase::setBaseType( Uml::Widget_Type type )
{
    m_Type = type;
}

Uml::Widget_Type WidgetBase::baseType() const
{
    return m_Type;
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

UMLObject* WidgetBase::umlObject() const
{
    return m_pObject;
}

void WidgetBase::setUMLObject(UMLObject * o)
{
    m_pObject = o;
}

void WidgetBase::setID(Uml::IDType id)
{
    if (m_pObject) {
        if (m_pObject->getID() != Uml::id_None)
            uWarning() << "changing old UMLObject " << ID2STR(m_pObject->getID())
                << " to " << ID2STR(id) << endl;
        m_pObject->setID(id);
    }
    m_nId = id;
}

Uml::IDType WidgetBase::id() const
{
    if (m_pObject)
        return m_pObject->getID();
    return m_nId;
}

QString WidgetBase::documentation() const
{
    if (m_pObject)
        return m_pObject->getDoc();
    return m_Doc;
}

void WidgetBase::setDocumentation( const QString &doc )
{
    if (m_pObject)
        m_pObject->setDoc( doc );
    else
        m_Doc = doc;
}

void WidgetBase::setLineColor(const QColor &colour)
{
    m_LineColour = colour;
    m_bUsesDiagramLineColour = false;
}

void WidgetBase::setLineWidth(uint width)
{
    m_LineWidth = width;
    m_bUsesDiagramLineWidth = false;
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
    } else if (m_Type != Uml::wt_Box && m_pView != NULL) {
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
