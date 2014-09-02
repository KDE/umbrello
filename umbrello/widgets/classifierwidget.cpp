/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "classifierwidget.h"

// app includes
#include "floatingtextwidget.h"
#include "associationwidget.h"
#include "associationline.h"
#include "classifier.h"
#include "cmds.h"
#include "debug_utils.h"
#include "listpopupmenu.h"
#include "object_factory.h"
#include "operation.h"
#include "template.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"

#define PACKAGE_MARGIN 5

// qt includes
#include <QPainter>

DEBUG_REGISTER_DISABLED(ClassifierWidget)

const int ClassifierWidget::MARGIN = 5;
const int ClassifierWidget::CIRCLE_SIZE = 30;
const int ClassifierWidget::SOCKET_INCREMENT = 10;

/**
 * Constructs a ClassifierWidget.
 *
 * @param scene   The parent of this ClassifierWidget.
 * @param c       The UMLClassifier to represent.
 */
ClassifierWidget::ClassifierWidget(UMLScene * scene, UMLClassifier *c)
  : UMLWidget(scene, WidgetBase::wt_Class, c),
    m_pAssocWidget(0), m_pInterfaceName(0)
{
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
    setVisualPropertyCmd(ShowStereotype, ops.classState.showStereoType);
    setVisualPropertyCmd(DrawAsCircle, false);

    setShowAttSigs(ops.classState.showAttSig);

    if (c && c->isInterface()) {
        m_baseType = WidgetBase::wt_Interface;
        m_visualProperties = ShowOperations | ShowVisibility | ShowStereotype;
        setShowStereotype(true);
        updateSignatureTypes();
    }
}

/**
 * Constructs a ClassifierWidget.
 *
 * @param scene   The parent of this ClassifierWidget.
 * @param c       The UMLClassifier to represent.
 */
ClassifierWidget::ClassifierWidget(UMLScene * scene, UMLPackage *o)
  : UMLWidget(scene, WidgetBase::wt_Package, o),
    m_pAssocWidget(0),
    m_pInterfaceName(0)
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
    setVisualPropertyCmd(ShowStereotype, ops.classState.showStereoType);
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
        m_pInterfaceName = 0;
    }
}

/**
 * Return the UMLClassifier which this ClassifierWidget
 * represents.
 */
