/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "classifierwidget.h"
// qt/kde includes
#include <qpainter.h>
#include <kdebug.h>
// app includes
#include "classifier.h"
#include "operation.h"
#include "template.h"
#include "associationwidget.h"
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"
#include "listpopupmenu.h"
#include "object_factory.h"
#include "umlscene.h"

ClassifierWidget::ClassifierWidget(UMLScene * scene, UMLClassifier *c)
  : UMLWidget(scene, c) {
    init();
    if (c != NULL && c->isInterface()) {
        WidgetBase::setBaseType(Uml::wt_Interface);
        m_bShowStereotype = true;
        m_bShowAttributes = false;
        updateSigs();
    }
}

ClassifierWidget::~ClassifierWidget() {
    if (m_pAssocWidget)
        m_pAssocWidget->removeAssocClassLine();
}

const qreal ClassifierWidget::MARGIN = 5;
const qreal ClassifierWidget::CIRCLE_SIZE = 30;

void ClassifierWidget::init()
{
    WidgetBase::setBaseType(Uml::wt_Class);

    const Settings::OptionState& ops = m_pScene->getOptionState();
    m_bShowAccess = ops.classState.showVisibility;
    m_bShowOperations = ops.classState.showOps;
    m_bShowPublicOnly = false;
    m_bShowPackage = ops.classState.showPackage;
    m_ShowAttSigs = Uml::st_ShowSig;
    /* setShowOpSigs( ops.classState.showOpSig );
      Cannot do that because we get "pure virtual method called". Open code:
     */
    if( !ops.classState.showOpSig ) {
        if (m_bShowAccess)
            m_ShowOpSigs = Uml::st_NoSig;
        else
            m_ShowOpSigs = Uml::st_NoSigNoVis;

    } else if (m_bShowAccess)
        m_ShowOpSigs = Uml::st_ShowSig;
    else
        m_ShowOpSigs = Uml::st_SigNoVis;

    m_bShowAttributes = ops.classState.showAtts;
    m_bShowStereotype = ops.classState.showStereoType;
    m_bDrawAsCircle = false;
    m_pAssocWidget = NULL;
    setShowAttSigs( ops.classState.showAttSig );
}

void ClassifierWidget::updateSigs() {
    //turn on scope
    if (m_bShowAccess) {
        if (m_ShowOpSigs == Uml::st_NoSigNoVis) {
            m_ShowOpSigs = Uml::st_NoSig;
        } else if (m_ShowOpSigs == Uml::st_SigNoVis) {
            m_ShowOpSigs = Uml::st_ShowSig;
        }
    } else { //turn off scope
        if (m_ShowOpSigs == Uml::st_ShowSig) {
            m_ShowOpSigs = Uml::st_SigNoVis;
        } else if (m_ShowOpSigs == Uml::st_NoSig) {
            m_ShowOpSigs = Uml::st_NoSigNoVis;
        }
    }
    if (m_bShowAccess) {
        if (m_ShowAttSigs == Uml::st_NoSigNoVis)
            m_ShowAttSigs = Uml::st_NoSig;
        else if (m_ShowAttSigs == Uml::st_SigNoVis)
            m_ShowAttSigs = Uml::st_ShowSig;
    } else {
        if (m_ShowAttSigs == Uml::st_ShowSig)
            m_ShowAttSigs = Uml::st_SigNoVis;
        else if(m_ShowAttSigs == Uml::st_NoSig)
            m_ShowAttSigs = Uml::st_NoSigNoVis;
    }
    updateComponentSize();
    update();
}

void ClassifierWidget::toggleShowStereotype()
{
    m_bShowStereotype = !m_bShowStereotype;
    updateSigs();
    updateComponentSize();
    update();
}

bool ClassifierWidget::getShowOps() const {
    return m_bShowOperations;
}

void ClassifierWidget::setShowOps(bool _show) {
    m_bShowOperations = _show;
    updateSigs();
    updateComponentSize();
    update();
}

