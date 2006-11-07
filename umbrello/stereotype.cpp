/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "stereotype.h"

// qt/kde includes
#include <klocale.h>
#include <kdebug.h>
#include <kinputdialog.h>

// local includes
#include "umldoc.h"
#include "uml.h"

UMLStereotype::UMLStereotype(const QString &name, Uml::IDType id /* = Uml::id_None */)
        : UMLObject( name, id ) {
    m_BaseType = Uml::ot_Stereotype;
    UMLStereotype * existing = UMLApp::app()->getDocument()->findStereotype(name);
    if (existing) {
        kError() << "UMLStereotype constructor: " << name << " already exists"
                  << kdBacktrace(25) << endl;
    }
    m_refCount = 0;
}

UMLStereotype::UMLStereotype() : UMLObject() {
    m_BaseType = Uml::ot_Stereotype;
    m_refCount = 0;
}

UMLStereotype::~UMLStereotype() {}

bool UMLStereotype::operator==( UMLStereotype &rhs) {
    if (this == &rhs) {
        return true;
    }

    if ( !UMLObject::operator==( rhs ) ) {
        return false;
    }

    return true;
}

void UMLStereotype::copyInto(UMLStereotype *rhs) const
{
    UMLObject::copyInto(rhs);
}

UMLObject* UMLStereotype::clone() const
{
    UMLStereotype *clone = new UMLStereotype();
    copyInto(clone);

    return clone;
}


void UMLStereotype::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    //FIXME: uml13.dtd compliance
    QDomElement stereotypeElement = UMLObject::save("UML:Stereotype", qDoc);
    qElement.appendChild( stereotypeElement );
}

bool UMLStereotype::showPropertiesDialog(QWidget* parent) {
    bool ok;
    QString name = KInputDialog::getText(i18n("Stereotype"), i18n("Enter name:"), getName(),&ok, parent);
    if (ok) {
        setName(name);
    }
    return ok;
}

void UMLStereotype::incrRefCount() {
    m_refCount++;
}

void UMLStereotype::decrRefCount() {
    m_refCount--;
}

int UMLStereotype::refCount() const {
    return m_refCount;
}

