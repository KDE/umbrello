 /*
  *  copyright (C) 2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "widgetbase.h"

#include <kdebug.h>
#include "umlview.h"
#include "umlobject.h"

WidgetBase::WidgetBase(UMLView *view)
  : QObject(view), m_Type(Uml::wt_UMLWidget), m_pView(view), m_pObject(NULL) {
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

QString WidgetBase::getDoc() const {
	if (m_pObject != NULL)
		return m_pObject->getDoc();
	return "";
}

void WidgetBase::setDoc( const QString &doc ) {
	if (m_pObject != NULL)
		m_pObject->setDoc( doc );
}

#include "widgetbase.moc"
