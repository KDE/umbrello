/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "category.h"

UMLCategory::UMLCategory(const QString & name, Uml::IDType id) : UMLCanvasObject(name, id) {
    init();
}

UMLCategory::~UMLCategory() {}

void UMLCategory::init() {
    m_BaseType = Uml::ot_Category;
    m_CategoryType = ct_Disjoint_Specialisation;
}

void UMLCategory::copyInto(UMLCategory* rhs) const {
    // call the parent first
    UMLCanvasObject::copyInto(rhs);

    rhs->m_CategoryType = m_CategoryType;
}

UMLObject* UMLCategory::clone() const {
    UMLCategory *clone = new UMLCategory();
    copyInto(clone);
    return clone;
}

void UMLCategory::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
     QDomElement categoryElement = UMLObject::save("UML:Category", qDoc);
     categoryElement.setAttribute( "categoryType", ( int )m_CategoryType );
     qElement.appendChild(categoryElement);
}

bool UMLCategory::load(QDomElement& element) {
    m_CategoryType = (Category_Type)element.attribute("categoryType", "0" ).toInt();
    return true;
}


UMLCategory::Category_Type UMLCategory::getType() {
    return m_CategoryType;
}

void UMLCategory::setType(Category_Type type) {
    m_CategoryType = type;
}



#include "category.moc"
