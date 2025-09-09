/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umloperation.h"

// app includes
#include "umlattribute.h"
#include "umlclassifier.h"
#include "model_utils.h"
#include "debug_utils.h"
#include "umlapp.h"
#include "umldoc.h"
#include "uniqueid.h"
#include "umloperationdialog.h"
#include "codegenerator.h"
#include "codedocument.h"
#include "codeblock.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QRegularExpression>

DEBUG_REGISTER(UMLOperation)

/**
 * Constructs a UMLOperation.
 * Not intended for general use: The operation is not tied in with
 * umbrello's Qt signalling for object creation.
 * If you want to create an Operation use the method in UMLDoc instead.
 *
 * @param parent    the parent to this operation
 * @param name      the name of the operation
 * @param id        the id of the operation
 * @param s         the visibility of the operation
 * @param rt        the return type of the operation
 */
UMLOperation::UMLOperation(UMLClassifier *parent, const QString& name,
                           Uml::ID::Type id, Uml::Visibility::Enum s, UMLObject *rt)
  : UMLClassifierListItem(parent, name, id),
    m_bVirtual(false),
    m_bInline(false)
{
    if (rt)
        m_returnId = UniqueID::gen();
    else
        m_returnId = Uml::ID::None;
    m_pSecondary = rt;
    m_visibility = s;
    m_BaseType = UMLObject::ot_Operation;
    m_bConst = false;
    m_bOverride = false;
    m_bFinal = false;
    m_Code.clear();
}

/**
 * Constructs a UMLOperation.
 * Not intended for general use: The operation is not tied in with
 * umbrello's Qt signalling for object creation.
 * If you want to create an Operation use the method in UMLDoc instead.
 *
 * @param parent    the parent to this operation
 */
UMLOperation::UMLOperation(UMLClassifier * parent)
  : UMLClassifierListItem (parent),
    m_bVirtual(false),
    m_bInline(false)
{
    m_BaseType = UMLObject::ot_Operation;
    m_bConst = false;
    m_bOverride = false;
    m_bFinal = false;
    m_Code.clear();
}

/**
 * Destructor.
 */
UMLOperation::~UMLOperation()
{
}

/**
 * Reimplement method from UMLClassifierListItem.
 *
 * @param type      pointer to the type object
 */
void UMLOperation::setType(UMLObject* type)
{
    UMLClassifierListItem::setType(type);
    if (m_returnId == Uml::ID::None)
        m_returnId = UniqueID::gen();
}

/**
 * Move a parameter one position to the left.
 *
 * @param a   the parameter to move
 */
void UMLOperation::moveParameterLeft(UMLAttribute * a)
{
    if (a == nullptr) {
        logDebug0("UMLOperation::moveParmLeft called on NULL attribute");
        return;
    }
    logDebug1("UMLOperation::moveParmLeft called for %1", a->name());
    disconnect(a, SIGNAL(modified()), this, SIGNAL(modified()));
    int idx;
    if ((idx = m_args.indexOf(a)) == -1) {
        logDebug1("UMLOperation::moveParmLeftError move parm left %1", a->name());
        return;
    }
    if (idx == 0)
        return;
    m_args.removeAll(a);
    m_args.insert(idx-1, a);
}

/**
 * Move a parameter one position to the right.
 *
 * @param a   the parameter to move
 */
void UMLOperation::moveParameterRight(UMLAttribute * a)
{
    if (a == nullptr) {
        logDebug0("UMLOperation::moveParmRight called on NULL attribute");
        return;
    }
    logDebug1("UMLOperation::moveParmRight called for %1", a->name());
    disconnect(a, SIGNAL(modified()), this, SIGNAL(modified()));
    int idx;
    if ((idx = m_args.indexOf(a)) == -1) {
        logDebug1("UMLOperation::moveParmRight: Error move parm right %1", a->name());
        return;
    }
    int count = m_args.count();
    if (idx == count-1)
        return;
    m_args.removeAll(a);
    m_args.insert(idx+1, a);
}

/**
 * Remove a parameter from the operation.
 *
 * @param a         the parameter to remove
 * @param emitModifiedSignal  whether to emit the "modified" signal
 *                  which creates an entry in the Undo stack for the
 *                  removal, default: true
 */
void UMLOperation::removeParameter(UMLAttribute * a, bool emitModifiedSignal /* =true */)
{
    if (a == nullptr) {
        logDebug0("UMLOperation::removeParm called on NULL attribute");
        return;
    }
    logDebug1("UMLOperation::removeParm called for %1", a->name());
    disconnect(a, SIGNAL(modified()), this, SIGNAL(modified()));
    if (!m_args.removeAll(a))
        logDebug1("UMLOperation::removeParm: Error removing parm %1", a->name());

    if (emitModifiedSignal)
        Q_EMIT modified();
}

