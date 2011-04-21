/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "attribute.h"

// app includes
#include "debug_utils.h"
#include "classifier.h"
#include "operation.h"
#include "umlobject.h"
#include "umldoc.h"
#include "uml.h"
#include "umlattributedialog.h"
#include "object_factory.h"

/**
 * Sets up an attribute.
 *
 * @param parent    The parent of this UMLAttribute.
 * @param name      The name of this UMLAttribute.
 * @param id        The unique id given to this UMLAttribute.
 * @param s         The visibility of the UMLAttribute.
 * @param type      The type of this UMLAttribute.
 * @param iv        The initial value of the attribute.
 */
UMLAttribute::UMLAttribute( UMLObject *parent,
                            const QString& name, Uml::IDType id,
                            Uml::Visibility s,
                            UMLObject *type, const QString& iv )
        : UMLClassifierListItem(parent, name, id)
{
    m_InitialValue = iv;
    m_BaseType = UMLObject::ot_Attribute;
    m_Vis = s;
    m_ParmKind = Uml::pd_In;
    /* CHECK: Do we need this:
    if (type == NULL) {
        type = Object_Factory::createUMLObject(Uml::ot_Datatype, "undef");
    }
     */
    m_pSecondary = type;
}

/**
 * Sets up an attribute.
 *
 * @param parent    The parent of this UMLAttribute.
 */
UMLAttribute::UMLAttribute(UMLObject *parent) : UMLClassifierListItem(parent)
{
    m_BaseType = UMLObject::ot_Attribute;
    m_Vis = Uml::Visibility::Private;
    m_ParmKind = Uml::pd_In;
}

/**
 * Destructor.
 */
UMLAttribute::~UMLAttribute()
{
}

/**
 * Reimplementation of method from UMLObject is required as
 * an extra signal, attributeChanged(), is emitted.
 */
void UMLAttribute::setName(const QString &name)
{
    m_name = name;
    emit attributeChanged();
    UMLObject::emitModified();
}

/**
 * Reimplementation of method from UMLObject is required as
 * an extra signal, attributeChanged(), is emitted.
 */
void UMLAttribute::setVisibility(Uml::Visibility s)
{
    m_Vis = s;
    emit attributeChanged();
    UMLObject::emitModified();
}

/**
 * Returns The initial value of the UMLAttribute.
 *
 * @return  The initial value of the Atrtibute.
 */
QString UMLAttribute::getInitialValue() const
{
    return m_InitialValue;
}

/**
 * Sets the initial value of the UMLAttribute.
 *
 * @param iv   The initial value of the UMLAttribute.
 */
void UMLAttribute::setInitialValue(const QString &iv) 
{
    if(m_InitialValue != iv) {
        m_InitialValue = iv;
        UMLObject::emitModified();
    }
}

void UMLAttribute::setParmKind (Uml::Parameter_Direction pk)
{
    m_ParmKind = pk;
}

Uml::Parameter_Direction UMLAttribute::getParmKind () const
{
    return m_ParmKind;
}

/**
 * Returns a string representation of the UMLAttribute.
 *
 * @param sig   If true will show the attribute type and initial value.
 * @return  Returns a string representation of the UMLAttribute.
 */
QString UMLAttribute::toString(Uml::SignatureType sig)
{
    QString s;

    if (sig == Uml::SignatureType::ShowSig || sig == Uml::SignatureType::NoSig) {
        s = m_Vis.toString(true) + ' ';
    }

    if (sig == Uml::SignatureType::ShowSig || sig == Uml::SignatureType::SigNoVis) {
        // Determine whether the type name needs to be scoped.
        UMLObject *owningObject = static_cast<UMLObject*>(parent());
        if (owningObject->baseType() == UMLObject::ot_Operation) {
            // The immediate parent() is the UMLOperation but we want
            // the UMLClassifier:
            owningObject = static_cast<UMLObject*>(owningObject->parent());
        }
        UMLClassifier *ownParent = dynamic_cast<UMLClassifier*>(owningObject);
        if (ownParent == NULL) {
            uError() << "parent " << owningObject->name()
                << " is not a UMLClassifier";
            return QString();
        }
        QString typeName;
        UMLClassifier *type = UMLClassifierListItem::getType();
        if (type) {
            UMLPackage *typeScope = type->umlPackage();
            if (typeScope != ownParent && typeScope != ownParent->umlPackage())
                typeName = type->fullyQualifiedName();
            else
                typeName = type->name();
        }
        // The default direction, "in", is not mentioned.
        // Perhaps we should include a pd_Unspecified in
        // Uml::Parameter_Direction to have better control over this.
        if (m_ParmKind == Uml::pd_InOut)
            s += "inout ";
        else if (m_ParmKind == Uml::pd_Out)
            s += "out ";
        // Construct the attribute text.
        QString string = s + name() + " : " + typeName;
        if (m_InitialValue.length() > 0)
            string += " = " + m_InitialValue;
        return string;
    }
    return s + name();
}

/**
 * Reimplement method from UMLObject.
 */