void ClassifierWidget::toggleShowOps() {
    m_bShowOperations = !m_bShowOperations;
    updateSigs();
    updateComponentSize();
    update();
}

bool ClassifierWidget::getShowPublicOnly() const {
    return m_bShowPublicOnly;
}

void ClassifierWidget::setShowPublicOnly(bool _status) {
    m_bShowPublicOnly = _status;
    updateComponentSize();
    update();
}

void ClassifierWidget::toggleShowPublicOnly() {
    m_bShowPublicOnly = !m_bShowPublicOnly;
    updateComponentSize();
    update();
}

bool ClassifierWidget::getShowVisibility() const {
    return m_bShowAccess;
}

void ClassifierWidget::setShowVisibility(bool _visibility) {
    m_bShowAccess = _visibility;
    updateSigs();
    updateComponentSize();
    update();
}

void ClassifierWidget::toggleShowVisibility() {
    m_bShowAccess = !m_bShowAccess;
    updateSigs();
    updateComponentSize();
    update();
}

Uml::Signature_Type ClassifierWidget::getShowOpSigs() const {
    return m_ShowOpSigs;
}

void ClassifierWidget::setShowOpSigs(bool _status) {
    if( !_status ) {
        if (m_bShowAccess)
            m_ShowOpSigs = Uml::st_NoSig;
        else
            m_ShowOpSigs = Uml::st_NoSigNoVis;
    } else if (m_bShowAccess)
        m_ShowOpSigs = Uml::st_ShowSig;
    else
        m_ShowOpSigs = Uml::st_SigNoVis;
    updateComponentSize();
    update();
}

void ClassifierWidget::toggleShowOpSigs() {
    if (m_ShowOpSigs == Uml::st_ShowSig || m_ShowOpSigs == Uml::st_SigNoVis) {
        if (m_bShowAccess) {
            m_ShowOpSigs = Uml::st_NoSig;
        } else {
            m_ShowOpSigs = Uml::st_NoSigNoVis;
        }
    } else if (m_bShowAccess) {
        m_ShowOpSigs = Uml::st_ShowSig;
    } else {
        m_ShowOpSigs = Uml::st_SigNoVis;
    }
    updateComponentSize();
    update();
}

bool ClassifierWidget::getShowPackage() const {
    return m_bShowPackage;
}

void ClassifierWidget::setShowPackage(bool _status) {
    m_bShowPackage = _status;
    updateComponentSize();
    update();
}

void ClassifierWidget::toggleShowPackage() {
    m_bShowPackage = !m_bShowPackage;
    updateSigs();
    updateComponentSize();
    update();
}

void ClassifierWidget::setOpSignature(Uml::Signature_Type sig) {
    m_ShowOpSigs = sig;
    updateSigs();
    updateComponentSize();
    update();
}

void ClassifierWidget::setShowAtts(bool _show) {
    m_bShowAttributes = _show;
    updateSigs();

    updateComponentSize();
    update();
}

void ClassifierWidget::setAttSignature(Uml::Signature_Type sig) {
    m_ShowAttSigs = sig;
    updateSigs();
    updateComponentSize();
    update();
}

void ClassifierWidget::setShowAttSigs(bool _status) {
    if( !_status ) {
        if (m_bShowAccess)
            m_ShowAttSigs = Uml::st_NoSig;
        else
            m_ShowAttSigs = Uml::st_NoSigNoVis;
    }
    else if (m_bShowAccess)
        m_ShowAttSigs = Uml::st_ShowSig;
    else
        m_ShowAttSigs = Uml::st_SigNoVis;
    if (UMLApp::app()->getDocument()->loading())
        return;
    updateComponentSize();
    update();
}

void ClassifierWidget::toggleShowAtts()
{
    m_bShowAttributes = !m_bShowAttributes;
    updateSigs();
    updateComponentSize();
    update();
}

