/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "classifierwidget.h"

// app includes
#include "floatingtextwidget.h"
#include "associationwidget.h"
#include "associationline.h"
#include "umlclassifier.h"
#include "cmds.h"
#include "debug_utils.h"
#include "diagram_utils.h"
#include "dialog_utils.h"
#include "umlinstance.h"
#include "listpopupmenu.h"
#include "object_factory.h"
#include "operation.h"
#include "optionstate.h"
#include "template.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "widget_utils.h"

// qt includes
#include <QPainter>
#include <QXmlStreamWriter>

DEBUG_REGISTER_DISABLED(ClassifierWidget)

const int ClassifierWidget::CIRCLE_SIZE = 30;
const int ClassifierWidget::SOCKET_INCREMENT = 10;

/**
 * Constructs a ClassifierWidget for a UMLClassifier.
 *
 * @param scene   The parent of this ClassifierWidget.
 * @param umlc    The UMLClassifier to represent.
 */
ClassifierWidget::ClassifierWidget(UMLScene * scene, UMLClassifier *umlc)
  : UMLWidget(scene, WidgetBase::wt_Class, umlc),
    m_attributeSignature(Uml::SignatureType::NoSigNoVis),
    m_operationSignature(Uml::SignatureType::NoSigNoVis),
    m_pAssocWidget(nullptr),
    m_pInterfaceName(nullptr)
{
    DiagramProxyWidget::setShowLinkedDiagram(false);
    const Settings::OptionState& ops = m_scene->optionState();
    setVisualPropertyCmd(ShowVisibility, ops.classState.showVisibility);
    setVisualPropertyCmd(ShowOperations, ops.classState.showOps);
    setVisualPropertyCmd(ShowPublicOnly, ops.classState.showPublicOnly);
    setVisualPropertyCmd(ShowPackage,    ops.classState.showPackage);
    m_attributeSignature = Uml::SignatureType::ShowSig;
    /*:TODO:
    setVisualProperty(ShowOperationSignature, ops.classState.showOpSig);
      Cannot do that because we get "pure virtual method called". Open code:
     */
    if(!ops.classState.showOpSig) {
        if (visualProperty(ShowVisibility))
            m_operationSignature = Uml::SignatureType::NoSig;
        else
            m_operationSignature = Uml::SignatureType::NoSigNoVis;

    } else if (visualProperty(ShowVisibility))
        m_operationSignature = Uml::SignatureType::ShowSig;
    else
        m_operationSignature = Uml::SignatureType::SigNoVis;

    setVisualPropertyCmd(ShowAttributes, ops.classState.showAtts);

    // Do not call setShowStereotype here, it is a virtual method
    // and setup of the vtbl_ptr has not yet been finalized.
    m_showStereotype = ops.classState.showStereoType;
    if (m_showStereotype != Uml::ShowStereoType::None)
        m_visualProperties |= ShowStereotype;

    setVisualPropertyCmd(DrawAsCircle, false);

    setShowAttSigs(ops.classState.showAttSig);

    if (umlc && umlc->isInterface()) {
        setBaseType(WidgetBase::wt_Interface);
        m_visualProperties = ShowOperations | ShowVisibility;
        setShowStereotype(Uml::ShowStereoType::Tags);
        updateSignatureTypes();
    }
}

/**
 * Constructs a ClassifierWidget for a UMLInstance.
 *
 * @param scene   The parent of this ClassifierWidget.
 * @param umli    The UMLInstance to represent.
 */
ClassifierWidget::ClassifierWidget(UMLScene * scene, UMLInstance * umli)
  : UMLWidget(scene, WidgetBase::wt_Instance, umli),
    m_pAssocWidget(nullptr),
    m_pInterfaceName(nullptr)
{
    DiagramProxyWidget::setShowLinkedDiagram(false);
    const Settings::OptionState& ops = m_scene->optionState();
    setVisualPropertyCmd(ShowVisibility, ops.classState.showVisibility);
    setVisualPropertyCmd(ShowPublicOnly, ops.classState.showPublicOnly);
    setVisualPropertyCmd(ShowPackage,    ops.classState.showPackage);
    m_attributeSignature = Uml::SignatureType::ShowSig;

    setVisualPropertyCmd(ShowAttributes, ops.classState.showAtts);

    setShowAttSigs(ops.classState.showAttSig);

    if (umli) {
        setBaseType(WidgetBase::wt_Instance);
        m_visualProperties =  ShowAttributes;
        updateSignatureTypes();
    }
}

/**
 * Constructs a ClassifierWidget for a UMLPackage.
 *
 * @param scene   The parent of this ClassifierWidget.
 * @param o       The UMLPackage to represent.
 */
ClassifierWidget::ClassifierWidget(UMLScene * scene, UMLPackage *o)
  : UMLWidget(scene, WidgetBase::wt_Package, o),
    m_pAssocWidget(nullptr),
    m_pInterfaceName(nullptr)
{
    const Settings::OptionState& ops = m_scene->optionState();
    setVisualPropertyCmd(ShowVisibility, ops.classState.showVisibility);
    setVisualPropertyCmd(ShowOperations, ops.classState.showOps);
    setVisualPropertyCmd(ShowPublicOnly, ops.classState.showPublicOnly);
    setVisualPropertyCmd(ShowPackage,    ops.classState.showPackage);
    m_attributeSignature = Uml::SignatureType::ShowSig;

    if(!ops.classState.showOpSig) {
        if (visualProperty(ShowVisibility))
            m_operationSignature = Uml::SignatureType::NoSig;
        else
            m_operationSignature = Uml::SignatureType::NoSigNoVis;

    } else if (visualProperty(ShowVisibility))
        m_operationSignature = Uml::SignatureType::ShowSig;
    else
        m_operationSignature = Uml::SignatureType::SigNoVis;

    setVisualPropertyCmd(ShowAttributes, ops.classState.showAtts);
    setShowStereotype(ops.classState.showStereoType);
    setVisualPropertyCmd(DrawAsPackage, true);

    setShowAttSigs(ops.classState.showAttSig);
}

/**
 * Destructor.
 */
ClassifierWidget::~ClassifierWidget()
{
    if (m_pAssocWidget)
        m_pAssocWidget->removeAssocClassLine();
    if (m_pInterfaceName) {
        delete m_pInterfaceName;
        m_pInterfaceName = nullptr;
    }
}

/**
 * Return the UMLClassifier which this ClassifierWidget
 * represents.
 */
UMLClassifier *ClassifierWidget::classifier() const
{
    return m_umlObject->asUMLClassifier();
}

/**
 * Reimplement method from UMLWidget.
 */
void ClassifierWidget::setShowStereotype(Uml::ShowStereoType::Enum flag)
{
    if (flag == Uml::ShowStereoType::None)
        m_visualProperties &= ~ShowStereotype;
    else
        m_visualProperties |= ShowStereotype;
    UMLWidget::setShowStereotype(flag);
}

/**
 * @return the visual properties
 */
ClassifierWidget::VisualProperties ClassifierWidget::visualProperties() const
{
    return m_visualProperties;
}

/**
 * Set an OR combination of properties stored in \a properties on this
 * widget.
 */
