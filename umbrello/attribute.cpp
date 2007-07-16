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

// own header
#include "attribute.h"
// qt/kde includes
#include <qregexp.h>
#include <kdebug.h>
// app includes
#include "classifier.h"
#include "operation.h"
#include "umlobject.h"
#include "umldoc.h"
#include "uml.h"
#include "dialogs/umlattributedialog.h"
#include "object_factory.h"

UMLAttribute::UMLAttribute( const UMLObject *parent,
                            const QString& name, Uml::IDType id,
                            Uml::Visibility s,
                            UMLObject *type, const QString& iv )
        : UMLClassifierListItem(parent, name, id) {
    m_InitialValue = iv;
    m_BaseType = Uml::ot_Attribute;
    m_Vis = s;
    m_ParmKind = Uml::pd_In;
    /* CHECK: Do we need this:
    if (type == NULL) {
        type = Object_Factory::createUMLObject(Uml::ot_Datatype, "undef");
    }
     */
    m_pSecondary = type;
}

UMLAttribute::UMLAttribute(const UMLObject *parent) : UMLClassifierListItem(parent) {
    m_BaseType = Uml::ot_Attribute;
    m_Vis = Uml::Visibility::Private;
    m_ParmKind = Uml::pd_In;
}

UMLAttribute::~UMLAttribute() { }

void UMLAttribute::setName(const QString &name) {
    m_Name = name;
    emit attributeChanged();
    UMLObject::emitModified();
}

void UMLAttribute::setVisibility(Uml::Visibility s) {
    m_Vis = s;
    emit attributeChanged();
    UMLObject::emitModified();
}

QString UMLAttribute::getInitialValue() {
    return m_InitialValue;
}

void UMLAttribute::setInitialValue(const QString &iv) {
    if(m_InitialValue != iv) {
        m_InitialValue = iv;
        UMLObject::emitModified();
    }
}

void UMLAttribute::setParmKind (Uml::Parameter_Direction pk) {
    m_ParmKind = pk;
}

Uml::Parameter_Direction UMLAttribute::getParmKind () const {
    return m_ParmKind;
}

QString UMLAttribute::toString(Uml::Signature_Type sig) {
    QString s;

    if(sig == Uml::st_ShowSig || sig == Uml::st_NoSig) {
        s = m_Vis.toString(true) + ' ';
    }

    if(sig == Uml::st_ShowSig || sig == Uml::st_SigNoVis) {
        // Determine whether the type name needs to be scoped.
        UMLObject *owningObject = static_cast<UMLObject*>(parent());
        if (owningObject->getBaseType() == Uml::ot_Operation) {
            // The immediate parent() is the UMLOperation but we want
            // the UMLClassifier:
            owningObject = static_cast<UMLObject*>(owningObject->parent());
        }
        UMLClassifier *ownParent = dynamic_cast<UMLClassifier*>(owningObject);
        if (ownParent == NULL) {
            kError() << "UMLAttribute::toString: parent "
            << owningObject->getName()
            << " is not a UMLClassifier" << endl;
            return "";
        }
        QString typeName;
        UMLClassifier *type = UMLClassifierListItem::getType();
        if (type) {
            UMLPackage *typeScope = type->getUMLPackage();
            if (typeScope != ownParent && typeScope != ownParent->getUMLPackage())
                typeName = type->getFullyQualifiedName();
            else
                typeName = type->getName();
        }
        // The default direction, "in", is not mentioned.
        // Perhaps we should include a pd_Unspecified in
        // Uml::Parameter_Direction to have better control over this.
        if (m_ParmKind == Uml::pd_InOut)
            s += "inout ";
        else if (m_ParmKind == Uml::pd_Out)
            s += "out ";
        // Construct the attribute text.
        QString string = s + getName() + " : " + typeName;
        if(m_InitialValue.length() > 0)
            string += " = " + m_InitialValue;
        return string;
    }
    return s + getName();
}

QString UMLAttribute::getFullyQualifiedName( const QString& separator,
                                            bool includeRoot /* = false */) const {
    UMLOperation *op = NULL;
    UMLObject *owningObject = static_cast<UMLObject*>(parent());
    if (owningObject->getBaseType() == Uml::ot_Operation) {
        op = static_cast<UMLOperation*>(owningObject);
        owningObject = static_cast<UMLObject*>(owningObject->parent());
    }
    UMLClassifier *ownParent = dynamic_cast<UMLClassifier*>(owningObject);
    if (ownParent == NULL) {
        kError() << "UMLAttribute::getFullyQualifiedName(" << m_Name
        << "): parent " << owningObject->getName()
        << " is not a UMLClassifier" << endl;
        return "";
    }
    QString tempSeparator = separator;
    if (tempSeparator.isEmpty())
        tempSeparator = UMLApp::app()->activeLanguageScopeSeparator();
    QString fqn = ownParent->getFullyQualifiedName(tempSeparator, includeRoot);
    if (op)
        fqn.append(tempSeparator + op->getName());
    fqn.append(tempSeparator + m_Name);
    return fqn;
}

bool UMLAttribute::operator==( UMLAttribute &rhs) {
    if( this == &rhs )
        return true;

    if( !UMLObject::operator==( rhs ) )
        return false;

    // The type name is the only distinguishing criterion.
    // (Some programming languages might support more, but others don't.)
    if (m_pSecondary != rhs.m_pSecondary)
        return false;

    return true;
}