void ClassifierWidget::toggleShowAttSigs()
{
    if (m_ShowAttSigs == Uml::st_ShowSig ||
            m_ShowAttSigs == Uml::st_SigNoVis) {
        if (m_bShowAccess) {
            m_ShowAttSigs = Uml::st_NoSig;
        } else {
            m_ShowAttSigs = Uml::st_NoSigNoVis;
        }
    } else if (m_bShowAccess) {
        m_ShowAttSigs = Uml::st_ShowSig;
    } else {
        m_ShowAttSigs = Uml::st_SigNoVis;
    }
    updateComponentSize();
    update();
}

int ClassifierWidget::displayedMembers(Uml::Object_Type ot) {
    int count = 0;
    UMLClassifierListItemList list = getClassifier()->getFilteredList(ot);
    foreach (UMLClassifierListItem *m , list ) {
      if (!(m_bShowPublicOnly && m->getVisibility() != Uml::Visibility::Public))
            count++;
    }
    return count;
}

int ClassifierWidget::displayedOperations() {
    if (!m_bShowOperations)
        return 0;
    return displayedMembers(Uml::ot_Operation);
}

QSizeF ClassifierWidget::calculateSize()
{
    if (!m_pObject) {
        return UMLWidget::calculateSize();
    }
    if (getClassifier()->isInterface() && m_bDrawAsCircle) {
        return calculateAsCircleSize();
    }

    const QFontMetrics &fm = getFontMetrics(UMLWidget::FT_NORMAL);
    const qreal fontHeight = fm.lineSpacing();
    // width is the width of the longest 'word'
    qreal width = 0, height = 0;

    // consider stereotype
    if (m_bShowStereotype && !m_pObject->getStereotype().isEmpty()) {
        height += fontHeight;
        // ... width
        const QFontMetrics &bfm = UMLWidget::getFontMetrics(UMLWidget::FT_BOLD);
        const qreal stereoWidth = bfm.size(0,m_pObject->getStereotype(true)).width();
        if (stereoWidth > width)
            width = stereoWidth;
    }

    // consider name
    height += fontHeight;
    // ... width
    QString displayedName;
    if (m_bShowPackage)
        displayedName = m_pObject->getFullyQualifiedName();
    else
        displayedName = m_pObject->getName();
    const UMLWidget::FontType nft = (m_pObject->getAbstract() ? FT_BOLD_ITALIC : FT_BOLD);
    //const qreal nameWidth = getFontMetrics(nft).boundingRect(displayName).width();
    const qreal nameWidth = UMLWidget::getFontMetrics(nft).size(0,displayedName).width();
    if (nameWidth > width)
        width = nameWidth;

    // consider attributes
    const int numAtts = displayedAttributes();
    if (numAtts == 0) {
        height += fontHeight / 2;  // no atts, so just add a bit of space
    } else {
        height += fontHeight * numAtts;
        // calculate width of the attributes
        UMLClassifierListItemList list = getClassifier()->getFilteredList(Uml::ot_Attribute);
        foreach (UMLClassifierListItem *a , list ) {
            if (m_bShowPublicOnly && a->getVisibility() != Uml::Visibility::Public)
                continue;
            const qreal attWidth = fm.size(0,a->toString(m_ShowAttSigs)).width();
            if (attWidth > width)
                width = attWidth;
        }
    }

    // consider operations
    const int numOps = displayedOperations();
    if (numOps == 0) {
        height += fontHeight / 2.;  // no ops, so just add a bit of space
    } else {
        height += numOps * fontHeight;
        // ... width
        UMLOperationList list(getClassifier()->getOpList());
        foreach (UMLOperation* op ,  list) {
            if (m_bShowPublicOnly && op->getVisibility() != Uml::Visibility::Public)
                continue;
            const QString displayedOp = op->toString(m_ShowOpSigs);
            UMLWidget::FontType oft;
            oft = (op->getAbstract() ? UMLWidget::FT_ITALIC : UMLWidget::FT_NORMAL);
            const qreal w = UMLWidget::getFontMetrics(oft).size(0,displayedOp).width();
            if (w > width)
                width = w;
        }
    }

    // consider template box _as last_ !
    QSizeF templatesBoxSize = calculateTemplatesBoxSize();
    if (templatesBoxSize.width() != 0) {
        // add width to largest 'word'
        width += templatesBoxSize.width() / 2;
    }
    if (templatesBoxSize.height() != 0) {
        height += templatesBoxSize.height() - MARGIN;
    }


    // allow for height margin
    if (!m_bShowOperations && !m_bShowAttributes && !m_bShowStereotype) {
        height += MARGIN * 2;
    }

    // allow for width margin
    width += MARGIN * 2;

    return QSizeF(width, height);
}