void ClassifierWidget::setVisualProperties(VisualProperties properties)
{
    // Don't do anything if the argument is equal to current status.
    if (quint32(m_visualProperties) == quint32(properties)) {
        return;
    }

    m_visualProperties = properties;
    updateSignatureTypes();
}

/**
 * @return The status of the property passed in.
 *
 * @note Use @ref attributeSignature() and @ref
 *       operationSignature() to get signature status.  This
 *       method only indicates whether signature is visible or not.
 */
bool ClassifierWidget::visualProperty(VisualProperty property) const
{
    if (property == ShowAttributeSignature) {
        return (m_attributeSignature == Uml::SignatureType::ShowSig
                || m_attributeSignature == Uml::SignatureType::SigNoVis);
    }

    else if (property == ShowOperationSignature) {
        return (m_operationSignature == Uml::SignatureType::ShowSig
                || m_operationSignature == Uml::SignatureType::SigNoVis);
    }

    else if (property == ShowStereotype) {
        return (m_showStereotype != Uml::ShowStereoType::None);
    }

    return m_visualProperties.testFlag(property);
}

/**
 * A convenient method to set and reset individual VisualProperty
 *
 * Undo command.
 *
 * @param property The property to be set/reset.
 * @param enable   True/false to set/reset. (default = true)
 *
 * @note This method handles ShowAttributeSignature and
 *       ShowOperationSignature specially.
 */
void ClassifierWidget::setVisualProperty(VisualProperty property, bool enable)
{
    if (visualProperty(property) != enable) {
        UMLApp::app()->executeCommand(new Uml::CmdChangeVisualProperty(this, property, enable));
    }
}

/**
 * A convenient method to set and reset individual VisualProperty
 *
 * @param property The property to be set/reset.
 * @param enable   True/false to set/reset. (default = true)
 *
 * @note This method handles ShowAttributeSignature and
 *       ShowOperationSignature specially.
 */
void ClassifierWidget::setVisualPropertyCmd(VisualProperty property, bool enable)
{
    // Handle ShowAttributeSignature and ShowOperationSignature
    // specially.

    if (property == ShowAttributeSignature) {
        if (!enable) {
            m_attributeSignature = visualProperty(ShowVisibility) ?
                Uml::SignatureType::NoSig : Uml::SignatureType::NoSigNoVis;
        } else {
            m_attributeSignature = visualProperty(ShowVisibility) ?
                Uml::SignatureType::ShowSig : Uml::SignatureType::SigNoVis;
        }
        //:TODO: updateTextItemGroups();
        updateSignatureTypes();
    }

    else if (property == ShowOperationSignature) {
        if (!enable) {
            m_operationSignature = visualProperty(ShowVisibility) ?
                Uml::SignatureType::NoSig : Uml::SignatureType::NoSigNoVis;
        } else {
            m_operationSignature = visualProperty(ShowVisibility) ?
                Uml::SignatureType::ShowSig : Uml::SignatureType::SigNoVis;
        }
        //:TODO: updateTextItemGroups();
        updateSignatureTypes();
    }
 
    else if (property == ShowStereotype) {
        setShowStereotype(enable ? Uml::ShowStereoType::Tags
                                 : Uml::ShowStereoType::None);
    }

    else if (property == DrawAsCircle) {
        // Don't do anything if the flag status is same.
        if (visualProperty(property) == enable)
            return;
        if (enable) {
            m_visualProperties |= property;
        } else {
            m_visualProperties &= ~property;
        }
        setDrawAsCircle(enable);
    }

    // Some other flag.
    else {
        // Don't do anything if the flag status is same.
        if (visualProperty(property) == enable) {
            return;
        }

        // Call setVisualProperties appropriately based on enable.
        if (enable) {
            setVisualProperties(visualProperties() | property);
        } else {
            setVisualProperties(visualProperties() & ~property);
        }
    }
}

/**
 * A convenient method to toggle individual VisualProperty of this
 * widget.
 *
 * @param property The property to be toggled.
 *
 * @note This method handles ShowAttributeSignature and
 *       ShowOperationSignature specially.
 */
void ClassifierWidget::toggleVisualProperty(VisualProperty property)
{
    bool oppositeStatus;
    if (property == ShowOperationSignature) {
        oppositeStatus = !(m_operationSignature == Uml::SignatureType::ShowSig
                           || m_operationSignature == Uml::SignatureType::SigNoVis);
    }
    else if (property == ShowAttributeSignature) {
        oppositeStatus = !(m_attributeSignature == Uml::SignatureType::ShowSig
                           || m_attributeSignature == Uml::SignatureType::SigNoVis);
    }
    else {
        oppositeStatus = !visualProperty(property);
    }

    logDebug2("ClassifierWidget::toggleVisualProperty property: %1 to opposite status: %2",
              property, oppositeStatus);
    setVisualProperty(property, oppositeStatus);
}

/**
 * Updates m_operationSignature to match m_showVisibility.
 */
void ClassifierWidget::updateSignatureTypes()
{
    //turn on scope
    if (visualProperty(ShowVisibility)) {
        if (m_operationSignature == Uml::SignatureType::NoSigNoVis) {
            m_operationSignature = Uml::SignatureType::NoSig;
        } else if (m_operationSignature == Uml::SignatureType::SigNoVis) {
            m_operationSignature = Uml::SignatureType::ShowSig;
        }
    }
    //turn off scope
    else {
        if (m_operationSignature == Uml::SignatureType::ShowSig) {
            m_operationSignature = Uml::SignatureType::SigNoVis;
        } else if (m_operationSignature == Uml::SignatureType::NoSig) {
            m_operationSignature = Uml::SignatureType::NoSigNoVis;
        }
    }
    if (visualProperty(ShowVisibility)) {
        if (m_attributeSignature == Uml::SignatureType::NoSigNoVis)
            m_attributeSignature = Uml::SignatureType::NoSig;
        else if (m_attributeSignature == Uml::SignatureType::SigNoVis)
            m_attributeSignature = Uml::SignatureType::ShowSig;
    } else {
        if (m_attributeSignature == Uml::SignatureType::ShowSig)
            m_attributeSignature = Uml::SignatureType::SigNoVis;
        else if(m_attributeSignature == Uml::SignatureType::NoSig)
            m_attributeSignature = Uml::SignatureType::NoSigNoVis;
    }
    updateGeometry();
    update();
}

/**
 * Returns whether to show attribute signatures.
 * Only applies when m_umlObject->getBaseType() is ot_Class.
 *
 * @return  Status of how attribute signatures are shown.
 */
Uml::SignatureType::Enum ClassifierWidget::attributeSignature() const
{
    return m_attributeSignature;
}

/**
 * Sets the type of signature to display for an attribute.
 * Only applies when m_umlObject->getBaseType() is ot_Class.
 *
 * @param sig   Type of signature to display for an attribute.
 */
void ClassifierWidget::setAttributeSignature(Uml::SignatureType::Enum sig)
{
    m_attributeSignature = sig;
    updateSignatureTypes();
    updateGeometry();
    update();
}

/**
 * @return The Uml::SignatureType::Enum value for the operations.
 */