/**
 * Returns a list of parameters.
 *
 * @return a list of the parameters in the operation
 */
UMLAttributeList UMLOperation::getParameterList() const
{
    return m_args;
}

/**
 * Finds a parameter of the operation.
 *
 * @param name      the parameter name to search for
 * @return          the found parameter, 0 if not found
 */
UMLAttribute* UMLOperation::findParameter(const QString &name) const
{
    for(UMLAttribute *obj: m_args) {
        if (obj->name() == name)
            return obj;
    }
    return (UMLAttribute*)nullptr;
}

/**
 * Returns a string representation of the operation.
 *
 * @param sig       what type of operation string to show
 * @param withStereotype  if true will show a possible stereotype applied to the operation
 * @return          the string representation of the operation
 */
QString UMLOperation::toString(Uml::SignatureType::Enum sig, bool withStereotype) const
{
    QString s;

    if (sig == Uml::SignatureType::ShowSig || sig == Uml::SignatureType::NoSig)
          s = Uml::Visibility::toString(m_visibility, true) + QLatin1Char(' ');

    s += name();
    Uml::ProgrammingLanguage::Enum pl = UMLApp::app()->activeLanguage();
    bool parameterlessOpNeedsParentheses =
        (pl != Uml::ProgrammingLanguage::Pascal && pl != Uml::ProgrammingLanguage::Ada);

    if (sig == Uml::SignatureType::NoSig || sig == Uml::SignatureType::NoSigNoVis) {
        if (parameterlessOpNeedsParentheses)
            s.append(QStringLiteral("()"));
        if (withStereotype) {
            QString st = stereotype(true);
            if (!st.isEmpty())
                s += QLatin1Char(' ') + st;
        }
        return s;
    }
    int last = m_args.count();
    if (last) {
        s.append(QStringLiteral("("));
        int i = 0;
        for(UMLAttribute *param : m_args) {
            i++;
            s.append(param->toString(Uml::SignatureType::SigNoVis, withStereotype));
            if (i < last)
                s.append(QStringLiteral(", "));
        }
        s.append(QStringLiteral(")"));
    } else if (parameterlessOpNeedsParentheses) {
        s.append(QStringLiteral("()"));
    }
    const UMLClassifier *ownParent = umlParent()->asUMLClassifier();
    QString returnType;
    UMLClassifier *retType = UMLClassifierListItem::getType();
    if (retType) {
        UMLPackage *retVisibility = retType->umlPackage();
        if (retVisibility != ownParent && retVisibility != ownParent->umlPackage())
            returnType = retType->fullyQualifiedName();
        else
            returnType = retType->name();
    }
    if (returnType.length() > 0 && returnType != QStringLiteral("void")) {
        s.append(QStringLiteral(" : "));

        if (returnType.startsWith(QStringLiteral("virtual "))) {
            s += returnType.mid(8);
        } else {
            s += returnType;
        }
    }
    if (withStereotype) {
        QString st = stereotype(true);
        if (!st.isEmpty())
            s += QLatin1Char(' ') + st;
    }
    return s;
}

/**
 * Add a parameter to the operation.
 *
 * @param parameter the parameter to add
 * @param position  the position in the parameter list.
 *                  If position = -1 the parameter will be
 *                  appended to the list.
 */
void UMLOperation::addParameter(UMLAttribute *parameter, int position)
{
    if (position >= 0 && position <= (int)m_args.count())
        m_args.insert(position, parameter);
    else
        m_args.append(parameter);
    UMLObject::emitModified();
    connect(parameter, SIGNAL(modified()), this, SIGNAL(modified()));
}

/**
 * Returns an unused parameter name for a new parameter.
 */
QString UMLOperation::getUniqueParameterName() const
{
    QString currentName = i18n("new_parameter");
    QString name = currentName;
    for (int number = 1; findParameter(name); ++number) {
        name = currentName + QLatin1Char('_') + QString::number(number);
    }
    return name;
}

/**
 * Overloaded '==' operator.
 */
bool UMLOperation::operator==(const UMLOperation & rhs) const
{
    if (this == &rhs)
        return true;

    if (!UMLObject::operator==(rhs))
        return false;

    if (getTypeName() != rhs.getTypeName())
        return false;

    if (m_args.count() != rhs.m_args.count())
        return false;

    if (!(m_args == rhs.m_args))
        return false;

    return true;
}