void ClassifierWidget::slotMenuSelection(QAction* action) {
    ListPopupMenu::Menu_Type sel = m_pMenu->getMenuType(action);
    switch (sel) {
    case ListPopupMenu::mt_Attribute:
    case ListPopupMenu::mt_Operation:
    case ListPopupMenu::mt_Template:
        {
            Uml::Object_Type ot = ListPopupMenu::convert_MT_OT(sel);
            if (Object_Factory::createChildObject(getClassifier(), ot)) {
                updateComponentSize();
                update();
                UMLApp::app()->getDocument()->setModified();
            }
            break;
        }
    case ListPopupMenu::mt_Show_Operations:
    case ListPopupMenu::mt_Show_Operations_Selection:
        toggleShowOps();
        break;

    case ListPopupMenu::mt_Show_Attributes:
    case ListPopupMenu::mt_Show_Attributes_Selection:
        toggleShowAtts();
        break;

    case ListPopupMenu::mt_Show_Public_Only:
    case ListPopupMenu::mt_Show_Public_Only_Selection:
        toggleShowPublicOnly();
        break;

    case ListPopupMenu::mt_Show_Operation_Signature:
    case ListPopupMenu::mt_Show_Operation_Signature_Selection:
        toggleShowOpSigs();
        break;

    case ListPopupMenu::mt_Show_Attribute_Signature:
    case ListPopupMenu::mt_Show_Attribute_Signature_Selection:
        toggleShowAttSigs();
        break;

    case ListPopupMenu::mt_Visibility:
    case ListPopupMenu::mt_Visibility_Selection:
        toggleShowVisibility();
        break;

    case ListPopupMenu::mt_Show_Packages:
    case ListPopupMenu::mt_Show_Packages_Selection:
        toggleShowPackage();
        break;

    case ListPopupMenu::mt_Show_Stereotypes:
    case ListPopupMenu::mt_Show_Stereotypes_Selection:
        toggleShowStereotype();
        break;

    case ListPopupMenu::mt_DrawAsCircle:
    case ListPopupMenu::mt_DrawAsCircle_Selection:
        toggleDrawAsCircle();
        break;

    case ListPopupMenu::mt_ChangeToClass:
    case ListPopupMenu::mt_ChangeToClass_Selection:
        changeToClass();
        break;

    case ListPopupMenu::mt_ChangeToInterface:
    case ListPopupMenu::mt_ChangeToInterface_Selection:
        changeToInterface();
        break;

    default:
        UMLWidget::slotMenuSelection(action);
        break;
    }
}

QSizeF ClassifierWidget::calculateTemplatesBoxSize()
{
    UMLTemplateList list = getClassifier()->getTemplateList();
    int count = list.count();
    if (count == 0) {
        return QSizeF(0, 0);
    }

    qreal width, height;
    height = width = 0;

    QFont font = UMLWidget::getFont();
    font.setItalic(false);
    font.setUnderline(false);
    font.setBold(false);
    const QFontMetrics fm(font);

    height = count * fm.lineSpacing() + (MARGIN*2);

    foreach (UMLTemplate *t , list ) {
        qreal textWidth = fm.size(0, t->toString() ).width();
        if (textWidth > width)
            width = textWidth;
    }

    width += (MARGIN*2);
    return QSizeF(width, height);
}

int ClassifierWidget::displayedAttributes() {
    if (!m_bShowAttributes)
        return 0;
    return displayedMembers(Uml::ot_Attribute);
}