Uml::SignatureType::Enum ClassifierWidget::operationSignature() const
{
    return m_operationSignature;
}

/**
 * Set the type of signature to display for an Operation
 *
 * @param sig   Type of signature to display for an operation.
 */
void ClassifierWidget::setOperationSignature(Uml::SignatureType::Enum sig)
{
    m_operationSignature = sig;
    updateSignatureTypes();
    updateGeometry();
    update();
}

/**
 * Sets whether to show attribute signature
 * Only applies when m_umlObject->getBaseType() is ot_Class.
 *
 * @param _status  True if attribute signatures shall be shown.
 */
void ClassifierWidget::setShowAttSigs(bool _status)
{
    if(!_status) {
        if (visualProperty(ShowVisibility))
            m_attributeSignature = Uml::SignatureType::NoSig;
        else
            m_attributeSignature = Uml::SignatureType::NoSigNoVis;
    }
    else if (visualProperty(ShowVisibility))
        m_attributeSignature = Uml::SignatureType::ShowSig;
    else
        m_attributeSignature = Uml::SignatureType::SigNoVis;
    if (UMLApp::app()->document()->loading())
        return;
    updateGeometry();
    update();
}

/**
 * Toggles whether to show attribute signatures.
 * Only applies when m_umlObject->getBaseType() is ot_Class.
 */
void ClassifierWidget::toggleShowAttSigs()
{
    if (m_attributeSignature == Uml::SignatureType::ShowSig ||
            m_attributeSignature == Uml::SignatureType::SigNoVis) {
        if (visualProperty(ShowVisibility)) {
            m_attributeSignature = Uml::SignatureType::NoSig;
        } else {
            m_attributeSignature = Uml::SignatureType::NoSigNoVis;
        }
    } else if (visualProperty(ShowVisibility)) {
        m_attributeSignature = Uml::SignatureType::ShowSig;
    } else {
        m_attributeSignature = Uml::SignatureType::SigNoVis;
    }
    updateGeometry();
    update();
}

/**
 * Return the number of displayed members of the given ObjectType.
 * Takes into consideration m_showPublicOnly but not other settings.
 */
int ClassifierWidget::displayedMembers(UMLObject::ObjectType ot) const
{
    int count = 0;
    UMLClassifier *umlc = this->classifier();
    if (!umlc && m_umlObject && m_umlObject->isUMLInstance() && ot == UMLObject::ot_Attribute)
        umlc = m_umlObject->asUMLInstance()->classifier();
    if (!umlc)
        return count;
    UMLClassifierListItemList list = umlc->getFilteredList(ot);
    for(UMLClassifierListItem *m : list) {
        if (!(visualProperty(ShowPublicOnly) && m->visibility() != Uml::Visibility::Public))
            count++;
    }
    return count;
}

/**
 * Overrides method from UMLWidget.
 */
QSizeF ClassifierWidget::minimumSize() const
{
    return calculateSize();
}

/**
 * Calculate content related size of widget.
 * Overrides method from UMLWidget.
 */
QSizeF ClassifierWidget::calculateSize(bool withExtensions /* = true */) const
{
    if (!m_umlObject) {
        return UMLWidget::minimumSize();
    }
    if (m_umlObject->baseType() == UMLObject::ot_Package) {
        return calculateAsPackageSize();
    }
    UMLClassifier *umlc = this->classifier();
    if (umlc) {
        if (umlc->isInterface() && visualProperty(DrawAsCircle))
            return calculateAsCircleSize();
    } else if (m_umlObject && m_umlObject->isUMLInstance()) {
       umlc = m_umlObject->asUMLInstance()->classifier();
    }

    const bool showNameOnly = !visualProperty(ShowAttributes) &&
                              !visualProperty(ShowOperations) &&
                              !visualProperty(ShowDocumentation);

    const QFontMetrics &fm = getFontMetrics(UMLWidget::FT_NORMAL);
    const int fontHeight = fm.lineSpacing();
    // width is the width of the longest 'word'
    int width = 0, height = 0;
    // consider stereotype
    if (m_showStereotype != Uml::ShowStereoType::None &&
                             m_umlObject && !m_umlObject->stereotype().isEmpty()) {
        height += fontHeight;
        QString taggedValues;
        if (m_showStereotype == Uml::ShowStereoType::Tags) {
            taggedValues = tags();
            if (!taggedValues.isEmpty())
                height += fontHeight;
        }
        // ... width
        const QFontMetrics &bfm = UMLWidget::getFontMetrics(UMLWidget::FT_BOLD);
        const int stereoWidth = bfm.size(0, m_umlObject->stereotype(true)).width();
        if (stereoWidth > width)
            width = stereoWidth;
        if (!taggedValues.isEmpty()) {
            const int tagsWidth = bfm.size(0, taggedValues).width();
            if (tagsWidth > width)
                width = tagsWidth;
        }
    } else if (showNameOnly) {
        height += defaultMargin;
    }

    // consider name
    height += fontHeight;
    // ... width
    QString name;
    UMLObject *o;
    if (m_umlObject && m_umlObject->isUMLInstance() && umlc)
        o = umlc;
    else
        o = m_umlObject;
    if (!o)
        name = m_Text;
    else if (visualProperty(ShowPackage))
        name = o->fullyQualifiedName();
    else
        name = o->name();

    QString displayedName;
    if (m_umlObject && m_umlObject->isUMLInstance())
        displayedName = m_umlObject->name() + QStringLiteral(" : ") + name;
    else
        displayedName = name;

    const UMLWidget::FontType nft = (m_umlObject && m_umlObject->isAbstract() ? FT_BOLD_ITALIC : FT_BOLD);
    const int nameWidth = UMLWidget::getFontMetrics(nft).size(0, displayedName).width();
    if (nameWidth > width)
        width = nameWidth;

#ifdef ENABLE_WIDGET_SHOW_DOC
    // consider documentation
    if (visualProperty(ShowDocumentation)) {
        if (!documentation().isEmpty()) {
            QRect brect = fm.boundingRect(QRect(0, 0, this->width()-2*defaultMargin, this->height()-height), Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, documentation());
            height += brect.height();
            if (!visualProperty(ShowOperations) && !visualProperty(ShowAttributes)) {
                if (brect.width() >= width)
                    width = brect.width();
            }
        }
        else
            height += fontHeight / 2;
    }
#endif

    // consider attributes
    if (visualProperty(ShowAttributes)) {
        const int numAtts = displayedAttributes();
        if (numAtts > 0) {
            height += fontHeight * numAtts;
            // calculate width of the attributes
            UMLClassifierListItemList list = umlc->getFilteredList(UMLObject::ot_Attribute);
            for(UMLClassifierListItem *a : list) {
                if (visualProperty(ShowPublicOnly) && a->visibility() != Uml::Visibility::Public)
                    continue;
                const int attWidth = fm.size(0, a->toString(m_attributeSignature, visualProperty(ShowStereotype))).width();
                if (attWidth > width)
                    width = attWidth;
            }
        }
        else
            height += fontHeight / 2;
    }

    // consider operations
    if (visualProperty(ShowOperations)) {
        const int numOps = displayedOperations();
        if (numOps > 0) {
            height += numOps * fontHeight;
            // ... width
            UMLOperationList list(umlc->getOpList());
            for(UMLOperation* op : list) {
                      if (visualProperty(ShowPublicOnly) && op->visibility() != Uml::Visibility::Public)
                    continue;
                const QString displayedOp = op->toString(m_operationSignature, visualProperty(ShowStereotype));
                UMLWidget::FontType oft;
                oft = (op->isAbstract() ? UMLWidget::FT_ITALIC : UMLWidget::FT_NORMAL);
                const int w = UMLWidget::getFontMetrics(oft).size(0, displayedOp).width();
                if (w > width)
                    width = w;
            }
        }
        else
            height += fontHeight / 2;
    }

    if (withExtensions) {
        // consider template box _as last_ !
        QSize templatesBoxSize = calculateTemplatesBoxSize();
        if (templatesBoxSize.width() != 0) {
            // add width to largest 'word'
            width += templatesBoxSize.width() / 2;
        }
        if (templatesBoxSize.height() != 0) {
            height += templatesBoxSize.height() - defaultMargin;
        }
    }

    // allow for height margin
    if (showNameOnly) {
        height += defaultMargin;
    }

    // allow for width margin
    width += defaultMargin * 2;

    if (DiagramProxyWidget::linkedDiagram() || DiagramProxyWidget::diagramLink() != Uml::ID::None)
        width += 2 * DiagramProxyWidget::iconRect().width();

    logDebug5("ClassifierWidget::calculateSize(%1) : "
              "rectWidth %2, rectHeight %3 ; calcWidth %4, calcHeight %5",
              name, this->width(), this->height(), width, height);
    return QSizeF(width, height);
}