QString UMLAttribute::getFullyQualifiedName( const QString& separator,
                                            bool includeRoot /* = false */) const
{
    UMLOperation *op = NULL;
    UMLObject *owningObject = static_cast<UMLObject*>(parent());
    if (owningObject->baseType() == UMLObject::ot_Operation) {
        op = static_cast<UMLOperation*>(owningObject);
        owningObject = static_cast<UMLObject*>(owningObject->parent());
    }
    UMLClassifier *ownParent = dynamic_cast<UMLClassifier*>(owningObject);
    if (ownParent == NULL) {
        uError() << name() << ": parent " << owningObject->name()
            << " is not a UMLClassifier";
        return QString();
    }
    QString tempSeparator = separator;
    if (tempSeparator.isEmpty())
        tempSeparator = UMLApp::app()->activeLanguageScopeSeparator();
    QString fqn = ownParent->fullyQualifiedName(tempSeparator, includeRoot);
    if (op)
        fqn.append(tempSeparator + op->name());
    fqn.append(tempSeparator + name());
    return fqn;
}

/**
 * Overloaded '==' operator
 */
bool UMLAttribute::operator==(const UMLAttribute &rhs) const
{
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

/**
 * Copy the internal presentation of this object into the UMLAttribute
 * object.
 */
void UMLAttribute::copyInto(UMLObject *lhs) const
{
    UMLAttribute *target = static_cast<UMLAttribute*>(lhs);
    // call the parent first.
    UMLClassifierListItem::copyInto(target);

    // Copy all datamembers
    target->m_pSecondary = m_pSecondary;
    target->m_SecondaryId = m_SecondaryId;
    target->m_InitialValue = m_InitialValue;
    target->m_ParmKind = m_ParmKind;
}

/**
 * Make a clone of the UMLAttribute.
 */
UMLObject* UMLAttribute::clone() const
{
    //FIXME: The new attribute should be slaved to the NEW parent not the old.
    UMLAttribute *clone = new UMLAttribute( static_cast<UMLObject*>(parent()) );
    copyInto(clone);

    return clone;
}

/**
 * Creates the <UML:Attribute> XMI element.
 */
void UMLAttribute::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement attributeElement = UMLObject::save("UML:Attribute", qDoc);
    if (m_pSecondary == NULL) {
        uDebug() << name() << ": m_pSecondary is NULL, m_SecondaryId is '"
            << m_SecondaryId << "'";
    } else {
        attributeElement.setAttribute( "type", ID2STR(m_pSecondary->id()) );
    }
    if (! m_InitialValue.isEmpty())
        attributeElement.setAttribute( "initialValue", m_InitialValue );
    qElement.appendChild( attributeElement );
}

/**
 * Loads the <UML:Attribute> XMI element.
 */
bool UMLAttribute::load( QDomElement & element )
{
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
            if (!UMLDoc::tagEq(tag, "type")) {
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
            uDebug() << name() << ": " << "cannot find type.";
        }
    }
    m_InitialValue = element.attribute( "initialValue", "" );
    if (m_InitialValue.isEmpty()) {
        // for backward compatibility
        m_InitialValue = element.attribute( "value", "" );
    }
    return true;
}

/**
 * Display the properties configuration dialog for the attribute.
 */
bool UMLAttribute::showPropertiesDialog(QWidget* parent)
{
    UMLAttributeDialog dialog(parent, this);
    return dialog.exec();
}

/**
 * Puts in the param templateParamList all the template params that are in templateParam
 */
void UMLAttribute::setTemplateParams(const QString& templateParam, UMLClassifierList &templateParamList)
{
    if (templateParam.isEmpty())
        return;
    QString type = templateParam.simplified();

    int start = type.indexOf(QChar('<'));
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
        QStringList paramsList = type.split(QChar(','));
        for ( QStringList::Iterator it = paramsList.begin(); it != paramsList.end(); ++it ) {
            QString param = *it;
            if (!param.isEmpty()) {
                UMLDoc *pDoc = UMLApp::app()->document();
                UMLObject* obj = pDoc->findUMLObject(param);
                if (obj == NULL ) {
                    obj = pDoc->findUMLObject(param.remove(QChar(' ')));
                }
                if (obj != NULL ) {
                    //We want to list only the params that already exist in this document
                    //If the param doesnt't already exist, we couldn't draw an association anyway
                    UMLClassifier* tmpClassifier = static_cast<UMLClassifier*>(obj);
                    if (templateParamList.indexOf(tmpClassifier) == -1) {
                        templateParamList.append(tmpClassifier);
                    }
                }
            }
        }
    }
}

/**
 * Returns all the template params (if any) that are in the type of this attribute
 */
UMLClassifierList UMLAttribute::getTemplateParams()
{
    UMLClassifierList templateParamList;
    QString type = getType()->name();
    QString templateParam;
    // Handle C++/D/Java template/generic parameters
    const Uml::ProgrammingLanguage pl = UMLApp::app()->activeLanguage();
    if (pl == Uml::ProgrammingLanguage::Cpp  ||
        pl == Uml::ProgrammingLanguage::Java || pl == Uml::ProgrammingLanguage::D) {
        int start = type.indexOf(QChar('<'));
        if (start >= 0 ) {
            int end = type.lastIndexOf(QChar('>'));
            if (end > start) {
                templateParam = type.mid(start + 1, end - start - 1);
                setTemplateParams(templateParam, templateParamList);
            }
        }
    }
    return templateParamList;
}

#include "attribute.moc"