void ClassifierWidget::setClassAssocWidget(AssociationWidget *assocwidget) {
    m_pAssocWidget = assocwidget;
    UMLAssociation *umlassoc = NULL;
    if (assocwidget)
        umlassoc = assocwidget->getAssociation();
    getClassifier()->setClassAssoc(umlassoc);
}

AssociationWidget *ClassifierWidget::getClassAssocWidget() {
    return m_pAssocWidget;
}

UMLClassifier *ClassifierWidget::getClassifier() {
    return static_cast<UMLClassifier*>(m_pObject);
}

void ClassifierWidget::paint(QPainter *p, const QStyleOptionGraphicsItem* o, QWidget *)
{
    setPenFromSettings(*p);
    if ( UMLWidget::getUseFillColour() )
        p->setBrush( UMLWidget::getFillColour() );
    else {
        // [PORT] Derive this from style option
        // p->setBrush( m_pScene->viewport()->palette().color(QPalette::Background) );
    }
    qreal offsetX = 0, offsetY = 0;

    if (getClassifier()->isInterface() && m_bDrawAsCircle) {
        drawAsCircle(*p, offsetX, offsetY);
        return;
    }

    // Draw the bounding rectangle
    QSizeF templatesBoxSize = calculateTemplatesBoxSize();
    m_bodyOffsetY = offsetY;
    if (templatesBoxSize.height() > 0)
        m_bodyOffsetY += templatesBoxSize.height() - MARGIN;
    qreal w = getWidth();
    if (templatesBoxSize.width() > 0)
        w -= templatesBoxSize.width() / 2;
    qreal h = getHeight();
    if (templatesBoxSize.height() > 0)
        h -= templatesBoxSize.height() - MARGIN;
    p->drawRect(offsetX, m_bodyOffsetY, w, h);

    QFont font = UMLWidget::getFont();
    font.setUnderline(false);
    font.setItalic(false);
    const QFontMetrics fm = UMLWidget::getFontMetrics(UMLWidget::FT_NORMAL);
    const qreal fontHeight = fm.lineSpacing();

    //If there are any templates then draw them
    UMLTemplateList tlist = getClassifier()->getTemplateList();
    if ( tlist.count() > 0 ) {
        setPenFromSettings(*p);
        QPen pen = p->pen();
        pen.setStyle(Qt::DotLine);
        p->setPen(pen);
        p->drawRect( offsetX + getWidth() - templatesBoxSize.width(), offsetY,
                    templatesBoxSize.width(), templatesBoxSize.height() );
        p->setPen( QPen(Qt::black) );
        font.setBold(false);
        p->setFont(font);
        const qreal x = offsetX + getWidth() - templatesBoxSize.width() + MARGIN;
        qreal y = offsetY + MARGIN;
        foreach ( UMLTemplate *t , tlist ) {
            QString text = t->toString();
            p->drawText(x, y, fm.size(0,text).width(), fontHeight, Qt::AlignVCenter, text);
            y += fontHeight;
        }
    }

    const qreal textX = offsetX + MARGIN;
    const qreal textWidth = w - MARGIN * 2;

    p->setPen(QPen(Qt::black));

    // draw stereotype
    font.setBold(true);
    QString stereo = m_pObject->getStereotype();
    /* if no stereotype is given we don't want to show the empty << >> */
    const bool showStereotype = (m_bShowStereotype && !stereo.isEmpty());
    const bool showNameOnly = (!m_bShowOperations && !m_bShowAttributes && !showStereotype);
    qreal nameHeight = fontHeight;
    if (showNameOnly) {
        nameHeight = h;
    } else if (showStereotype) {
        p->setFont(font);
        stereo = m_pObject->getStereotype(true);
        p->drawText(textX, m_bodyOffsetY, textWidth, fontHeight, Qt::AlignCenter, stereo);
        m_bodyOffsetY += fontHeight;
    }

    // draw name
    QString name;
    if (m_bShowPackage) {
        name = m_pObject->getFullyQualifiedName();
    } else {
        name = this->getName();
    }
    font.setItalic( m_pObject->getAbstract() );
    p->setFont(font);
    p->drawText(textX, m_bodyOffsetY, textWidth, nameHeight, Qt::AlignCenter, name);
    if (!showNameOnly) {
        m_bodyOffsetY += fontHeight;
        setPenFromSettings(*p);
        p->drawLine(offsetX, m_bodyOffsetY, offsetX + w - 1, m_bodyOffsetY);
        p->setPen(QPen(Qt::black));
    }
    font.setBold(false);
    font.setItalic(false);
    p->setFont(font);

    // draw attributes
    const qreal numAtts = displayedAttributes();
    if (m_bShowAttributes) {
        drawMembers(*p, Uml::ot_Attribute, m_ShowAttSigs, textX,
                    m_bodyOffsetY, fontHeight);
    }

    // draw dividing line between attributes and operations
    if (!showNameOnly) {
        if (numAtts == 0)
            m_bodyOffsetY += fontHeight / 2;  // no atts, so just add a bit of space
        else
            m_bodyOffsetY += fontHeight * numAtts;
        setPenFromSettings(*p);
        p->drawLine(offsetX, m_bodyOffsetY, offsetX + w - 1, m_bodyOffsetY);
        p->setPen(QPen(Qt::black));
    }

    // draw operations
    if (m_bShowOperations) {
        drawMembers(*p, Uml::ot_Operation, m_ShowOpSigs, textX,
                    m_bodyOffsetY, fontHeight);
    }

    if (UMLWidget::isSelected())
        UMLWidget::drawSelected(p, offsetX, offsetY);
}