/**
 * Calculcates the size of the templates box in the top left
 * if it exists, returns QSize(0, 0) if it doesn't.
 *
 * @return  QSize of the templates flap.
 */
QSize ClassifierWidget::calculateTemplatesBoxSize() const
{
    if (!classifier())
        return QSize(0, 0);
    UMLTemplateList list = classifier()->getTemplateList();
    int count = list.count();
    if (count == 0) {
        return QSize(0, 0);
    }

    QFont font = UMLWidget::font();
    font.setItalic(false);
    font.setUnderline(false);
    font.setBold(false);
    const QFontMetrics fm(font);

    int width = 0;
    int height = count * fm.lineSpacing() + (defaultMargin*2);

    for(UMLTemplate *t : list) {
        int textWidth = fm.size(0, t->toString(Uml::SignatureType::NoSig, visualProperty(ShowStereotype))).width();
        if (textWidth > width)
            width = textWidth;
    }

    width += (defaultMargin*2);
    return QSize(width, height);
}

/**
 * Return the number of displayed attributes.
 */
int ClassifierWidget::displayedAttributes() const
{
    if (!visualProperty(ShowAttributes))
        return 0;
    return displayedMembers(UMLObject::ot_Attribute);
}

/**
 * Return the number of displayed operations.
 */
int ClassifierWidget::displayedOperations() const
{
    if (!visualProperty(ShowOperations))
        return 0;
    return displayedMembers(UMLObject::ot_Operation);
}

/**
 * Set the AssociationWidget when this ClassWidget acts as
 * an association class.
 */
void ClassifierWidget::setClassAssociationWidget(AssociationWidget *assocwidget)
{
    if (!classifier()) {
        logError0("ClassifierWidget::setClassAssociationWidget: "
                  "Class association cannot be applied to non classifier");
        return;
    }
    m_pAssocWidget = assocwidget;
}

/**
 * Return the AssociationWidget when this classifier acts as
 * an association class (else return NULL.)
 */
AssociationWidget *ClassifierWidget::classAssociationWidget() const
{
    return m_pAssocWidget;
}

/**
 * Overrides standard method.
 * Auxiliary to reimplementations in the derived classes.
 * @note keep fetching attributes in sync with calculateSize()
 */
void ClassifierWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    setPenFromSettings(painter);
    if (UMLWidget::useFillColor())
        painter->setBrush(UMLWidget::fillColor());
    else {
        painter->setBrush(m_scene->backgroundColor());
    }
    if (m_umlObject->baseType() == UMLObject::ot_Package) {
        drawAsPackage(painter, option);
        UMLWidget::paint(painter, option, widget);
        return;
    }
    UMLClassifier *umlc = this->classifier();
    if (!umlc) {
        if (m_umlObject && m_umlObject->isUMLInstance()) {
            umlc = m_umlObject->asUMLInstance()->classifier();
        } else {
            logError0("ClassifierWidget::paint internal error - classifier() returns null");
            return;
        }
    }
    if (umlc && umlc->isInterface() && visualProperty(DrawAsCircle)) {
        drawAsCircle(painter, option);
        UMLWidget::paint(painter, option, widget);
        return;
    }

    // Draw the bounding rectangle
    QSize templatesBoxSize = calculateTemplatesBoxSize();
    int bodyOffsetY = 0;
    if (templatesBoxSize.height() > 0)
        bodyOffsetY += templatesBoxSize.height() - defaultMargin;
    int w = width();
    if (templatesBoxSize.width() > 0)
        w -= templatesBoxSize.width() / 2;
    int h = height();
    if (templatesBoxSize.height() > 0)
        h -= templatesBoxSize.height() - defaultMargin;
    painter->drawRect(0, bodyOffsetY, w, h);

    QFont font = UMLWidget::font();
    font.setUnderline(false);
    font.setItalic(false);
    const QFontMetrics &fm = UMLWidget::getFontMetrics(UMLWidget::FT_NORMAL);
    const int fontHeight = fm.lineSpacing();

    //If there are any templates then draw them
    UMLTemplateList tlist;
    if (umlc)
        tlist = umlc->getTemplateList();
    if (tlist.count() > 0) {
        setPenFromSettings(painter);
        QPen pen = painter->pen();
        pen.setStyle(Qt::DotLine);
        painter->setPen(pen);
        painter->drawRect(width() - templatesBoxSize.width(), 0,
                    templatesBoxSize.width(), templatesBoxSize.height());
        painter->setPen(QPen(textColor()));
        font.setBold(false);
        painter->setFont(font);
        const int x = width() - templatesBoxSize.width() + defaultMargin;
        int y = defaultMargin;
        const int templateWidth = templatesBoxSize.width() - 2 * defaultMargin;
        for(UMLTemplate *t : tlist) {
            QString text = t->toString(Uml::SignatureType::NoSig, m_showStereotype != Uml::ShowStereoType::None);
            painter->drawText(x, y, templateWidth, fontHeight, Qt::AlignVCenter, text);
            y += fontHeight;
        }
    }

    const int textX = defaultMargin;
    const int textWidth = w - defaultMargin * 2;

    painter->setPen(QPen(textColor()));

    // draw stereotype
    font.setBold(true);
    const bool showNameOnly = !visualProperty(ShowAttributes) &&
                              !visualProperty(ShowOperations) &&
                              !visualProperty(ShowDocumentation);

    int nameHeight = fontHeight;
    if (m_showStereotype != Uml::ShowStereoType::None && !m_umlObject->stereotype().isEmpty()) {
        painter->setFont(font);
        painter->drawText(textX, bodyOffsetY, textWidth, fontHeight, Qt::AlignCenter, m_umlObject->stereotype(true));
        bodyOffsetY += fontHeight;
        if (m_showStereotype == Uml::ShowStereoType::Tags) {
            QString taggedValues = tags();
            if (!taggedValues.isEmpty()) {
                painter->drawText(textX, bodyOffsetY, textWidth, fontHeight, Qt::AlignCenter, taggedValues);
                bodyOffsetY += fontHeight;
            }
        }
    } else if (showNameOnly) {
        nameHeight = h;
    }

    // draw name
    QString displayedName;
    if (!m_umlObject) {
        displayedName = m_Text;
    } else if (m_umlObject->isUMLInstance()) {
        displayedName = m_umlObject->name() + QStringLiteral(" : ");
        if (umlc) {
            if (visualProperty(ShowPackage))
                displayedName.append(umlc->fullyQualifiedName());
            else
                displayedName.append(umlc->name());
        }
    } else if (visualProperty(ShowPackage)) {
        displayedName = m_umlObject->fullyQualifiedName();
    } else {
        displayedName = m_umlObject->name();
    }

    if (baseType() == WidgetBase::wt_Object || baseType() == WidgetBase::wt_Instance)
        font.setUnderline(true);

    font.setItalic(m_umlObject->isAbstract());
    painter->setFont(font);
    painter->drawText(textX, bodyOffsetY, textWidth, nameHeight, Qt::AlignCenter, displayedName);
    bodyOffsetY += fontHeight;
    font.setBold(false);
    font.setItalic(false);
    font.setUnderline(false);
    painter->setFont(font);

