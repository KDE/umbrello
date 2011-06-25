/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "classifierwidget.h"
// qt/kde includes
#include <QtGui/QPainter>
// app includes
#include "classifier.h"
#include "debug_utils.h"
#include "operation.h"
#include "template.h"
#include "associationwidget.h"
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"
#include "listpopupmenu.h"
#include "object_factory.h"

ClassifierWidget::ClassifierWidget(UMLView * view, UMLClassifier *c)
  : UMLWidget(view, c)
{
    init();
    if (c != NULL && c->isInterface()) {
        WidgetBase::setBaseType(WidgetBase::wt_Interface);
        m_bShowStereotype = true;
        m_bShowAttributes = false;
        updateSigs();
    }
}

ClassifierWidget::~ClassifierWidget()
{
    if (m_pAssocWidget)
        m_pAssocWidget->removeAssocClassLine();
}

const int ClassifierWidget::MARGIN = 5;
const int ClassifierWidget::CIRCLE_SIZE = 30;

void ClassifierWidget::init()
{
    WidgetBase::setBaseType(WidgetBase::wt_Class);

    const Settings::OptionState& ops = m_pView->optionState();
    m_bShowAccess = ops.classState.showVisibility;
    m_bShowOperations = ops.classState.showOps;
    m_bShowPublicOnly = ops.classState.showPublicOnly;
    m_bShowPackage = ops.classState.showPackage;
    m_ShowAttSigs = Uml::SignatureType::ShowSig;
    /* setShowOpSigs( ops.classState.showOpSig );
      Cannot do that because we get "pure virtual method called". Open code:
     */
    if( !ops.classState.showOpSig ) {
        if (m_bShowAccess)
            m_ShowOpSigs = Uml::SignatureType::NoSig;
        else
            m_ShowOpSigs = Uml::SignatureType::NoSigNoVis;

    } else if (m_bShowAccess)
        m_ShowOpSigs = Uml::SignatureType::ShowSig;
    else
        m_ShowOpSigs = Uml::SignatureType::SigNoVis;

    m_bShowAttributes = ops.classState.showAtts;
    m_bShowStereotype = ops.classState.showStereoType;
    m_bDrawAsCircle = false;
    m_pAssocWidget = NULL;
    setShowAttSigs( ops.classState.showAttSig );
}