void ClassifierWidget::drawAsCircle(QPainter& p, qreal offsetX, qreal offsetY) {
    qreal w = getWidth();

    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const qreal fontHeight  = fm.lineSpacing();
    QString name;
    if ( m_bShowPackage ) {
        name = m_pObject->getFullyQualifiedName();
    } else {
        name = this -> getName();
    }

    p.drawEllipse(offsetX + w/2 - CIRCLE_SIZE/2, offsetY, CIRCLE_SIZE, CIRCLE_SIZE);
    p.setPen( QPen(Qt::black) );

    QFont font = UMLWidget::getFont();
    p.setFont(font);
    p.drawText(offsetX, offsetY + CIRCLE_SIZE, w, fontHeight, Qt::AlignCenter, name);

    if (isSelected()) {
        drawSelected(&p, offsetX, offsetY);
    }
}

QSizeF ClassifierWidget::calculateAsCircleSize()
{
    const QFontMetrics &fm = UMLWidget::getFontMetrics(UMLWidget::FT_ITALIC_UNDERLINE);
    const qreal fontHeight = fm.lineSpacing();

    qreal height = CIRCLE_SIZE + fontHeight;

    qreal width = CIRCLE_SIZE;
    QString displayedName;
    if (m_bShowPackage) {
        displayedName = m_pObject->getFullyQualifiedName();
    } else {
        displayedName = m_pObject->getName();
    }
    const qreal nameWidth = fm.size(0,displayedName).width();
    if (nameWidth > width)
        width = nameWidth;
    width += MARGIN * 2;

    return QSizeF(width, height);
}

void ClassifierWidget::drawMembers(QPainter & p, Uml::Object_Type ot, Uml::Signature_Type sigType,
                                   qreal x, qreal y, qreal fontHeight) {
    QFont f = UMLWidget::getFont();
    f.setBold(false);
    UMLClassifierListItemList list = getClassifier()->getFilteredList(ot);
    foreach (UMLClassifierListItem *obj , list ) {
          if (m_bShowPublicOnly && obj->getVisibility() != Uml::Visibility::Public)
            continue;
        QString text = obj->toString(sigType);
        f.setItalic( obj->getAbstract() );
        f.setUnderline( obj->getStatic() );
        p.setFont( f );
        QFontMetrics fontMetrics(f);
        p.drawText(x, y, fontMetrics.size(0,text).width(), fontHeight, Qt::AlignVCenter, text);
        f.setItalic(false);
        f.setUnderline(false);
        p.setFont(f);
        y += fontHeight;
    }
}