UMLClassifier *ClassifierWidget::classifier() const
{
    return static_cast<UMLClassifier*>(m_umlObject);
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

    else if(property == ShowOperationSignature) {
        return (m_operationSignature == Uml::SignatureType::ShowSig
                || m_operationSignature == Uml::SignatureType::SigNoVis);
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
        // Now just update flag and use base method for actual work.
        if (enable) {
            m_visualProperties |= property;
        } else {
            m_visualProperties &= ~property;
        }
        setShowStereotype(enable);
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

        // Call setVisualProperties appropriately based on enbable.
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

    DEBUG(DBG_SRC) << "VisualProperty: " << property << " to opposite status " << oppositeStatus;
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
    UMLClassifier *classifier = this->classifier();
    if (!classifier)
        return count;
    UMLClassifierListItemList list = classifier->getFilteredList(ot);
    foreach (UMLClassifierListItem *m, list) {
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
    if (classifier()->isInterface() && visualProperty(DrawAsCircle)) {
        return calculateAsCircleSize();
    }
    else if (m_umlObject->baseType() == UMLObject::ot_Package) {
        return calculateAsPackageSize();
    }

    const QFontMetrics &fm = getFontMetrics(UMLWidget::FT_NORMAL);
    const int fontHeight = fm.lineSpacing();
    // width is the width of the longest 'word'
    int width = 0, height = 0;

    // consider stereotype
    if (m_showStereotype && !m_umlObject->stereotype().isEmpty()) {
        height += fontHeight;
        // ... width
        const QFontMetrics &bfm = UMLWidget::getFontMetrics(UMLWidget::FT_BOLD);
        const int stereoWidth = bfm.size(0, m_umlObject->stereotype(true)).width();
        if (stereoWidth > width)
            width = stereoWidth;
    }

    // consider name
    height += fontHeight;
    // ... width
    QString displayedName;
    if (visualProperty(ShowPackage))
        displayedName = m_umlObject->fullyQualifiedName();
    else
        displayedName = m_umlObject->name();
    const UMLWidget::FontType nft = (m_umlObject->isAbstract() ? FT_BOLD_ITALIC : FT_BOLD);
    const int nameWidth = UMLWidget::getFontMetrics(nft).size(0, displayedName).width();
    if (nameWidth > width)
        width = nameWidth;

    // consider attributes
    const int numAtts = displayedAttributes();
    if (numAtts == 0) {
        height += fontHeight / 2;  // no atts, so just add a bit of space
    } else {
        height += fontHeight * numAtts;
        // calculate width of the attributes
        UMLClassifierListItemList list = classifier()->getFilteredList(UMLObject::ot_Attribute);
        foreach (UMLClassifierListItem *a, list) {
            if (visualProperty(ShowPublicOnly) && a->visibility() != Uml::Visibility::Public)
                continue;
            const int attWidth = fm.size(0, a->toString(m_attributeSignature)).width();
            if (attWidth > width)
                width = attWidth;
        }
    }

    // consider operations
    const int numOps = displayedOperations();
    if (numOps == 0) {
        height += fontHeight / 2;  // no ops, so just add a bit of space
    } else {
        height += numOps * fontHeight;
        // ... width
        UMLOperationList list(classifier()->getOpList());
        foreach (UMLOperation* op,  list) {
                  if (visualProperty(ShowPublicOnly) && op->visibility() != Uml::Visibility::Public)
                continue;
            const QString displayedOp = op->toString(m_operationSignature);
            UMLWidget::FontType oft;
            oft = (op->isAbstract() ? UMLWidget::FT_ITALIC : UMLWidget::FT_NORMAL);
            const int w = UMLWidget::getFontMetrics(oft).size(0, displayedOp).width();
            if (w > width)
                width = w;
        }
    }

    if (withExtensions) {
        // consider template box _as last_ !
        QSize templatesBoxSize = calculateTemplatesBoxSize();
        if (templatesBoxSize.width() != 0) {
            // add width to largest 'word'
            width += templatesBoxSize.width() / 2;
        }
        if (templatesBoxSize.height() != 0) {
            height += templatesBoxSize.height() - MARGIN;
        }
    }

    // allow for height margin
    if (!visualProperty(ShowOperations) && !visualProperty(ShowAttributes) && !m_showStereotype) {
        height += MARGIN * 2;
    }

    // allow for width margin
    width += MARGIN * 2;

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
    int height = count * fm.lineSpacing() + (MARGIN*2);

    foreach (UMLTemplate *t, list) {
        int textWidth = fm.size(0, t->toString()).width();
        if (textWidth > width)
            width = textWidth;
    }

    width += (MARGIN*2);
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
    m_pAssocWidget = assocwidget;
    UMLAssociation *umlassoc = NULL;
    if (assocwidget)
        umlassoc = assocwidget->association();
    classifier()->setClassAssoc(umlassoc);
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
 */
void ClassifierWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    setPenFromSettings(painter);
    if (UMLWidget::useFillColor())
        painter->setBrush(UMLWidget::fillColor());
    else
        painter->setBrush(m_scene->activeView()->viewport()->palette().color(QPalette::Background));

    if (classifier()->isInterface() && visualProperty(DrawAsCircle)) {
        drawAsCircle(painter, option);
        return;
    }
    else if (classifier()->baseType() == UMLObject::ot_Package) {
        drawAsPackage(painter, option);
        return;
    }

    // Draw the bounding rectangle
    QSize templatesBoxSize = calculateTemplatesBoxSize();
    int bodyOffsetY = 0;
    if (templatesBoxSize.height() > 0)
        bodyOffsetY += templatesBoxSize.height() - MARGIN;
    int w = width();
    if (templatesBoxSize.width() > 0)
        w -= templatesBoxSize.width() / 2;
    int h = height();
    if (templatesBoxSize.height() > 0)
        h -= templatesBoxSize.height() - MARGIN;
    painter->drawRect(0, bodyOffsetY, w, h);

    QFont font = UMLWidget::font();
    font.setUnderline(false);
    font.setItalic(false);
    const QFontMetrics fm = UMLWidget::getFontMetrics(UMLWidget::FT_NORMAL);
    const int fontHeight = fm.lineSpacing();

    //If there are any templates then draw them
    UMLTemplateList tlist = classifier()->getTemplateList();
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
        const int x = width() - templatesBoxSize.width() + MARGIN;
        int y = MARGIN;
        foreach (UMLTemplate *t, tlist) {
            QString text = t->toString();
            painter->drawText(x, y, fm.size(0, text).width(), fontHeight, Qt::AlignVCenter, text);
            y += fontHeight;
        }
    }

    const int textX = MARGIN;
    const int textWidth = w - MARGIN * 2;

    painter->setPen(QPen(textColor()));

    // draw stereotype
    font.setBold(true);
    QString stereo = m_umlObject->stereotype();
    /* if no stereotype is given we don't want to show the empty << >> */
    const bool showStereotype = (m_showStereotype && !stereo.isEmpty());
    const bool showNameOnly = (!visualProperty(ShowOperations) &&
                               !visualProperty(ShowAttributes) &&
                               !visualProperty(ShowStereotype));
    int nameHeight = fontHeight;
    if (showNameOnly) {
        nameHeight = h;
    } else if (showStereotype) {
        painter->setFont(font);
        stereo = m_umlObject->stereotype(true);
        painter->drawText(textX, bodyOffsetY, textWidth, fontHeight, Qt::AlignCenter, stereo);
        bodyOffsetY += fontHeight;
    }

    // draw name
    QString name;
    if (visualProperty(ShowPackage)) {
        name = m_umlObject->fullyQualifiedName();
    } else {
        name = this->name();
    }
    font.setItalic(m_umlObject->isAbstract());
    painter->setFont(font);
    painter->drawText(textX, bodyOffsetY, textWidth, nameHeight, Qt::AlignCenter, name);
    if (!showNameOnly) {
        bodyOffsetY += fontHeight;
        setPenFromSettings(painter);
        painter->drawLine(0, bodyOffsetY, w, bodyOffsetY);
        painter->setPen(textColor());
    }
    font.setBold(false);
    font.setItalic(false);
    painter->setFont(font);

    // draw attributes
    const int numAtts = displayedAttributes();
    if (visualProperty(ShowAttributes)) {
        drawMembers(painter, UMLObject::ot_Attribute, m_attributeSignature, textX,
                    bodyOffsetY, fontHeight);
    }

    // draw dividing line between attributes and operations
    if (!showNameOnly) {
        if (numAtts == 0)
            bodyOffsetY += fontHeight / 2;  // no atts, so just add a bit of space
        else
            bodyOffsetY += fontHeight * numAtts;
        setPenFromSettings(painter);
        painter->drawLine(0, bodyOffsetY, w, bodyOffsetY);
        painter->setPen(QPen(textColor()));
    }

    // draw operations
    if (visualProperty(ShowOperations)) {
        drawMembers(painter, UMLObject::ot_Operation, m_operationSignature, textX,
                    bodyOffsetY, fontHeight);
    }

    UMLWidget::paint(painter, option, widget);
}

/**
 * @return The shape of the ClassifierWidget.
 */
QPainterPath ClassifierWidget::shape() const
{
    QPainterPath path;
    if (classifier()->isInterface() && visualProperty(DrawAsCircle)) {
        path.addEllipse(rect());
        return path;
    }
    QSizeF mainSize = calculateSize(false);
    QSize templatesBoxSize = calculateTemplatesBoxSize();
    qreal mainY = 0.0;
    if (templatesBoxSize.height() > 0) {
        mainY += templatesBoxSize.height() - MARGIN;
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
    const int w = width();

    painter->drawEllipse(w/2 - CIRCLE_SIZE/2, SOCKET_INCREMENT / 2, CIRCLE_SIZE, CIRCLE_SIZE);
    if (m_Assocs.size() > 1) {
        // Draw socket for required interface.
        const qreal angleSpan = 180;   // 360.0 / (m_Assocs.size() + 1.0);
        const int arcDiameter = CIRCLE_SIZE + SOCKET_INCREMENT;
        QRect requireArc(w/2 - arcDiameter/2, 0, arcDiameter, arcDiameter);
        const QPointF center(x() + w/2, y() + arcDiameter/2);
        const qreal cX = center.x();
        const qreal cY = center.y();
        foreach (AssociationWidget *aw, m_Assocs) {
            const Uml::AssociationType::Enum aType = aw->associationType();
            if (aType == Uml::AssociationType::UniAssociation ||
                   aType == Uml::AssociationType::Association)  // provider
                continue;
            UMLWidget *otherEnd = aw->widgetForRole(Uml::RoleType::A);
            const WidgetBase::WidgetType oType = otherEnd->baseType();
            if (oType != WidgetBase::wt_Component && oType != WidgetBase::wt_Port)
                continue;

            AssociationLine *assocLine = aw->associationLine();
            const QPointF p(assocLine->endPoint());
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
                uError() << "socket: assocLine endPoint " << p
                         << " too close to own center" << center;
            }
        }
    }

    UMLWidget::paint(painter, option);
}

/**
 * Calculates the size of the object when drawn as a circle.
 * Only applies when m_umlObject->getBaseType() is ot_Interface.
 */
QSize ClassifierWidget::calculateAsCircleSize() const
{
    int circleSize = CIRCLE_SIZE;
    if (m_Assocs.size() > 1)
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
    if (m_umlObject->stereotype() == QLatin1String("subsystem")) {
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
    if (m_umlObject != NULL) {
        QString stereotype = m_umlObject->stereotype();
        if (!stereotype.isEmpty()) {
            painter->drawText(0, fontHeight + PACKAGE_MARGIN,
                       w, fontHeight, Qt::AlignCenter, m_umlObject->stereotype(true));
            lines = 2;
        }
    }

    painter->drawText(0, (fontHeight*lines) + PACKAGE_MARGIN,
               w, fontHeight, Qt::AlignCenter, name());
}

QSize ClassifierWidget::calculateAsPackageSize() const
{
    const QFontMetrics &fm = getFontMetrics(FT_BOLD_ITALIC);
    const int fontHeight = fm.lineSpacing();

    int lines = 1;

    int width = fm.width(m_umlObject->name());

    int tempWidth = 0;
    if (!m_umlObject->stereotype().isEmpty()) {
        tempWidth = fm.width(m_umlObject->stereotype(true));
        lines = 2;
    }
    if (tempWidth > width)
        width = tempWidth;
    width += PACKAGE_MARGIN * 2;
    if (width < 70)
        width = 70;  // minumin width of 70

    int height = (lines*fontHeight) + fontHeight + (PACKAGE_MARGIN * 2);

    return QSize(width, height);
}

/**
 * Auxiliary method for draw() of child classes:
 * Draw the attributes or operations.
 *
 * @param p          QPainter to paint to.
 * @param ot         Object type to draw, either ot_Attribute or ot_Operation.
 * @param sigType    Governs details of the member display.
 * @param x          X coordinate at which to draw the texts.
 * @param y          Y coordinate at which text drawing commences.
 * @param fontHeight The font height.
 */
void ClassifierWidget::drawMembers(QPainter * painter, UMLObject::ObjectType ot, Uml::SignatureType::Enum sigType,
                                   int x, int y, int fontHeight)
{
    QFont f = UMLWidget::font();
    f.setBold(false);
    UMLClassifierListItemList list = classifier()->getFilteredList(ot);
    painter->setClipping(true);
    painter->setClipRect(rect());
    foreach (UMLClassifierListItem *obj, list) {
          if (visualProperty(ShowPublicOnly) && obj->visibility() != Uml::Visibility::Public)
            continue;
        QString text = obj->toString(sigType);
        f.setItalic(obj->isAbstract());
        f.setUnderline(obj->isStatic());
        painter->setFont(f);
        QFontMetrics fontMetrics(f);
        painter->drawText(x, y, fontMetrics.size(0, text).width(), fontHeight, Qt::AlignVCenter, text);
        f.setItalic(false);
        f.setUnderline(false);
        painter->setFont(f);
        y += fontHeight;
    }
    painter->setClipping(false);
}

/**
 * Override method from UMLWidget in order to additionally check m_pInterfaceName.
 *
 * @param p Point to be checked.
 *
 * @return 'this' if UMLWidget::onWidget(p) returns non NULL;
 *         m_pInterfaceName if m_pName is non NULL and
 *         m_pInterfaceName->onWidget(p) returns non NULL; else NULL.
 */
UMLWidget* ClassifierWidget::onWidget(const QPointF &p)
{
    if (UMLWidget::onWidget(p) != NULL)
        return this;
    if (getDrawAsCircle() && m_pInterfaceName) {
        uDebug() << "floatingtext: " << m_pInterfaceName->text();
        return m_pInterfaceName->onWidget(p);
    }
    return NULL;
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
    return NULL;
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
    m_baseType = WidgetBase::wt_Class;
    classifier()->setBaseType(UMLObject::ot_Class);
    setVisualPropertyCmd(DrawAsCircle, false);
    const Settings::OptionState& ops = m_scene->optionState();
    setVisualProperty(ShowAttributes, ops.classState.showAtts);
    setVisualProperty(ShowStereotype, ops.classState.showStereoType);

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
    m_baseType = WidgetBase::wt_Interface;
    classifier()->setBaseType(UMLObject::ot_Interface);

    setVisualProperty(ShowAttributes, false);
    setVisualProperty(ShowStereotype, true);

    updateGeometry();
    update();
}

/**
 * Changes this classifier from an "class-or-package" to a package.
 * This widget is also updated.
 */
void ClassifierWidget::changeToPackage()
{
    m_baseType = WidgetBase::wt_Package;
    classifier()->setBaseType(UMLObject::ot_Package);

    setVisualProperty(ShowAttributes, false);
    setVisualProperty(ShowStereotype, true);

    updateGeometry();
    update();
}

/**
 * Extends base method to adjust also the association of a class
 * association.
 * Executes the base method and then, if file isn't loading and the
 * classifier acts as a class association, the association position is
 * updated.
 * TODO: This is never called.
 *
 *  param x The x-coordinate.
 *  param y The y-coordinate.
 */
//void ClassifierWidget::adjustAssociations(int x, int y)
//{
//    DEBUG(DBG_SRC) << "x=" << x << " / y=" << y;
//    UMLWidget::adjustAssocs(x, y);

//    if (m_doc->loading() || m_pAssocWidget == 0) {
//        return;
//    }

//    //:TODO: the following is also called from UMLWidgetr::ajdustAssocs(...)
//    //       and then AssociationWidget::widgetMoved(...)
//    //m_pAssocWidget->computeAssocClassLine();
//}

/**
 * Loads the "classwidget" or "interfacewidget" XML element.
 */
bool ClassifierWidget::loadFromXMI(QDomElement & qElement)
{
    if (!UMLWidget::loadFromXMI(qElement)) {
        return false;
    }

    QString showatts = qElement.attribute(QLatin1String("showattributes"), QLatin1String("0"));
    QString showops = qElement.attribute(QLatin1String("showoperations"), QLatin1String("1"));
    QString showpubliconly = qElement.attribute(QLatin1String("showpubliconly"), QLatin1String("0"));
    QString showattsigs = qElement.attribute(QLatin1String("showattsigs"), QLatin1String("600"));
    QString showopsigs = qElement.attribute(QLatin1String("showopsigs"), QLatin1String("600"));
    QString showpackage = qElement.attribute(QLatin1String("showpackage"), QLatin1String("0"));
    QString showscope = qElement.attribute(QLatin1String("showscope"), QLatin1String("0"));
    QString drawascircle = qElement.attribute(QLatin1String("drawascircle"), QLatin1String("0"));

    setVisualPropertyCmd(ShowAttributes, (bool)showatts.toInt());
    setVisualPropertyCmd(ShowOperations, (bool)showops.toInt());
    setVisualPropertyCmd(ShowPublicOnly, (bool)showpubliconly.toInt());
    setVisualPropertyCmd(ShowPackage,    (bool)showpackage.toInt());
    setVisualPropertyCmd(ShowVisibility, (bool)showscope.toInt());
    setVisualPropertyCmd(DrawAsCircle,   (bool)drawascircle.toInt());

    m_attributeSignature = Uml::SignatureType::fromInt(showattsigs.toInt());
    m_operationSignature = Uml::SignatureType::fromInt(showopsigs.toInt());

    if (!getDrawAsCircle())
        return true;

    // Optional child element: floatingtext
    QDomNode node = qElement.firstChild();
    QDomElement element = node.toElement();
    if (!element.isNull()) {
        QString tag = element.tagName();
        if (tag == QLatin1String("floatingtext")) {
            if (m_pInterfaceName == NULL) {
                m_pInterfaceName = new FloatingTextWidget(m_scene,
                                                          Uml::TextRole::Floating,
                                                          name(), Uml::ID::Reserved);
                m_pInterfaceName->setParentItem(this);
            }
            if (!m_pInterfaceName->loadFromXMI(element)) {
                // Most likely cause: The FloatingTextWidget is empty.
                delete m_pInterfaceName;
                m_pInterfaceName = NULL;
            } else {
                m_pInterfaceName->activate();
                m_pInterfaceName->update();
            }
        } else {
            uError() << "unknown tag " << tag;
        }
    }

    return true;
}

/**
 * Creates the "classwidget" or "interfacewidget" XML element.
 */
void ClassifierWidget::saveToXMI(QDomDocument & qDoc, QDomElement & qElement)
{
    QDomElement conceptElement;
    UMLClassifier *umlc = classifier();
    if (umlc->isInterface())
        conceptElement = qDoc.createElement(QLatin1String("interfacewidget"));
    else
        conceptElement = qDoc.createElement(QLatin1String("classwidget"));
    UMLWidget::saveToXMI(qDoc, conceptElement);
    conceptElement.setAttribute(QLatin1String("showoperations"), visualProperty(ShowOperations));
    conceptElement.setAttribute(QLatin1String("showpubliconly"), visualProperty(ShowPublicOnly));
    conceptElement.setAttribute(QLatin1String("showopsigs"),     m_operationSignature);
    conceptElement.setAttribute(QLatin1String("showpackage"),    visualProperty(ShowPackage));
    conceptElement.setAttribute(QLatin1String("showscope"),      visualProperty(ShowVisibility));
    if (! umlc->isInterface()) {
        conceptElement.setAttribute(QLatin1String("showattributes"), visualProperty(ShowAttributes));
        conceptElement.setAttribute(QLatin1String("showattsigs"),    m_attributeSignature);
    }
    if (umlc->isInterface() || umlc->isAbstract()) {
        conceptElement.setAttribute(QLatin1String("drawascircle"), visualProperty(DrawAsCircle));
        if (visualProperty(DrawAsCircle) && m_pInterfaceName) {
            m_pInterfaceName->saveToXMI(qDoc, conceptElement);
        }
    }
    qElement.appendChild(conceptElement);
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
    switch (sel) {
    case ListPopupMenu::mt_Attribute:
    case ListPopupMenu::mt_Operation:
    case ListPopupMenu::mt_Template:
        {
            UMLObject::ObjectType ot = ListPopupMenu::convert_MT_OT(sel);
            if (Object_Factory::createChildObject(classifier(), ot)) {
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
        UMLWidget::slotMenuSelection(action);
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