/**
 * Copy the internal presentation of this object into the new
 * object.
 */
void UMLOperation::copyInto(UMLObject *lhs) const
{
    UMLOperation *target = lhs->asUMLOperation();

    UMLClassifierListItem::copyInto(target);

    m_args.copyInto(&(target->m_args));
}

/**
 * Make a clone of this object.
 */
UMLObject* UMLOperation::clone() const
{
    //FIXME: The new operation should be slaved to the NEW parent not the old.
    UMLOperation *clone = new UMLOperation(umlParent()->asUMLClassifier());
    copyInto(clone);

    return clone;
}

/**
 * Calls resolveRef() on all parameters.
 * Needs to be called after all UML objects are loaded from file.
 *
 * @return  true for success
 */
bool UMLOperation::resolveRef()
{
    bool overallSuccess = UMLObject::resolveRef();
    // See remark on iteration style in UMLClassifier::resolveRef()
    for(UMLAttribute* pAtt : m_args) {
        if (! pAtt->resolveRef())
            overallSuccess = false;
    }
    return overallSuccess;
}

/**
 * Returns whether this operation is a constructor.
 *
 * @return  true if this operation is a constructor
 */
bool UMLOperation::isConstructorOperation() const
{
    // if an operation has the stereotype constructor
    // return true
    if (stereotype() == QStringLiteral("constructor"))
        return true;

    const UMLClassifier * c = umlParent()->asUMLClassifier();
    if (!c)
        return false;
    QString cName = c->name();
    QString opName = name();
    // It's a constructor operation if the operation name
    // matches that of the parent classifier.
    return (cName == opName);
}

/**
 * Returns whether this operation is a destructor.
 *
 * @return  true if this operation is a destructor
 */
bool UMLOperation::isDestructorOperation() const
{
    if (stereotype() == QStringLiteral("destructor"))
        return true;
    const UMLClassifier * c = umlParent()->asUMLClassifier();
    if (!c)
        return false;
    QString cName = c->name();
    QString opName = name();
    // Special support for C++ syntax:
    // It's a destructor operation if the operation name begins
    // with "~" followed by the name of the parent classifier.
    if (! opName.startsWith(QLatin1Char('~')))
        return false;
    opName.remove(QRegularExpression(QStringLiteral("^~\\s*")));
    return (cName == opName);
}

/**
 * Shortcut for (isConstructorOperation() || isDestructorOperation()).
 *
 * @return  true if this operation is a constructor or destructor
 */
bool UMLOperation::isLifeOperation() const
{
    return (isConstructorOperation() || isDestructorOperation());
}

/**
 * Sets whether this operation is a query (C++ "const").
 */
void UMLOperation::setConst(bool b)
{
    m_bConst = b;
}

/**
 * Returns whether this operation is a query (C++ "const").
 */
bool UMLOperation::getConst() const
{
    return m_bConst;
}

/**
 * Sets whether this operation has override flag.
 */
void UMLOperation::setOverride(bool b)
{
    m_bOverride = b;
}

/**
 * Returns whether this operation has override flag.
 */
bool UMLOperation::getOverride() const
{
    return m_bOverride;
}

/**
 * Sets whether this operation has final flag.
 */
void UMLOperation::setFinal(bool b)
{
    m_bFinal = b;
}

/**
 * Returns whether this operation has final flag.
 */
bool UMLOperation::getFinal() const
{
    return m_bFinal;
}

/**
 * Sets whether this operation is a virtual method.
 */
void UMLOperation::setVirtual(bool b)
{
    m_bVirtual = b;
}

/**
 * Returns whether this operation is a virtual method.
 */
bool UMLOperation::isVirtual() const
{
    return m_bVirtual;
}

/**
 * Sets whether this operation is inlined.
 */
void UMLOperation::setInline(bool b)
{
    m_bInline = b;
}

/**
 * Returns whether this operation is inlined.
 */
bool UMLOperation::isInline() const
{
    return m_bInline;
}

/**
 * Display the properties configuration dialog for the template.
 *
 * @param parent   the parent for the dialog
 */
bool UMLOperation::showPropertiesDialog(QWidget* parent)
{
    UMLOperationDialog dialog(parent, this);
    return dialog.exec();
}

/**
 * Sets the source code for this operation.
 *
 * @param code  the body of this operation
 */
void UMLOperation::setSourceCode(const QString& code)
{
    m_Code = code;
}

/**
 * Returns the source code for this operation.
 */
QString UMLOperation::getSourceCode() const
{
    return m_Code;
}

/**
 * Saves to the <UML:Operation> XMI element.
 */