#ifdef ENABLE_WIDGET_SHOW_DOC
    // draw documentation
    if (visualProperty(ShowDocumentation)) {
        setPenFromSettings(painter);
        painter->drawLine(0, bodyOffsetY, w, bodyOffsetY);
        painter->setPen(textColor());

        if (!documentation().isEmpty()) {
            QRect brect = fm.boundingRect(QRect(0, 0, w-2*defaultMargin, h-bodyOffsetY), Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, documentation());
            if (brect.width() > width() + 2*defaultMargin)
                brect.setWidth(width()-2*defaultMargin);
            brect.adjust(textX, bodyOffsetY, textX, bodyOffsetY);
            painter->drawText(brect, Qt::AlignLeft | Qt::TextWordWrap, documentation());
            bodyOffsetY += brect.height();
        }
        else
            bodyOffsetY += fontHeight / 2;
    }
#endif
    // draw attributes
    if (visualProperty(ShowAttributes)) {
        // draw dividing line between doc/name and attributes
        setPenFromSettings(painter);
        painter->drawLine(0, bodyOffsetY, w, bodyOffsetY);
        painter->setPen(textColor());

        const int numAtts = displayedAttributes();
        if (numAtts > 0) {
            drawMembers(painter, UMLObject::ot_Attribute, m_attributeSignature, textX,
                        bodyOffsetY, textWidth, fontHeight);
            bodyOffsetY += fontHeight * numAtts;
        }
        else
            bodyOffsetY += fontHeight / 2;
    }

    // draw operations
    if (visualProperty(ShowOperations)) {
        // draw dividing line between attributes and operations
        setPenFromSettings(painter);
        painter->drawLine(0, bodyOffsetY, w, bodyOffsetY);
        painter->setPen(QPen(textColor()));

        const int numOps = displayedOperations();
        if (numOps >= 0) {
            drawMembers(painter, UMLObject::ot_Operation, m_operationSignature, textX,
                        bodyOffsetY, textWidth, fontHeight);
        }
    }

    if (DiagramProxyWidget::linkedDiagram()) {
        DiagramProxyWidget::paint(painter, option, widget);
    }

    UMLWidget::paint(painter, option, widget);
}

/**
 * @return The shape of the ClassifierWidget.
 */
QPainterPath ClassifierWidget::shape() const
{
    QPainterPath path;
    if (classifier() && classifier()->isInterface() && visualProperty(DrawAsCircle)) {
        path.addEllipse(QRectF(QPointF(), calculateAsCircleSize()));
        return path;
    }
    QSizeF mainSize = rect().size();
    QSize templatesBoxSize = calculateTemplatesBoxSize();
    qreal mainY = 0.0;
    if (templatesBoxSize.height() > 0) {
        mainY += templatesBoxSize.height() - defaultMargin;
        path.addRect(QRectF(mainSize.width() - templatesBoxSize.width() / 2, 0.0,
                            templatesBoxSize.width(), templatesBoxSize.height()));
    }
    path.addRect(QRectF(0.0, mainY, mainSize.width(), mainSize.height()));
    return path;
}

/**
 * Draws the interface as a circle.
 * Only applies when m_umlObject->getBaseType() is ot_Interface.
 */
void ClassifierWidget::drawAsCircle(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    Q_UNUSED(option);

    const int w = width();
    bool showProvider = associationWidgetList().size() == 0;
    bool showRequired = false;
    AssociationWidgetList requiredAssocs;

    for(AssociationWidget *aw : associationWidgetList()) {
        const Uml::AssociationType::Enum aType = aw->associationType();
        UMLWidget *otherEnd = aw->widgetForRole(Uml::RoleType::A);
        UMLWidget *thisEnd = aw->widgetForRole(Uml::RoleType::B);
        if (aType == Uml::AssociationType::UniAssociation ||
                aType == Uml::AssociationType::Association) {
            if (otherEnd->baseType() == WidgetBase::wt_Component ||
                    otherEnd->baseType() == WidgetBase::wt_Port)  // provider
                showProvider = true;
            else if (thisEnd->baseType() == WidgetBase::wt_Component ||
                     thisEnd->baseType() == WidgetBase::wt_Port) {
                showRequired = true;
                requiredAssocs.push_back(aw);
            }
        }
    }

    if (showProvider || !showRequired)
        painter->drawEllipse(w/2 - CIRCLE_SIZE/2, SOCKET_INCREMENT / 2, CIRCLE_SIZE, CIRCLE_SIZE);

    if (showRequired) {
        // Draw socket for required interface.
        const qreal angleSpan = 180;   // 360.0 / (m_Assocs.size() + 1.0);
        const int arcDiameter = CIRCLE_SIZE + SOCKET_INCREMENT;
        QRect requireArc(w/2 - arcDiameter/2, 0, arcDiameter, arcDiameter);
        const QPointF center(x() + w/2, y() + arcDiameter/2);
        const qreal cX = center.x();
        const qreal cY = center.y();

        for(AssociationWidget *aw : requiredAssocs) {
            const AssociationLine& assocLine = aw->associationLine();
            const QPointF p(assocLine.endPoint());
            const qreal tolerance = 18.0;
            bool drawArc = true;
            qreal midAngle;
            if (p.x() < cX - tolerance) {
                if (p.y() < cY - tolerance)
                    midAngle = 135;
                else if (p.y() > cY + tolerance)
                    midAngle = 225;
                else
                    midAngle = 180;
            } else if (p.x() > cX + tolerance) {
                if (p.y() < cY - tolerance)
                    midAngle = 45;
                else if (p.y() > cY + tolerance)
                    midAngle = 315;
                else
                    midAngle = 0;
            } else {
                if (p.y() < cY - tolerance)
                    midAngle = 90;
                else if (p.y() > cY + tolerance)
                    midAngle = 270;
                else
                    drawArc = false;
            }
            if (drawArc) {
                // uDebug() << "number of assocs: " << m_Assocs.size()
                //          << ", p: " << p << ", center: " << center
                //          << ", midAngle: " << midAngle << ", angleSpan: " << angleSpan;
                painter->drawArc(requireArc, 16 * (midAngle - angleSpan/2), 16 * angleSpan);
            } else {
                logError4("ClassifierWidget::drawAsCircle socket: assocLine endPoint (%1,%2) is "
                          "too close to own center (%3,%4)", p.x(), p.y(), cX, cY);
            }
        }
    }
}