void UMLAttribute::copyInto(UMLAttribute *rhs) const
{
    // call the parent first.
    UMLClassifierListItem::copyInto(rhs);

    // Copy all datamembers
    rhs->m_pSecondary = m_pSecondary;
    rhs->m_SecondaryId = m_SecondaryId;
    rhs->m_InitialValue = m_InitialValue;
    rhs->m_ParmKind = m_ParmKind;
}

UMLObject* UMLAttribute::clone() const
{
    //FIXME: The new attribute should be slaved to the NEW parent not the old.
    UMLAttribute *clone = new UMLAttribute( static_cast<UMLObject*>(parent()) );
    copyInto(clone);

    return clone;
}


void UMLAttribute::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement attributeElement = UMLObject::save("UML:Attribute", qDoc);
    if (m_pSecondary == NULL) {
        kDebug() << "UMLAttribute::saveToXMI(" << m_Name
        << "): m_pSecondary is NULL, m_SecondaryId is '"
        << m_SecondaryId << "'" << endl;
    } else {
        attributeElement.setAttribute( "type", ID2STR(m_pSecondary->getID()) );
    }
    if (! m_InitialValue.isEmpty())
        attributeElement.setAttribute( "initialValue", m_InitialValue );
    qElement.appendChild( attributeElement );
}

bool UMLAttribute::load( QDomElement & element ) {
    m_SecondaryId = element.attribute( "type", "" );
    // We use the m_SecondaryId as a temporary store for the xmi.id
    // of the attribute type model object.
    // It is resolved later on, when all classes have been loaded.
    // This deferred resolution is required because the xmi.id may
    // be a forward reference, i.e. it may identify a model object
    // that has not yet been loaded.
    if (m_SecondaryId.isEmpty()) {
        // Perhaps the type is stored in a child node:
        QDomNode node = element.firstChild();
        while (!node.isNull()) {
            if (node.isComment()) {
                node = node.nextSibling();
                continue;
            }
            QDomElement tempElement = node.toElement();
            QString tag = tempElement.tagName();
            if (!Uml::tagEq(tag, "type")) {
                node = node.nextSibling();
                continue;
            }
            m_SecondaryId = tempElement.attribute( "xmi.id", "" );
            if (m_SecondaryId.isEmpty())
                m_SecondaryId = tempElement.attribute( "xmi.idref", "" );
            if (m_SecondaryId.isEmpty()) {
                QDomNode inner = node.firstChild();
                QDomElement tmpElem = inner.toElement();
                m_SecondaryId = tmpElem.attribute( "xmi.id", "" );
                if (m_SecondaryId.isEmpty())
                    m_SecondaryId = tmpElem.attribute( "xmi.idref", "" );
            }
            break;
        }
        if (m_SecondaryId.isEmpty()) {
            kDebug() << "UMLAttribute::load(" << m_Name << "): "
            << "cannot find type." << endl;
        }
    }
    m_InitialValue = element.attribute( "initialValue", "" );
    if (m_InitialValue.isEmpty()) {
        // for backward compatibility
        m_InitialValue = element.attribute( "value", "" );
    }
    return true;
}

bool UMLAttribute::showPropertiesDialog(QWidget* parent) {
    UMLAttributeDialog dialog(parent, this);
    return dialog.exec();
}


void UMLAttribute::setTemplateParams(const QString& templateParam, UMLClassifierList &templateParamList) {
    if (templateParam.isEmpty())
        return;
    QString type = templateParam.simplifyWhiteSpace();

    int start = type.find(QChar('<'));
    if (start >= 0 ) {
        int end = start;
        int count = 1;
        int len = type.length();
        while (count != 0 && ++end < len) {
            QChar c = type.at(end);
            if (c == QChar('<')) {
                count++;
            }
            if (c == QChar('>')) {
                count--;
            }
        }
        if (count != 0) {
            //The template is ill-formated, let's quit
            return;
        }
        setTemplateParams(type.mid(start + 1, end - start - 1), templateParamList);
        setTemplateParams(type.left(start) + type.right(len - end - 1), templateParamList);
    } else {
        QStringList paramsList = QStringList::split(QChar(','), type);
        for ( QStringList::Iterator it = paramsList.begin(); it != paramsList.end(); ++it ) {
            QString param = *it;
            if (!param.isEmpty()) {
                UMLDoc *pDoc = UMLApp::app()->getDocument();
                UMLObject* obj = pDoc->findUMLObject(param);
                if (obj == NULL ) {
                    obj = pDoc->findUMLObject(param.remove(QChar(' ')));
                }
                if (obj != NULL ) {
                    //We want to list only the params that already exist in this document
                    //If the param doesnt't already exist, we couldn't draw an association anyway
                    UMLClassifier* tmpClassifier = static_cast<UMLClassifier*>(obj);
                    if (templateParamList.findRef(tmpClassifier) == -1) {
                        templateParamList.append(tmpClassifier);
                    }
                }
            }
        }
    }
}


UMLClassifierList UMLAttribute::getTemplateParams() {
    UMLClassifierList templateParamList;
    QString type = getType()->getName();
    QString templateParam;
    // Handle C++/D/Java template/generic parameters
    const Uml::Programming_Language pl = UMLApp::app()->getActiveLanguage();
    if (pl == Uml::pl_Cpp || pl == Uml::pl_Java || pl == Uml::pl_D) {
        int start = type.find(QChar('<'));
        if (start >= 0 ) {
            int end = type.findRev(QChar('>'));
            if (end > start) {
                templateParam = type.mid(start + 1, end - start - 1);
                setTemplateParams(templateParam, templateParamList);
            }
        }
    }
    return templateParamList;
}


#include "attribute.moc"
