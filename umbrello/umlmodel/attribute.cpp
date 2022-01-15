/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "attribute.h"

// app includes
#include "debug_utils.h"
#include "classifier.h"
#include "operation.h"
#include "umlobject.h"
#include "umldoc.h"
#include "uml.h"
#include "folder.h"
#include "umlattributedialog.h"
#include "object_factory.h"
#include "optionstate.h"

// qt includes
#include <QApplication>

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
UMLAttribute::UMLAttribute(UMLObject *parent,
                           const QString& name, Uml::ID::Type id,
                           Uml::Visibility::Enum s,
                           UMLObject *type, const QString& iv)
  : UMLClassifierListItem(parent, name, id)
{
    m_InitialValue = iv;
    m_BaseType = UMLObject::ot_Attribute;
    m_visibility = s;
    m_ParmKind = Uml::ParameterDirection::In;
    /* CHECK: Do we need this:
    if (type == 0) {
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
    m_visibility = Uml::Visibility::Private;
    m_ParmKind = Uml::ParameterDirection::In;
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
void UMLAttribute::setVisibility(Uml::Visibility::Enum s)
{
    m_visibility = s;
    emit attributeChanged();
    UMLObject::emitModified();
}

/**
 * Returns The initial value of the UMLAttribute.
 *
 * @return  The initial value of the Attribute.
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

void UMLAttribute::setParmKind (Uml::ParameterDirection::Enum pk)
{
    m_ParmKind = pk;
}

Uml::ParameterDirection::Enum UMLAttribute::getParmKind() const
{
    return m_ParmKind;
}

/**
 * Returns a string representation of the UMLAttribute.
 *
 * @param sig   If true will show the attribute type and initial value.
 * @return  Returns a string representation of the UMLAttribute.
 */
QString UMLAttribute::toString(Uml::SignatureType::Enum sig, bool withStereotype) const
{
    QString s;

    if (sig == Uml::SignatureType::ShowSig || sig == Uml::SignatureType::NoSig) {
        s = Uml::Visibility::toString(m_visibility, true) + QLatin1Char(' ');
    }

    if (sig == Uml::SignatureType::ShowSig || sig == Uml::SignatureType::SigNoVis) {
        // Determine whether the type name needs to be scoped.
        UMLObject *owningObject = umlParent();
        if (owningObject->baseType() == UMLObject::ot_Operation) {
            // The immediate parent is the UMLOperation but we want
            // the UMLClassifier:
            owningObject = owningObject->umlParent();
        }
        const UMLClassifier *ownParent = owningObject->asUMLClassifier();
        if (ownParent == 0) {
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
        // Uml::ParameterDirection::Enum to have better control over this.
        if (m_ParmKind == Uml::ParameterDirection::InOut)
            s += QLatin1String("inout ");
        else if (m_ParmKind == Uml::ParameterDirection::Out)
            s += QLatin1String("out ");
        // Construct the attribute text.
        QString string = s + name() + QLatin1String(" : ") + typeName;
        if (m_InitialValue.length() > 0)
            string += QLatin1String(" = ") + m_InitialValue;
        if (withStereotype) {
            QString st = stereotype(true);
            if (!st.isEmpty())
                string += QLatin1Char(' ') + st;
        }
        return string;
    }
    return s + name();
}

/**
 * Reimplement method from UMLObject.
 */
QString UMLAttribute::getFullyQualifiedName(const QString& separator,
                                            bool includeRoot /* = false */) const
{
    const UMLOperation *op = 0;
    UMLObject *owningObject = umlParent();
    if (owningObject->baseType() == UMLObject::ot_Operation) {
        op = owningObject->asUMLOperation();
        owningObject = owningObject->umlParent();
    }
    UMLClassifier *ownParent = owningObject->asUMLClassifier();
    if (ownParent == 0) {
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
    if(this == &rhs)
        return true;

    if(!UMLObject::operator==(rhs))
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
    UMLAttribute *target = lhs->asUMLAttribute();
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
    UMLAttribute *clone = new UMLAttribute(umlParent());
    copyInto(clone);

    return clone;
}

/**
 * Creates the <UML:Attribute> XMI element.
 */
void UMLAttribute::saveToXMI(QXmlStreamWriter& writer)
{
    if (Settings::optionState().generalState.uml2) {
        UMLObject::save1(writer, QLatin1String("Property"), QLatin1String("ownedAttribute"));
    } else {
        UMLObject::save1(writer, QLatin1String("Attribute"));
    }
    if (m_pSecondary == 0) {
        uDebug() << name() << ": m_pSecondary is 0, m_SecondaryId is '"
            << m_SecondaryId << "'";
    } else {
        writer.writeAttribute(QLatin1String("type"), Uml::ID::toString(m_pSecondary->id()));
    }
    if (! m_InitialValue.isEmpty())
        writer.writeAttribute(QLatin1String("initialValue"), m_InitialValue);
    UMLObject::save1end(writer);
}

/**
 * Loads the <UML:Attribute> XMI element.
 */
bool UMLAttribute::load1(QDomElement & element)
{
    m_SecondaryId = element.attribute(QLatin1String("type"));
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
            if (!UMLDoc::tagEq(tag, QLatin1String("type"))) {
                node = node.nextSibling();
                continue;
            }
            m_SecondaryId = Model_Utils::getXmiId(tempElement);
            if (m_SecondaryId.isEmpty())
                m_SecondaryId = tempElement.attribute(QLatin1String("xmi.idref"));
            if (!m_SecondaryId.isEmpty())
                break;
            QString href = tempElement.attribute(QLatin1String("href"));
            if (href.isEmpty()) {
                QDomNode inner = node.firstChild();
                QDomElement tmpElem = inner.toElement();
                m_SecondaryId = Model_Utils::getXmiId(tmpElem);
                if (m_SecondaryId.isEmpty())
                    m_SecondaryId = tmpElem.attribute(QLatin1String("xmi.idref"));
            } else {
                QString xmiType = tempElement.attribute(QLatin1String("xmi:type"));
                if (xmiType.contains(QLatin1String("PrimitiveType")) &&
                       href.contains(QRegExp(QLatin1String("#[A-Za-z][a-z]+$")))) {
                    // Example from OMG XMI:
                    //   <type xmi:type="uml:PrimitiveType" href="http://www.omg.org/spec/UML/20090901/UML.xmi#Boolean"/>
                    // Examples from PapyrusUML:
                    //   <type xmi:type="uml:PrimitiveType" href="pathmap://UML_LIBRARIES/UMLPrimitiveTypes.library.uml#String"/>
                    //   <type xmi:type="uml:PrimitiveType" href="pathmap://UML_LIBRARIES/JavaPrimitiveTypes.library.uml#float"/>
                    int hashpos = href.lastIndexOf(QChar(QLatin1Char('#')));
                    QString typeName = href.mid(hashpos + 1);
                    UMLFolder *dtFolder = UMLApp::app()->document()->datatypeFolder();
                    UMLObjectList dataTypes = dtFolder->containedObjects();
                    m_pSecondary = Model_Utils::findUMLObject(dataTypes,
                                                              typeName, UMLObject::ot_Datatype);
                    if (m_pSecondary) {
                        uDebug() << "UMLAttribute::load1(" << name() << ") : href type =" << typeName
                                 << ", number of datatypes =" << dataTypes.size()
                                 << ", found existing " << m_pSecondary;
                    } else {
                        UMLDoc *pDoc = UMLApp::app()->document();
                        pDoc->createDatatype(typeName);
                        /* m_pSecondary = Object_Factory::createUMLObject(UMLObject::ot_Datatype,
                                                                       typeName, dtFolder);  */
                        qApp->processEvents();
                        m_pSecondary = Model_Utils::findUMLObject(dataTypes,
                                                                  typeName, UMLObject::ot_Datatype);
                        uDebug() << "UMLAttribute::load1(" << name() << ") : href type =" << typeName
                                 << ", number of datatypes =" << dataTypes.size()
                                 << ", did not find it so created it now, " << m_pSecondary;
                    }
                } else {
                    uWarning() << "UMLAttribute::load1(" << name()
                               << ") : resolving of href is not yet implemented: " << href;
                }
            }
            break;
        }
        if (m_SecondaryId.isEmpty() && !m_pSecondary) {
            uDebug() << name() << ": cannot find type.";
        }
    }
    m_InitialValue = element.attribute(QLatin1String("initialValue"));
    if (m_InitialValue.isEmpty()) {
        // for backward compatibility
        m_InitialValue = element.attribute(QLatin1String("value"));
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

    int start = type.indexOf(QLatin1Char('<'));
    if (start >= 0) {
        int end = start;
        int count = 1;
        int len = type.length();
        while (count != 0 && ++end < len) {
            QChar c = type.at(end);
            if (c == QLatin1Char('<')) {
                count++;
            }
            if (c == QLatin1Char('>')) {
                count--;
            }
        }
        if (count != 0) {
            //The template is ill-formed, let's quit
            return;
        }
        setTemplateParams(type.mid(start + 1, end - start - 1), templateParamList);
        setTemplateParams(type.left(start) + type.right(len - end - 1), templateParamList);
    } else {
        QStringList paramsList = type.split(QLatin1Char(','));
        for (QStringList::Iterator it = paramsList.begin(); it != paramsList.end(); ++it) {
            QString param = *it;
            if (!param.isEmpty()) {
                UMLDoc *pDoc = UMLApp::app()->document();
                UMLObject* obj = pDoc->findUMLObject(param);
                if (obj == 0) {
                    obj = pDoc->findUMLObject(param.remove(QLatin1Char(' ')));
                }
                if (obj != 0) {
                    //We want to list only the params that already exist in this document
                    //If the param doesn't already exist, we couldn't draw an association anyway
                    UMLClassifier* tmpClassifier = obj->asUMLClassifier();
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
    const Uml::ProgrammingLanguage::Enum pl = UMLApp::app()->activeLanguage();
    if (pl == Uml::ProgrammingLanguage::Cpp || pl == Uml::ProgrammingLanguage::CSharp ||
        pl == Uml::ProgrammingLanguage::Java || pl == Uml::ProgrammingLanguage::D) {
        int start = type.indexOf(QLatin1Char('<'));
        if (start >= 0) {
            int end = type.lastIndexOf(QLatin1Char('>'));
            if (end > start) {
                templateParam = type.mid(start + 1, end - start - 1);
                setTemplateParams(templateParam, templateParamList);
            }
        }
    }
    return templateParamList;
}