/**
 * Calculates the size of the object when drawn as a circle.
 * Only applies when m_umlObject->getBaseType() is ot_Interface.
 */
QSize ClassifierWidget::calculateAsCircleSize() const
{
    int circleSize = CIRCLE_SIZE;
    circleSize += SOCKET_INCREMENT;
    return QSize(circleSize, circleSize);
}

void ClassifierWidget::drawAsPackage(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    Q_UNUSED(option);

    int w = width();
    int h = height();
    QFont font = UMLWidget::font();
    font.setBold(true);
    //FIXME italic is true when a package is first created until you click elsewhere, not sure why
    font.setItalic(false);
    const QFontMetrics &fm = getFontMetrics(FT_BOLD);
    const int fontHeight  = fm.lineSpacing();

    painter->drawRect(0, 0, 50, fontHeight);
    if (m_umlObject->stereotype() == QStringLiteral("subsystem")) {
        const int fHalf = fontHeight / 2;
        const int symY = fHalf;
        const int symX = 38;
        painter->drawLine(symX, symY, symX, symY + fHalf - 2);          // left leg
        painter->drawLine(symX + 8, symY, symX + 8, symY + fHalf - 2);  // right leg
        painter->drawLine(symX, symY, symX + 8, symY);                  // waist
        painter->drawLine(symX + 4, symY, symX + 4, symY - fHalf + 2);  // head
    }
    painter->drawRect(0, fontHeight - 1, w, h - fontHeight);

    painter->setPen(textColor());
    painter->setFont(font);

    int lines = 1;
    QString stereotype = m_umlObject->stereotype();
    if (!stereotype.isEmpty()) {
        painter->drawText(0, fontHeight + defaultMargin,
                   w, fontHeight, Qt::AlignCenter, m_umlObject->stereotype(true));
        lines = 2;
    }

    painter->drawText(0, (fontHeight*lines) + defaultMargin,
               w, fontHeight, Qt::AlignCenter, name());
}

QSize ClassifierWidget::calculateAsPackageSize() const
{
    const QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    const int fontHeight = fm.lineSpacing();

    int lines = 1;

    int width = fm.horizontalAdvance(m_umlObject->name());

    int tempWidth = 0;
    if (!m_umlObject->stereotype().isEmpty()) {
        tempWidth = fm.horizontalAdvance(m_umlObject->stereotype(true));
        lines = 2;
    }
    if (tempWidth > width)
        width = tempWidth;
    width += defaultMargin * 2;
    if (width < 70)
        width = 70;  // minumin width of 70

    int height = (lines*fontHeight) + fontHeight + (defaultMargin * 2);

    return QSize(width, height);
}

/**
 * Auxiliary method for draw() of child classes:
 * Draw the attributes or operations.
 *
 * @param painter    QPainter to paint to.
 * @param ot         Object type to draw, either ot_Attribute or ot_Operation.
 * @param sigType    Governs details of the member display.
 * @param x          X coordinate at which to draw the texts.
 * @param y          Y coordinate at which text drawing commences.
 * @param width      The text width.
 * @param height     The text height.
 */
void ClassifierWidget::drawMembers(QPainter * painter,
                                   UMLObject::ObjectType ot,
                                   Uml::SignatureType::Enum sigType,
                                   int x, int y, int width, int height)
{
    UMLClassifier *umlc = classifier();
    const bool drawInstanceAttributes = (!umlc && m_umlObject && m_umlObject->isUMLInstance()
                                                            && ot == UMLObject::ot_Attribute);
    if (!umlc && !drawInstanceAttributes) {
        return;
    }
    QFont f = UMLWidget::font();
    f.setBold(false);
    painter->setClipping(true);
    painter->setClipRect(rect());
    UMLObjectList ialist;
    if (drawInstanceAttributes) {
        UMLInstance *umlinst = m_umlObject->asUMLInstance();
        umlc = umlinst->classifier();
        ialist = umlinst->subordinates();
    }
    UMLClassifierListItemList list = umlc->getFilteredList(ot);
    for (int i = 0; i < list.count(); i++) {
        UMLClassifierListItem *obj = list.at(i);
        uIgnoreZeroPointer(obj);
        if (visualProperty(ShowPublicOnly) && obj->visibility() != Uml::Visibility::Public)
            continue;
        QString text;
        if (drawInstanceAttributes) {
            UMLInstanceAttribute *iatt = ialist.at(i)->asUMLInstanceAttribute();
            if (!iatt) {
                logDebug1("ClassifierWidget::drawMembers(%1) : skipping non InstanceAttribute subordinate",
                          obj->name());
                continue;
            }
            /* CHECK: Do we want visibility indication on instance attributes?
            if (sigType == Uml::SignatureType::ShowSig || sigType == Uml::SignatureType::NoSig)
                text = Uml::Visibility::toString(iatt->visibility(), true) + QLatin1Char(' ');
             */
            text.append(iatt->toString());
        } else {
            text = obj->toString(sigType, visualProperty(ShowStereotype));
        }
        f.setItalic(obj->isAbstract());
        f.setUnderline(obj->isStatic());
        painter->setFont(f);
        painter->drawText(x, y, width, height, Qt::AlignVCenter, text);
        f.setItalic(false);
        f.setUnderline(false);
        painter->setFont(f);
        y += height;
    }
    painter->setClipping(false);
}

/**
 * Override method from UMLWidget in order to additionally check m_pInterfaceName.
 *
 * @param p Point to be checked.
 *
 * @return 'this' if UMLWidget::onWidget(p) returns non 0;
 *         m_pInterfaceName if m_pName is non NULL and
 *         m_pInterfaceName->onWidget(p) returns non 0; else NULL.
 */
UMLWidget* ClassifierWidget::onWidget(const QPointF &p)
{
    if (UMLWidget::onWidget(p) != nullptr)
        return this;
    if (getDrawAsCircle() && m_pInterfaceName) {
        logDebug1("ClassifierWidget::onWidget: floatingtext %1", m_pInterfaceName->text());
        return m_pInterfaceName->onWidget(p);
    }
    return nullptr;
}

/**
 * Reimplement function from UMLWidget.
 */
