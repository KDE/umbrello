/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "widgetbase.h"

#include <kdebug.h>
#include "umlview.h"
#include "umlobject.h"
#include "optionstate.h"

WidgetBase::WidgetBase(UMLView *view) : QObject(view) {
    init(view);
}

void WidgetBase::init(UMLView *view, Uml::Widget_Type type /* = Uml::wt_UMLWidget */) {
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
        kError() << "WidgetBase constructor: SERIOUS PROBLEM - m_pView is NULL" << endl;
        m_bUsesDiagramLineColour = false;
        m_bUsesDiagramLineWidth  = false;
        m_LineColour = QColor("black");
        m_LineWidth = 0; // initialize with 0 to have valid start condition
    }
}

void WidgetBase::setBaseType( Uml::Widget_Type type ) {
    m_Type = type;
}

Uml::Widget_Type WidgetBase::getBaseType() const {
    return m_Type;
}

UMLObject *WidgetBase::getUMLObject() {
    return m_pObject;
}

void WidgetBase::setUMLObject(UMLObject * o) {
    m_pObject = o;
}

void WidgetBase::setID(Uml::IDType id) {
    if (m_pObject) {
        if (m_pObject->getID() != Uml::id_None)
            kWarning() << "WidgetBase::setID(): changing old UMLObject "
            << ID2STR(m_pObject->getID()) << " to "
            << ID2STR(id) << endl;
        m_pObject->setID(id);
    }
    m_nId = id;
}

Uml::IDType WidgetBase::getID() const {
    if (m_pObject)
        return m_pObject->getID();
    return m_nId;
}

QString WidgetBase::getDoc() const {
    if (m_pObject != NULL)
        return m_pObject->getDoc();
    return m_Doc;
}

void WidgetBase::setDoc( const QString &doc ) {
    if (m_pObject != NULL)
        m_pObject->setDoc( doc );
    else
        m_Doc = doc;
}

void WidgetBase::setLineColor(const QColor &colour) {
    m_LineColour = colour;
    m_bUsesDiagramLineColour = false;
}

void WidgetBase::setLineWidth(uint width) {
    m_LineWidth = width;
    m_bUsesDiagramLineWidth = false;
}

void WidgetBase::saveToXMI( QDomDocument & /*qDoc*/, QDomElement & qElement ) {
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

bool WidgetBase::loadFromXMI( QDomElement & qElement ) {
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