void ClassifierWidget::setDrawAsCircle(bool drawAsCircle) {
    m_bDrawAsCircle = drawAsCircle;
    updateComponentSize();
    update();
}

bool ClassifierWidget::getDrawAsCircle() const {
    return m_bDrawAsCircle;
}

void ClassifierWidget::toggleDrawAsCircle() {
    m_bDrawAsCircle = !m_bDrawAsCircle;
    updateSigs();
    updateComponentSize();
    update();
}

void ClassifierWidget::changeToClass() {
    WidgetBase::setBaseType(Uml::wt_Class);
    getClassifier()->setBaseType(Uml::ot_Class);

    const Settings::OptionState& ops = m_pScene->getOptionState();
    m_bShowAttributes = ops.classState.showAtts;
    m_bShowStereotype = ops.classState.showStereoType;

    updateComponentSize();
    update();
}

void ClassifierWidget::changeToInterface() {
    WidgetBase::setBaseType(Uml::wt_Interface);
    getClassifier()->setBaseType(Uml::ot_Interface);

    m_bShowAttributes = false;
    m_bShowStereotype = true;

    updateComponentSize();
    update();
}

void ClassifierWidget::adjustAssocs(qreal x, qreal y) {
    UMLWidget::adjustAssocs(x, y);

    if (m_pDoc->loading() || m_pAssocWidget == 0) {
        return;
    }

    m_pAssocWidget->computeAssocClassLine();
}

void ClassifierWidget::saveToXMI(QDomDocument & qDoc, QDomElement & qElement) {
    QDomElement conceptElement;
    UMLClassifier *umlc = getClassifier();
    if (umlc->isInterface())
        conceptElement = qDoc.createElement("interfacewidget");
    else
        conceptElement = qDoc.createElement("classwidget");
    UMLWidget::saveToXMI( qDoc, conceptElement );
    conceptElement.setAttribute( "showoperations", m_bShowOperations );
    conceptElement.setAttribute( "showpubliconly", m_bShowPublicOnly );
    conceptElement.setAttribute( "showopsigs", m_ShowOpSigs );
    conceptElement.setAttribute( "showpackage", m_bShowPackage );
    conceptElement.setAttribute( "showscope", m_bShowAccess );
    if (! umlc->isInterface()) {
        conceptElement.setAttribute("showattributes", m_bShowAttributes);
        conceptElement.setAttribute("showattsigs", m_ShowAttSigs);
    }
    if (umlc->isInterface() || umlc->getAbstract())
        conceptElement.setAttribute("drawascircle", m_bDrawAsCircle);
    qElement.appendChild( conceptElement );
}

bool ClassifierWidget::loadFromXMI(QDomElement & qElement) {
    if (!UMLWidget::loadFromXMI(qElement))
        return false;
    QString showatts = qElement.attribute( "showattributes", "0" );
    QString showops = qElement.attribute( "showoperations", "1" );
    QString showpubliconly = qElement.attribute( "showpubliconly", "0" );
    QString showattsigs = qElement.attribute( "showattsigs", "600" );
    QString showopsigs = qElement.attribute( "showopsigs", "600" );
    QString showpackage = qElement.attribute( "showpackage", "0" );
    QString showscope = qElement.attribute( "showscope", "0" );
    QString drawascircle = qElement.attribute("drawascircle", "0");

    m_bShowAttributes = (bool)showatts.toInt();
    m_bShowOperations = (bool)showops.toInt();
    m_bShowPublicOnly = (bool)showpubliconly.toInt();
    m_ShowAttSigs = (Uml::Signature_Type)showattsigs.toInt();
    m_ShowOpSigs = (Uml::Signature_Type)showopsigs.toInt();
    m_bShowPackage = (bool)showpackage.toInt();
    m_bShowAccess = (bool)showscope.toInt();
    m_bDrawAsCircle = (bool)drawascircle.toInt();

    return true;
}