UMLWidget* ClassifierWidget::widgetWithID(Uml::ID::Type id)
{
    if (UMLWidget::widgetWithID(id))
        return this;
    if (getDrawAsCircle() && m_pInterfaceName && m_pInterfaceName->widgetWithID(id))
        return m_pInterfaceName;
    return nullptr;
}

void ClassifierWidget::setDocumentation(const QString &doc)
{
    WidgetBase::setDocumentation(doc);
    updateGeometry();
}

/**
 * Sets whether to draw as circle.
 * Only applies when m_umlObject->getBaseType() is ot_Interface.
 *
 * @param drawAsCircle   True if widget shall be drawn as circle.
 */
void ClassifierWidget::setDrawAsCircle(bool drawAsCircle)
{
    setVisualPropertyCmd(DrawAsCircle, drawAsCircle);
    const int circleSize = CIRCLE_SIZE + SOCKET_INCREMENT;
    if (drawAsCircle) {
        setX(x() + (width()/2 - circleSize/2));
        setY(y() + (height()/2 - circleSize/2));
        setSize(circleSize, circleSize);
        if (m_pInterfaceName) {
            m_pInterfaceName->show();
        } else {
            m_pInterfaceName = new FloatingTextWidget(m_scene, Uml::TextRole::Floating, name());
            m_pInterfaceName->setParentItem(this);
            m_pInterfaceName->setText(name());  // to get geometry update
            m_pInterfaceName->setX(circleSize/2 - m_pInterfaceName->width() / 2);
            m_pInterfaceName->setY(circleSize + SOCKET_INCREMENT);
        }
        m_resizable = false;
    } else {
        setSize(ClassifierWidget::minimumSize());
        setX(x() - (width()/2 - circleSize/2));
        setY(y() - (height()/2 - circleSize/2));
        if (m_pInterfaceName)
            m_pInterfaceName->hide();
        m_resizable = true;
    }
    setChangesShape(drawAsCircle);
    updateGeometry();
    update();
}

/**
 * Returns whether to draw as circle.
 * Only applies when m_umlObject->getBaseType() is ot_Interface.
 *
 * @return   True if widget is drawn as circle.
 */
bool ClassifierWidget::getDrawAsCircle() const
{
    return visualProperty(DrawAsCircle);
}

/**
 * Toggles whether to draw as circle.
 * Only applies when m_umlObject->getBaseType() is ot_Interface.
 */
void ClassifierWidget::toggleDrawAsCircle()
{
    toggleVisualProperty(DrawAsCircle);
    updateSignatureTypes();
    updateGeometry();
    update();
}

/**
 * Changes this classifier from an interface to a class.
 * Attributes and stereotype visibility is got from the view OptionState.
 * This widget is also updated.
 */
void ClassifierWidget::changeToClass()
{
    setBaseType(WidgetBase::wt_Class);
    m_umlObject->setBaseType(UMLObject::ot_Class);
    setVisualPropertyCmd(DrawAsCircle, false);
    const Settings::OptionState& ops = m_scene->optionState();
    setVisualProperty(ShowAttributes, ops.classState.showAtts);
    setShowStereotype(ops.classState.showStereoType);

    updateGeometry();
    update();
}

/**
 * Changes this classifier from a class to an interface.
 * Attributes are hidden and stereotype is shown.
 * This widget is also updated.
 */
void ClassifierWidget::changeToInterface()
{
    setBaseType(WidgetBase::wt_Interface);
    m_umlObject->setBaseType(UMLObject::ot_Interface);

    setVisualProperty(ShowAttributes, false);
    setShowStereotype(Settings::optionState().classState.showStereoType);

    updateGeometry();
    update();
}

/**
 * Changes this classifier from an "class-or-package" to a package.
 * This widget is also updated.
 */
void ClassifierWidget::changeToPackage()
{
    setBaseType(WidgetBase::wt_Package);
    m_umlObject->setBaseType(UMLObject::ot_Package);

    setVisualProperty(ShowAttributes, false);
    setShowStereotype(Uml::ShowStereoType::Name);

    updateGeometry();
    update();
}

/**
 * Loads the "classwidget" or "interfacewidget" XML element.
 */
bool ClassifierWidget::loadFromXMI(QDomElement & qElement)
{
    if (!UMLWidget::loadFromXMI(qElement)) {
        return false;
    }
    if (DiagramProxyWidget::linkedDiagram())
        DiagramProxyWidget::setShowLinkedDiagram(false);

    bool loadShowAttributes = true;
    if (umlObject() && (umlObject()->isUMLPackage() || umlObject()->isUMLInstance())) {
        loadShowAttributes = false;
    }
    if (loadShowAttributes) {
        QString showatts = qElement.attribute(QStringLiteral("showattributes"), QStringLiteral("0"));
        QString showops = qElement.attribute(QStringLiteral("showoperations"), QStringLiteral("1"));
        QString showpubliconly = qElement.attribute(QStringLiteral("showpubliconly"), QStringLiteral("0"));
        QString showattsigs = qElement.attribute(QStringLiteral("showattsigs"), QStringLiteral("600"));
        QString showopsigs = qElement.attribute(QStringLiteral("showopsigs"), QStringLiteral("600"));
        QString showpackage = qElement.attribute(QStringLiteral("showpackage"), QStringLiteral("0"));
        QString showscope = qElement.attribute(QStringLiteral("showscope"), QStringLiteral("0"));
        QString drawascircle = qElement.attribute(QStringLiteral("drawascircle"), QStringLiteral("0"));
        QString showstereotype = qElement.attribute(QStringLiteral("showstereotype"), QStringLiteral("1"));
        setVisualPropertyCmd(ShowAttributes, (bool)showatts.toInt());
        setVisualPropertyCmd(ShowOperations, (bool)showops.toInt());
        setVisualPropertyCmd(ShowPublicOnly, (bool)showpubliconly.toInt());
        setVisualPropertyCmd(ShowPackage,    (bool)showpackage.toInt());
        setVisualPropertyCmd(ShowVisibility, (bool)showscope.toInt());
        setVisualPropertyCmd(DrawAsCircle,   (bool)drawascircle.toInt());
        setShowStereotype((Uml::ShowStereoType::Enum)showstereotype.toInt());
        m_attributeSignature = Uml::SignatureType::fromInt(showattsigs.toInt());
        m_operationSignature = Uml::SignatureType::fromInt(showopsigs.toInt());
    }

#ifdef ENABLE_WIDGET_SHOW_DOC
    QString showDocumentation = qElement.attribute(QStringLiteral("showdocumentation"), QStringLiteral("0"));
    setVisualPropertyCmd(ShowDocumentation, (bool)showDocumentation.toInt());
#endif

    if (!getDrawAsCircle())
        return true;

    // Optional child element: floatingtext
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    if (!element.isNull()) {
        QString tag = element.tagName();
        if (tag == QStringLiteral("floatingtext")) {
            if (m_pInterfaceName == nullptr) {
                m_pInterfaceName = new FloatingTextWidget(m_scene,
                                                          Uml::TextRole::Floating,
                                                          name(), Uml::ID::Reserved);
                m_pInterfaceName->setParentItem(this);
            }
            if (!m_pInterfaceName->loadFromXMI(element)) {
                // Most likely cause: The FloatingTextWidget is empty.
                delete m_pInterfaceName;
                m_pInterfaceName = nullptr;
            } else {
                m_pInterfaceName->activate();
                m_pInterfaceName->update();
            }
        } else {
            logError1("ClassifierWidget::loadFromXMI: unknown tag %1", tag);
        }
    }

    return true;
}