void ClassifierWidget::updateSigs()
{
    //turn on scope
    if (m_bShowAccess) {
        if (m_ShowOpSigs == Uml::SignatureType::NoSigNoVis) {
            m_ShowOpSigs = Uml::SignatureType::NoSig;
        } else if (m_ShowOpSigs == Uml::SignatureType::SigNoVis) {
            m_ShowOpSigs = Uml::SignatureType::ShowSig;
        }
    } else { //turn off scope
        if (m_ShowOpSigs == Uml::SignatureType::ShowSig) {
            m_ShowOpSigs = Uml::SignatureType::SigNoVis;
        } else if (m_ShowOpSigs == Uml::SignatureType::NoSig) {
            m_ShowOpSigs = Uml::SignatureType::NoSigNoVis;
        }
    }
    if (m_bShowAccess) {
        if (m_ShowAttSigs == Uml::SignatureType::NoSigNoVis)
            m_ShowAttSigs = Uml::SignatureType::NoSig;
        else if (m_ShowAttSigs == Uml::SignatureType::SigNoVis)
            m_ShowAttSigs = Uml::SignatureType::ShowSig;
    } else {
        if (m_ShowAttSigs == Uml::SignatureType::ShowSig)
            m_ShowAttSigs = Uml::SignatureType::SigNoVis;
        else if(m_ShowAttSigs == Uml::SignatureType::NoSig)
            m_ShowAttSigs = Uml::SignatureType::NoSigNoVis;
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

bool ClassifierWidget::getShowOps() const
{
    return m_bShowOperations;
}

void ClassifierWidget::setShowOps(bool _show)
{
    m_bShowOperations = _show;
    updateSigs();
    updateComponentSize();
    update();
}

void ClassifierWidget::toggleShowOps()
{
    m_bShowOperations = !m_bShowOperations;
    updateSigs();
    updateComponentSize();
    update();
}

bool ClassifierWidget::getShowPublicOnly() const
{
    return m_bShowPublicOnly;
}

void ClassifierWidget::setShowPublicOnly(bool _status)
{
    m_bShowPublicOnly = _status;
    updateComponentSize();
    update();
}

void ClassifierWidget::toggleShowPublicOnly()
{
    m_bShowPublicOnly = !m_bShowPublicOnly;
    updateComponentSize();
    update();
}

bool ClassifierWidget::getShowVisibility() const
{
    return m_bShowAccess;
}

void ClassifierWidget::setShowVisibility(bool _visibility)
{
    m_bShowAccess = _visibility;
    updateSigs();
    updateComponentSize();
    update();
}

void ClassifierWidget::toggleShowVisibility()
{
    m_bShowAccess = !m_bShowAccess;
    updateSigs();
    updateComponentSize();
    update();
}

Uml::SignatureType ClassifierWidget::operationSignatureType() const
{
    return m_ShowOpSigs;
}

void ClassifierWidget::setShowOpSigs(bool _status)
{
    if( !_status ) {
        if (m_bShowAccess)
            m_ShowOpSigs = Uml::SignatureType::NoSig;
        else
            m_ShowOpSigs = Uml::SignatureType::NoSigNoVis;
    } else if (m_bShowAccess)
        m_ShowOpSigs = Uml::SignatureType::ShowSig;
    else
        m_ShowOpSigs = Uml::SignatureType::SigNoVis;
    updateComponentSize();
    update();
}

void ClassifierWidget::toggleShowOpSigs()
{
    if (m_ShowOpSigs == Uml::SignatureType::ShowSig || m_ShowOpSigs == Uml::SignatureType::SigNoVis) {
        if (m_bShowAccess) {
            m_ShowOpSigs = Uml::SignatureType::NoSig;
        } else {
            m_ShowOpSigs = Uml::SignatureType::NoSigNoVis;
        }
    } else if (m_bShowAccess) {
        m_ShowOpSigs = Uml::SignatureType::ShowSig;
    } else {
        m_ShowOpSigs = Uml::SignatureType::SigNoVis;
    }
    updateComponentSize();
    update();
}

bool ClassifierWidget::getShowPackage() const
{
    return m_bShowPackage;
}

void ClassifierWidget::setShowPackage(bool _status)
{
    m_bShowPackage = _status;
    updateComponentSize();
    update();
}

void ClassifierWidget::toggleShowPackage()
{
    m_bShowPackage = !m_bShowPackage;
    updateSigs();
    updateComponentSize();
    update();
}

void ClassifierWidget::setOpSignature(Uml::SignatureType sig)
{
    m_ShowOpSigs = sig;
    updateSigs();
    updateComponentSize();
    update();
}

void ClassifierWidget::setShowAtts(bool _show)
{
    m_bShowAttributes = _show;
    updateSigs();

    updateComponentSize();
    update();
}

void ClassifierWidget::setAttSignature(Uml::SignatureType sig)
{
    m_ShowAttSigs = sig;
    updateSigs();
    updateComponentSize();
    update();
}

void ClassifierWidget::setShowAttSigs(bool _status)
{
    if( !_status ) {
        if (m_bShowAccess)
            m_ShowAttSigs = Uml::SignatureType::NoSig;
        else
            m_ShowAttSigs = Uml::SignatureType::NoSigNoVis;
    }
    else if (m_bShowAccess)
        m_ShowAttSigs = Uml::SignatureType::ShowSig;
    else
        m_ShowAttSigs = Uml::SignatureType::SigNoVis;
    if (UMLApp::app()->document()->loading())
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
    if (m_ShowAttSigs == Uml::SignatureType::ShowSig ||
            m_ShowAttSigs == Uml::SignatureType::SigNoVis) {
        if (m_bShowAccess) {
            m_ShowAttSigs = Uml::SignatureType::NoSig;
        } else {
            m_ShowAttSigs = Uml::SignatureType::NoSigNoVis;
        }
    } else if (m_bShowAccess) {
        m_ShowAttSigs = Uml::SignatureType::ShowSig;
    } else {
        m_ShowAttSigs = Uml::SignatureType::SigNoVis;
    }
    updateComponentSize();
    update();
}

int ClassifierWidget::displayedMembers(UMLObject::Object_Type ot)
{
    int count = 0;
    UMLClassifierListItemList list = classifier()->getFilteredList(ot);
    foreach (UMLClassifierListItem *m , list ) {
      if (!(m_bShowPublicOnly && m->visibility() != Uml::Visibility::Public))
            count++;
    }
    return count;
}

int ClassifierWidget::displayedOperations()
{
    if (!m_bShowOperations)
        return 0;
    return displayedMembers(UMLObject::ot_Operation);
}

QSize ClassifierWidget::calculateSize()
{
    if (!m_pObject) {
        return UMLWidget::calculateSize();
    }
    if (classifier()->isInterface() && m_bDrawAsCircle) {
        return calculateAsCircleSize();
    }

    const QFontMetrics &fm = getFontMetrics(UMLWidget::FT_NORMAL);
    const int fontHeight = fm.lineSpacing();
    // width is the width of the longest 'word'
    int width = 0, height = 0;

    // consider stereotype
    if (m_bShowStereotype && !m_pObject->stereotype().isEmpty()) {
        height += fontHeight;
        // ... width
        const QFontMetrics &bfm = UMLWidget::getFontMetrics(UMLWidget::FT_BOLD);
        const int stereoWidth = bfm.size(0,m_pObject->stereotype(true)).width();
        if (stereoWidth > width)
            width = stereoWidth;
    }

    // consider name
    height += fontHeight;
    // ... width
    QString displayedName;
    if (m_bShowPackage)
        displayedName = m_pObject->fullyQualifiedName();
    else
        displayedName = m_pObject->name();
    const UMLWidget::FontType nft = (m_pObject->isAbstract() ? FT_BOLD_ITALIC : FT_BOLD);
    //const int nameWidth = getFontMetrics(nft).boundingRect(displayName).width();
    const int nameWidth = UMLWidget::getFontMetrics(nft).size(0,displayedName).width();
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
        foreach (UMLClassifierListItem *a , list ) {
            if (m_bShowPublicOnly && a->visibility() != Uml::Visibility::Public)
                continue;
            const int attWidth = fm.size(0,a->toString(m_ShowAttSigs)).width();
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
        foreach (UMLOperation* op ,  list) {
                  if (m_bShowPublicOnly && op->visibility() != Uml::Visibility::Public)
                continue;
            const QString displayedOp = op->toString(m_ShowOpSigs);
            UMLWidget::FontType oft;
            oft = (op->isAbstract() ? UMLWidget::FT_ITALIC : UMLWidget::FT_NORMAL);
            const int w = UMLWidget::getFontMetrics(oft).size(0,displayedOp).width();
            if (w > width)
                width = w;
        }
    }

    // consider template box _as last_ !
    QSize templatesBoxSize = calculateTemplatesBoxSize();
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

    return QSize(width, height);
}

void ClassifierWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::Menu_Type sel = m_pMenu->getMenuType(action);
    switch (sel) {
    case ListPopupMenu::mt_Attribute:
    case ListPopupMenu::mt_Operation:
    case ListPopupMenu::mt_Template:
        {
            UMLObject::Object_Type ot = ListPopupMenu::convert_MT_OT(sel);
            if (Object_Factory::createChildObject(classifier(), ot)) {
                updateComponentSize();
                update();
                UMLApp::app()->document()->setModified();
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

QSize ClassifierWidget::calculateTemplatesBoxSize()
{
    UMLTemplateList list = classifier()->getTemplateList();
    int count = list.count();
    if (count == 0) {
        return QSize(0, 0);
    }

    int width, height;
    height = width = 0;

    QFont font = UMLWidget::font();
    font.setItalic(false);
    font.setUnderline(false);
    font.setBold(false);
    const QFontMetrics fm(font);

    height = count * fm.lineSpacing() + (MARGIN*2);

    foreach (UMLTemplate *t , list ) {
        int textWidth = fm.size(0, t->toString() ).width();
        if (textWidth > width)
            width = textWidth;
    }

    width += (MARGIN*2);
    return QSize(width, height);
}

int ClassifierWidget::displayedAttributes()
{
    if (!m_bShowAttributes)
        return 0;
    return displayedMembers(UMLObject::ot_Attribute);
}

void ClassifierWidget::setClassAssocWidget(AssociationWidget *assocwidget)
{
    m_pAssocWidget = assocwidget;
    UMLAssociation *umlassoc = NULL;
    if (assocwidget)
        umlassoc = assocwidget->getAssociation();
    classifier()->setClassAssoc(umlassoc);
}

AssociationWidget *ClassifierWidget::getClassAssocWidget()
{
    return m_pAssocWidget;
}

UMLClassifier *ClassifierWidget::classifier()
{
    return static_cast<UMLClassifier*>(m_pObject);
}

void ClassifierWidget::draw(QPainter & p, int offsetX, int offsetY)
{
    setPenFromSettings(p);
    if ( UMLWidget::getUseFillColour() )
        p.setBrush( UMLWidget::getFillColor() );
    else
        p.setBrush( m_pView->viewport()->palette().color(QPalette::Background) );

    if (classifier()->isInterface() && m_bDrawAsCircle) {
        drawAsCircle(p, offsetX, offsetY);
        return;
    }

    // Draw the bounding rectangle
    QSize templatesBoxSize = calculateTemplatesBoxSize();
    m_bodyOffsetY = offsetY;
    if (templatesBoxSize.height() > 0)
        m_bodyOffsetY += templatesBoxSize.height() - MARGIN;
    int w = width();
    if (templatesBoxSize.width() > 0)
        w -= templatesBoxSize.width() / 2;
    int h = height();
    if (templatesBoxSize.height() > 0)
        h -= templatesBoxSize.height() - MARGIN;
    p.drawRect(offsetX, m_bodyOffsetY, w, h);

    QFont font = UMLWidget::font();
    font.setUnderline(false);
    font.setItalic(false);
    const QFontMetrics fm = UMLWidget::getFontMetrics(UMLWidget::FT_NORMAL);
    const int fontHeight = fm.lineSpacing();

    //If there are any templates then draw them
    UMLTemplateList tlist = classifier()->getTemplateList();
    if ( tlist.count() > 0 ) {
        setPenFromSettings(p);
        QPen pen = p.pen();
        pen.setStyle(Qt::DotLine);
        p.setPen(pen);
        p.drawRect( offsetX + width() - templatesBoxSize.width(), offsetY,
                    templatesBoxSize.width(), templatesBoxSize.height() );
        p.setPen( QPen(Qt::black) );
        font.setBold(false);
        p.setFont(font);
        const int x = offsetX + width() - templatesBoxSize.width() + MARGIN;
        int y = offsetY + MARGIN;
        foreach ( UMLTemplate *t , tlist ) {
            QString text = t->toString();
            p.drawText(x, y, fm.size(0,text).width(), fontHeight, Qt::AlignVCenter, text);
            y += fontHeight;
        }
    }

    const int textX = offsetX + MARGIN;
    const int textWidth = w - MARGIN * 2;

    p.setPen(QPen(Qt::black));

    // draw stereotype
    font.setBold(true);
    QString stereo = m_pObject->stereotype();
    /* if no stereotype is given we don't want to show the empty << >> */
    const bool showStereotype = (m_bShowStereotype && !stereo.isEmpty());
    const bool showNameOnly = (!m_bShowOperations && !m_bShowAttributes && !showStereotype);
    int nameHeight = fontHeight;
    if (showNameOnly) {
        nameHeight = h;
    } else if (showStereotype) {
        p.setFont(font);
        stereo = m_pObject->stereotype(true);
        p.drawText(textX, m_bodyOffsetY, textWidth, fontHeight, Qt::AlignCenter, stereo);
        m_bodyOffsetY += fontHeight;
    }

    // draw name
    QString name;
    if (m_bShowPackage) {
        name = m_pObject->fullyQualifiedName();
    } else {
        name = this->name();
    }
    font.setItalic( m_pObject->isAbstract() );
    p.setFont(font);
    p.drawText(textX, m_bodyOffsetY, textWidth, nameHeight, Qt::AlignCenter, name);
    if (!showNameOnly) {
        m_bodyOffsetY += fontHeight;
        setPenFromSettings(p);
        p.drawLine(offsetX, m_bodyOffsetY, offsetX + w - 1, m_bodyOffsetY);
        p.setPen(QPen(Qt::black));
    }
    font.setBold(false);
    font.setItalic(false);
    p.setFont(font);

    // draw attributes
    const int numAtts = displayedAttributes();
    if (m_bShowAttributes) {
        drawMembers(p, UMLObject::ot_Attribute, m_ShowAttSigs, textX,
                    m_bodyOffsetY, fontHeight);
    }

    // draw dividing line between attributes and operations
    if (!showNameOnly) {
        if (numAtts == 0)
            m_bodyOffsetY += fontHeight / 2;  // no atts, so just add a bit of space
        else
            m_bodyOffsetY += fontHeight * numAtts;
        setPenFromSettings(p);
        p.drawLine(offsetX, m_bodyOffsetY, offsetX + w - 1, m_bodyOffsetY);
        p.setPen(QPen(Qt::black));
    }

    // draw operations
    if (m_bShowOperations) {
        drawMembers(p, UMLObject::ot_Operation, m_ShowOpSigs, textX,
                    m_bodyOffsetY, fontHeight);
    }

    if (UMLWidget::m_bSelected)
        UMLWidget::drawSelected(&p, offsetX, offsetY);
}

void ClassifierWidget::drawAsCircle(QPainter& p, int offsetX, int offsetY)
{
    int w = width();

    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    QString name;
    if ( m_bShowPackage ) {
        name = m_pObject->fullyQualifiedName();
    } else {
        name = this->name();
    }

    p.drawEllipse(offsetX + w/2 - CIRCLE_SIZE/2, offsetY, CIRCLE_SIZE, CIRCLE_SIZE);
    p.setPen( QPen(Qt::black) );

    QFont font = UMLWidget::font();
    p.setFont(font);
    p.drawText(offsetX, offsetY + CIRCLE_SIZE, w, fontHeight, Qt::AlignCenter, name);

    if (m_bSelected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

QSize ClassifierWidget::calculateAsCircleSize()
{
    const QFontMetrics &fm = UMLWidget::getFontMetrics(UMLWidget::FT_ITALIC_UNDERLINE);
    const int fontHeight = fm.lineSpacing();

    int height = CIRCLE_SIZE + fontHeight;

    int width = CIRCLE_SIZE;
    QString displayedName;
    if (m_bShowPackage) {
        displayedName = m_pObject->fullyQualifiedName();
    } else {
        displayedName = m_pObject->name();
    }
    const int nameWidth = fm.size(0,displayedName).width();
    if (nameWidth > width)
        width = nameWidth;
    width += MARGIN * 2;

    return QSize(width, height);
}

void ClassifierWidget::drawMembers(QPainter & p, UMLObject::Object_Type ot, Uml::SignatureType sigType,
                                   int x, int y, int fontHeight)
{
    QFont f = UMLWidget::font();
    f.setBold(false);
    UMLClassifierListItemList list = classifier()->getFilteredList(ot);
    foreach (UMLClassifierListItem *obj , list ) {
          if (m_bShowPublicOnly && obj->visibility() != Uml::Visibility::Public)
            continue;
        QString text = obj->toString(sigType);
        f.setItalic( obj->isAbstract() );
        f.setUnderline( obj->isStatic() );
        p.setFont( f );
        QFontMetrics fontMetrics(f);
        p.drawText(x, y, fontMetrics.size(0,text).width(), fontHeight, Qt::AlignVCenter, text);
        f.setItalic(false);
        f.setUnderline(false);
        p.setFont(f);
        y += fontHeight;
    }
}

void ClassifierWidget::setDrawAsCircle(bool drawAsCircle)
{
    m_bDrawAsCircle = drawAsCircle;
    updateComponentSize();
    update();
}

bool ClassifierWidget::getDrawAsCircle() const
{
    return m_bDrawAsCircle;
}

void ClassifierWidget::toggleDrawAsCircle()
{
    m_bDrawAsCircle = !m_bDrawAsCircle;
    updateSigs();
    updateComponentSize();
    update();
}

void ClassifierWidget::changeToClass()
{
    WidgetBase::setBaseType(WidgetBase::wt_Class);
    classifier()->setBaseType(UMLObject::ot_Class);

    const Settings::OptionState& ops = m_pView->optionState();
    m_bShowAttributes = ops.classState.showAtts;
    m_bShowStereotype = ops.classState.showStereoType;

    updateComponentSize();
    update();
}

void ClassifierWidget::changeToInterface()
{
    WidgetBase::setBaseType(WidgetBase::wt_Interface);
    classifier()->setBaseType(UMLObject::ot_Interface);

    m_bShowAttributes = false;
    m_bShowStereotype = true;

    updateComponentSize();
    update();
}

void ClassifierWidget::adjustAssocs(int x, int y)
{
    UMLWidget::adjustAssocs(x, y);

    if (m_pDoc->loading() || m_pAssocWidget == 0) {
        return;
    }

    m_pAssocWidget->computeAssocClassLine();
}

void ClassifierWidget::saveToXMI(QDomDocument & qDoc, QDomElement & qElement)
{
    QDomElement conceptElement;
    UMLClassifier *umlc = classifier();
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
    if (umlc->isInterface() || umlc->isAbstract())
        conceptElement.setAttribute("drawascircle", m_bDrawAsCircle);
    qElement.appendChild( conceptElement );
}

bool ClassifierWidget::loadFromXMI(QDomElement & qElement)
{
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
    m_ShowAttSigs = Uml::SignatureType::Value(showattsigs.toInt());
    m_ShowOpSigs = Uml::SignatureType::Value(showopsigs.toInt());
    m_bShowPackage = (bool)showpackage.toInt();
    m_bShowAccess = (bool)showscope.toInt();
    m_bDrawAsCircle = (bool)drawascircle.toInt();

    return true;
}