void UMLOperation::saveToXMI(QXmlStreamWriter& writer)
{
    UMLObject::save1(writer, QStringLiteral("Operation"), QStringLiteral("ownedOperation"));
    if (m_bConst)
        writer.writeAttribute(QStringLiteral("isQuery"), QStringLiteral("true"));
    if (m_bOverride)
        writer.writeAttribute(QStringLiteral("isOverride"), QStringLiteral("true"));
    if (m_bFinal)
        writer.writeAttribute(QStringLiteral("isFinal"), QStringLiteral("true"));
    if (m_bVirtual)
        writer.writeAttribute(QStringLiteral("isVirtual"), QStringLiteral("true"));
    if (m_bInline)
        writer.writeAttribute(QStringLiteral("isInline"), QStringLiteral("true"));
    if (m_pSecondary == nullptr && m_args.isEmpty()) {
        writer.writeEndElement();  // UML:Operation
        return;
    }
    if (! Settings::optionState().generalState.uml2) {
        writer.writeStartElement(QStringLiteral("UML:BehavioralFeature.parameter"));
    }
    const QString dirAttrName = (Settings::optionState().generalState.uml2 ?
                                 QStringLiteral("direction") : QStringLiteral("kind"));
    if (m_pSecondary) {
        if (m_returnId == Uml::ID::None) {
            logDebug1("UMLOperation::saveToXMI %1: m_returnId is not set, setting it now.", name());
            m_returnId = UniqueID::gen();
        }
        if (Settings::optionState().generalState.uml2) {
            writer.writeStartElement(QStringLiteral("ownedParameter"));
            writer.writeAttribute(QStringLiteral("xmi:type"), QStringLiteral("uml:Parameter"));
            writer.writeAttribute(QStringLiteral("xmi:id"), Uml::ID::toString(m_returnId));
        } else {
            writer.writeStartElement(QStringLiteral("UML:Parameter"));
            writer.writeAttribute(QStringLiteral("xmi.id"), Uml::ID::toString(m_returnId));
        }
        writer.writeAttribute(QStringLiteral("type"), Uml::ID::toString(m_pSecondary->id()));
        writer.writeAttribute(dirAttrName, QStringLiteral("return"));
        writer.writeEndElement();
    } else {
        logDebug1("UMLOperation::saveToXMI: m_SecondaryId is %1", m_SecondaryId);
    }

    //save each attribute here, type different
    for(UMLAttribute* pAtt : m_args) {
        pAtt->UMLObject::save1(writer, QStringLiteral("Parameter"), QStringLiteral("ownedParameter"));
        UMLClassifier *attrType = pAtt->getType();
        if (attrType) {
            writer.writeAttribute(QStringLiteral("type"), Uml::ID::toString(attrType->id()));
        } else {
            writer.writeAttribute(QStringLiteral("type"), pAtt->getTypeName());
        }
        writer.writeAttribute(QStringLiteral("value"), pAtt->getInitialValue());

        Uml::ParameterDirection::Enum kind = pAtt->getParmKind();
        if (kind == Uml::ParameterDirection::Out)
            writer.writeAttribute(dirAttrName, QStringLiteral("out"));
        else if (kind == Uml::ParameterDirection::InOut)
            writer.writeAttribute(dirAttrName, QStringLiteral("inout"));
        // The default for the parameter kind is "in".

        writer.writeEndElement();
    }
    if (! Settings::optionState().generalState.uml2) {
        writer.writeEndElement();            // UML:BehavioralFeature.parameter
    }
    UMLObject::save1end(writer);  // UML:Operation
}

/**
 * Loads a <UML:Operation> XMI element.
 */