/**
 * Creates the "classwidget" or "interfacewidget" XML element.
 */
void ClassifierWidget::saveToXMI(QXmlStreamWriter& writer)
{
    bool saveShowAttributes = true;
    UMLClassifier *umlc = classifier();
    QString tag;
    if (umlObject()->baseType() == UMLObject::ot_Package) {
        tag = QStringLiteral("packagewidget");
        saveShowAttributes = false;
    } else if (umlObject()->baseType() == UMLObject::ot_Instance) {
        tag = QStringLiteral("instancewidget");
        saveShowAttributes = false;
    } else if (umlc && umlc->isInterface()) {
        tag = QStringLiteral("interfacewidget");
    } else {
        tag = QStringLiteral("classwidget");
    }
    writer.writeStartElement(tag);

    UMLWidget::saveToXMI(writer);
    if (saveShowAttributes) {
        writer.writeAttribute(QStringLiteral("showoperations"), QString::number(visualProperty(ShowOperations)));
        writer.writeAttribute(QStringLiteral("showpubliconly"), QString::number(visualProperty(ShowPublicOnly)));
        writer.writeAttribute(QStringLiteral("showopsigs"),     QString::number(m_operationSignature));
        writer.writeAttribute(QStringLiteral("showpackage"),    QString::number(visualProperty(ShowPackage)));
        writer.writeAttribute(QStringLiteral("showscope"),      QString::number(visualProperty(ShowVisibility)));
        writer.writeAttribute(QStringLiteral("showattributes"), QString::number(visualProperty(ShowAttributes)));
        writer.writeAttribute(QStringLiteral("showattsigs"),    QString::number(m_attributeSignature));
    }
#ifdef ENABLE_WIDGET_SHOW_DOC
    writer.writeAttribute(QStringLiteral("showdocumentation"), QString::number(visualProperty(ShowDocumentation)));
#endif
    if (umlc && (umlc->isInterface() || umlc->isAbstract())) {
        writer.writeAttribute(QStringLiteral("drawascircle"), QString::number(visualProperty(DrawAsCircle)));
        if (visualProperty(DrawAsCircle) && m_pInterfaceName) {
            m_pInterfaceName->saveToXMI(writer);
        }
    }
    writer.writeEndElement();
}

/**
 * Will be called when a menu selection has been made from the
 * popup menu.
 *
 * @param action   The action that has been selected.
 */
void ClassifierWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = ListPopupMenu::typeFromAction(action);
    logDebug1("ClassifierWidget::slotMenuSelection sel = %1", sel);
    switch (sel) {
    case ListPopupMenu::mt_Attribute:
    case ListPopupMenu::mt_Operation:
    case ListPopupMenu::mt_Template:
        {
            UMLObject::ObjectType ot = ListPopupMenu::convert_MT_OT(sel);
            UMLClassifier *umlc = classifier();
            if (!umlc) {
                logError1("ClassifierWidget::slotMenuSelection(%1) internal error - classifier() returns null",
                          sel);
                return;
            }
            if (Object_Factory::createChildObject(umlc, ot)) {
                updateGeometry();
                update();
                UMLApp::app()->document()->setModified();
            }
            break;
        }

    case ListPopupMenu::mt_Class:
    case ListPopupMenu::mt_Datatype:
    case ListPopupMenu::mt_Enum:
    case ListPopupMenu::mt_Interface:
        {
            UMLObject::ObjectType ot = ListPopupMenu::convert_MT_OT(sel);
            UMLClassifier *umlc = classifier();
            if (!umlc) {
                logError1("ClassifierWidget::slotMenuSelection(%1) internal error - classifier() returns null",
                          sel);
                return;
            }
            umlScene()->setCreateObject(true);
            if (Object_Factory::createUMLObject(ot, QString(), umlc)) {
                updateGeometry();
                update();
                UMLApp::app()->document()->setModified();
            }
            break;
        }

    case ListPopupMenu::mt_Show_Operations:
        toggleVisualProperty(ShowOperations);
        break;

    case ListPopupMenu::mt_Show_Attributes:
        toggleVisualProperty(ShowAttributes);
        break;

    case ListPopupMenu::mt_Show_Documentation:
        toggleVisualProperty(ShowDocumentation);
        break;

    case ListPopupMenu::mt_Show_Public_Only:
        toggleVisualProperty(ShowPublicOnly);
        break;

    case ListPopupMenu::mt_Show_Operation_Signature:
        toggleVisualProperty(ShowOperationSignature);
        break;

    case ListPopupMenu::mt_Show_Attribute_Signature:
        toggleVisualProperty(ShowAttributeSignature);
        break;

    case ListPopupMenu::mt_Visibility:
        toggleVisualProperty(ShowVisibility);
        break;

    case ListPopupMenu::mt_Show_Packages:
        toggleVisualProperty(ShowPackage);
        break;

    case ListPopupMenu::mt_Show_Stereotypes:
        toggleVisualProperty(ShowStereotype);
        break;

    case ListPopupMenu::mt_DrawAsCircle:
        toggleVisualProperty(DrawAsCircle);
        break;

    case ListPopupMenu::mt_ChangeToClass:
        changeToClass();
        break;

    case ListPopupMenu::mt_ChangeToInterface:
        changeToInterface();
        break;

    case ListPopupMenu::mt_ChangeToPackage:
        changeToPackage();
        break;

    default:
        DiagramProxyWidget::slotMenuSelection(action);
        break;
    }
}

/**
 * Slot to show/hide attributes based on \a state.
 */
void ClassifierWidget::slotShowAttributes(bool state)
{
    setVisualProperty(ShowAttributes, state);
}

/**
 * Slot to show/hide operations based on \a state.
 */
void ClassifierWidget::slotShowOperations(bool state)
{
    setVisualProperty(ShowOperations, state);
}

void ClassifierWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (linkedDiagram())
        DiagramProxyWidget::mouseDoubleClickEvent(event);
    if (event->isAccepted())
        UMLWidget::mouseDoubleClickEvent(event);
}

/**
 * Show a properties dialog for a ClassifierWidget
 */
bool ClassifierWidget::showPropertiesDialog()
{
    if (UMLWidget::showPropertiesDialog()) {
        if (isInterfaceWidget() && visualProperty(DrawAsCircle))
            m_pInterfaceName->setText(name());
        return true;
    }
    return false;
}

/**
 * Overriding the method from WidgetBase because we need to do
 * something extra in case this ClassifierWidget represents
 * an interface widget used in component diagrams.
 */
void ClassifierWidget::setUMLObject(UMLObject *obj)
{
    WidgetBase::setUMLObject(obj);
    if (isInterfaceWidget() && visualProperty(DrawAsCircle))
        m_pInterfaceName->setText(obj->name());
}