bool UMLOperation::load1(QDomElement & element)
{
    m_SecondaryId = element.attribute(QStringLiteral("type"));
    QString isQuery = element.attribute(QStringLiteral("isQuery"));
    if (!isQuery.isEmpty()) {
        // We need this extra test for isEmpty() because load() might have been
        // called again by the processing for BehavioralFeature.parameter (see below)
        m_bConst = (isQuery == QStringLiteral("true"));
    }
    QString isOverride = element.attribute(QStringLiteral("isOverride"));
    m_bOverride = (isOverride == QStringLiteral("true"));
    QString isFinal = element.attribute(QStringLiteral("isFinal"));
    m_bFinal = (isFinal == QStringLiteral("true"));
    QString isVirtual = element.attribute(QStringLiteral("isVirtual"));
    m_bVirtual = (isVirtual == QStringLiteral("true"));
    QString isInline = element.attribute(QStringLiteral("isInline"));
    m_bInline = (isInline == QStringLiteral("true"));
    QDomNode node = element.firstChild();
    if (node.isComment())
        node = node.nextSibling();
    QDomElement attElement = node.toElement();
    while (!attElement.isNull()) {
        QString tag = attElement.tagName();
        if (UMLDoc::tagEq(tag, QStringLiteral("BehavioralFeature.parameter")) ||
            UMLDoc::tagEq(tag, QStringLiteral("Element.ownedElement"))) {  // Embarcadero's Describe
            if (! load1(attElement))
                return false;
        } else if (UMLDoc::tagEq(tag, QStringLiteral("Parameter")) ||
                   UMLDoc::tagEq(tag, QStringLiteral("ownedParameter"))) {
            QString kind = attElement.attribute(QStringLiteral("kind"));
            if (kind.isEmpty()) {
                kind = attElement.attribute(QStringLiteral("direction"));  // UML2
                if (kind.isEmpty()) {
                    // Perhaps the kind is stored in a child node:
                    for (QDomNode n = attElement.firstChild(); !n.isNull(); n = n.nextSibling()) {
                        if (n.isComment())
                            continue;
                        QDomElement tempElement = n.toElement();
                        QString tag = tempElement.tagName();
                        if (!UMLDoc::tagEq(tag, QStringLiteral("kind")))
                            continue;
                        kind = tempElement.attribute(QStringLiteral("xmi.value"));
                        break;
                    }
                }
                if (kind.isEmpty()) {
                    kind = QStringLiteral("in");
                }
            }
            if (kind == QStringLiteral("return") ||
                kind == QStringLiteral("result")) {  // Embarcadero's Describe
                QString returnId = Model_Utils::getXmiId(attElement);
                if (!returnId.isEmpty())
                    m_returnId = Uml::ID::fromString(returnId);
                m_SecondaryId = attElement.attribute(QStringLiteral("type"));
                if (m_SecondaryId.isEmpty()) {
                    // Perhaps the type is stored in a child node:
                    QDomNode node = attElement.firstChild();
                    while (!node.isNull()) {
                        if (node.isComment()) {
                            node = node.nextSibling();
                            continue;
                        }
                        QDomElement tempElement = node.toElement();
                        QString tag = tempElement.tagName();
                        if (!UMLDoc::tagEq(tag, QStringLiteral("type"))) {
                            node = node.nextSibling();
                            continue;
                        }
                        m_SecondaryId = Model_Utils::getXmiId(tempElement);
                        if (m_SecondaryId.isEmpty())
                            m_SecondaryId = tempElement.attribute(QStringLiteral("xmi.idref"));
                        if (m_SecondaryId.isEmpty()) {
                            QDomNode inner = node.firstChild();
                            QDomElement tmpElem = inner.toElement();
                            m_SecondaryId = Model_Utils::getXmiId(tmpElem);
                            if (m_SecondaryId.isEmpty())
                                m_SecondaryId = tmpElem.attribute(QStringLiteral("xmi.idref"));
                        }
                        break;
                    }
                    if (m_SecondaryId.isEmpty()) {
                        logError1("UMLOperation::load1(%1) : cannot find return type", name());
                    }
                }
                // Use deferred xmi.id resolution.
                m_pSecondary = nullptr;
            } else {
                UMLAttribute * pAtt = new UMLAttribute(this);
                if(!pAtt->loadFromXMI(attElement)) {
                    delete pAtt;
                    return false;
                }
                if (kind == QStringLiteral("out"))
                    pAtt->setParmKind(Uml::ParameterDirection::Out);
                else if (kind == QStringLiteral("inout"))
                    pAtt->setParmKind(Uml::ParameterDirection::InOut);
                else
                    pAtt->setParmKind(Uml::ParameterDirection::In);
                m_args.append(pAtt);
            }
        }
        node = node.nextSibling();
        if (node.isComment())
            node = node.nextSibling();
        attElement = node.toElement();

        // loading the source code which was entered in the 'classpropdlg' dialog is not done
        // with the following code, because there is no CodeDocument.
        /*
        CodeGenerator* codegen = UMLApp::app()->getGenerator();
        if (codegen) {
            logDebug1("UMLOperation::load1: CodeDocument searching with id=%1",
                      Uml::ID::toString(UMLObject::ID());
            CodeDocument* codeDoc = codegen->findCodeDocumentByID(Uml::ID::toString(UMLObject::ID()));
            if (codeDoc) {
                logDebug0("UMLOperation::load1: CodeDocument was found");
            }
        }
        */
        // it is done in the code generators by calling CodeGenerator::loadFromXMI(...).

    }//end while
    return true;
}

