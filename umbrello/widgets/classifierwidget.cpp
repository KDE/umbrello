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

ClassifierWidget::ClassifierWidget(UMLScene * scene, UMLClassifier *c)
  : UMLWidget(scene, WidgetBase::wt_Class, c)
{
    init();
    if (c != NULL && c->isInterface()) {
        setBaseType(WidgetBase::wt_Interface);
        m_showStereotype = true;
        m_showAttributes = false;
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
    const Settings::OptionState& ops = m_scene->optionState();
    m_showAccess = ops.classState.showVisibility;
    m_showOperations = ops.classState.showOps;
    m_showPublicOnly = ops.classState.showPublicOnly;
    m_showPackage = ops.classState.showPackage;
    m_ShowAttSigs = Uml::SignatureType::ShowSig;
    /* setShowOpSigs( ops.classState.showOpSig );
      Cannot do that because we get "pure virtual method called". Open code:
     */
    if( !ops.classState.showOpSig ) {
        if (m_showAccess)
            m_ShowOpSigs = Uml::SignatureType::NoSig;
        else
            m_ShowOpSigs = Uml::SignatureType::NoSigNoVis;

    } else if (m_showAccess)
        m_ShowOpSigs = Uml::SignatureType::ShowSig;
    else
        m_ShowOpSigs = Uml::SignatureType::SigNoVis;

    m_showAttributes = ops.classState.showAtts;
    m_showStereotype = ops.classState.showStereoType;
    m_drawAsCircle = false;
    m_pAssocWidget = NULL;

    setShowAttSigs( ops.classState.showAttSig );
}

void ClassifierWidget::updateSigs()
{
    //turn on scope
    if (m_showAccess) {
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
    if (m_showAccess) {
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
    m_showStereotype = !m_showStereotype;
    updateSigs();
    updateComponentSize();
    update();
}

bool ClassifierWidget::getShowOps() const
{
    return m_showOperations;
}

void ClassifierWidget::setShowOps(bool _show)
{
    m_showOperations = _show;
    updateSigs();
    updateComponentSize();
    update();
}

void ClassifierWidget::toggleShowOps()
{
    m_showOperations = !m_showOperations;
    updateSigs();
    updateComponentSize();
    update();
}

bool ClassifierWidget::getShowPublicOnly() const
{
    return m_showPublicOnly;
}

void ClassifierWidget::setShowPublicOnly(bool _status)
{
    m_showPublicOnly = _status;
    updateComponentSize();
    update();
}

void ClassifierWidget::toggleShowPublicOnly()
{
    m_showPublicOnly = !m_showPublicOnly;
    updateComponentSize();
    update();
}

bool ClassifierWidget::getShowVisibility() const
{
    return m_showAccess;
}

void ClassifierWidget::setShowVisibility(bool _visibility)
{
    m_showAccess = _visibility;
    updateSigs();
    updateComponentSize();
    update();
}

void ClassifierWidget::toggleShowVisibility()
{
    m_showAccess = !m_showAccess;
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
        if (m_showAccess)
            m_ShowOpSigs = Uml::SignatureType::NoSig;
        else
            m_ShowOpSigs = Uml::SignatureType::NoSigNoVis;
    } else if (m_showAccess)
        m_ShowOpSigs = Uml::SignatureType::ShowSig;
    else
        m_ShowOpSigs = Uml::SignatureType::SigNoVis;
    updateComponentSize();
    update();
}

void ClassifierWidget::toggleShowOpSigs()
{
    if (m_ShowOpSigs == Uml::SignatureType::ShowSig || m_ShowOpSigs == Uml::SignatureType::SigNoVis) {
        if (m_showAccess) {
            m_ShowOpSigs = Uml::SignatureType::NoSig;
        } else {
            m_ShowOpSigs = Uml::SignatureType::NoSigNoVis;
        }
    } else if (m_showAccess) {
        m_ShowOpSigs = Uml::SignatureType::ShowSig;
    } else {
        m_ShowOpSigs = Uml::SignatureType::SigNoVis;
    }
    updateComponentSize();
    update();
}

bool ClassifierWidget::getShowPackage() const
{
    return m_showPackage;
}

void ClassifierWidget::setShowPackage(bool _status)
{
    m_showPackage = _status;
    updateComponentSize();
    update();
}

void ClassifierWidget::toggleShowPackage()
{
    m_showPackage = !m_showPackage;
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
    m_showAttributes = _show;
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
        if (m_showAccess)
            m_ShowAttSigs = Uml::SignatureType::NoSig;
        else
            m_ShowAttSigs = Uml::SignatureType::NoSigNoVis;
    }
    else if (m_showAccess)
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
    m_showAttributes = !m_showAttributes;
    updateSigs();
    updateComponentSize();
    update();
}

void ClassifierWidget::toggleShowAttSigs()
{
    if (m_ShowAttSigs == Uml::SignatureType::ShowSig ||
            m_ShowAttSigs == Uml::SignatureType::SigNoVis) {
        if (m_showAccess) {
            m_ShowAttSigs = Uml::SignatureType::NoSig;
        } else {
            m_ShowAttSigs = Uml::SignatureType::NoSigNoVis;
        }
    } else if (m_showAccess) {
        m_ShowAttSigs = Uml::SignatureType::ShowSig;
    } else {
        m_ShowAttSigs = Uml::SignatureType::SigNoVis;
    }
    updateComponentSize();
    update();
}

int ClassifierWidget::displayedMembers(UMLObject::ObjectType ot)
{
    int count = 0;
    UMLClassifierListItemList list = classifier()->getFilteredList(ot);
    foreach (UMLClassifierListItem *m , list ) {
      if (!(m_showPublicOnly && m->visibility() != Uml::Visibility::Public))
            count++;
    }
    return count;
}

int ClassifierWidget::displayedOperations()
{
    if (!m_showOperations)
        return 0;
    return displayedMembers(UMLObject::ot_Operation);
}

UMLSceneSize ClassifierWidget::minimumSize()
{
    if (!m_pObject) {
        return UMLWidget::minimumSize();
    }
    if (classifier()->isInterface() && m_drawAsCircle) {
        return calculateAsCircleSize();
    }

    const QFontMetrics &fm = getFontMetrics(UMLWidget::FT_NORMAL);
    const int fontHeight = fm.lineSpacing();
    // width is the width of the longest 'word'
    int width = 0, height = 0;

    // consider stereotype
    if (m_showStereotype && !m_pObject->stereotype().isEmpty()) {
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
    if (m_showPackage)
        displayedName = m_pObject->fullyQualifiedName();
    else
        displayedName = m_pObject->name();
    const UMLWidget::FontType nft = (m_pObject->isAbstract() ? FT_BOLD_ITALIC : FT_BOLD);
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
            if (m_showPublicOnly && a->visibility() != Uml::Visibility::Public)
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
                  if (m_showPublicOnly && op->visibility() != Uml::Visibility::Public)
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
    if (!m_showOperations && !m_showAttributes && !m_showStereotype) {
        height += MARGIN * 2;
    }

    // allow for width margin
    width += MARGIN * 2;

    return UMLSceneSize(width, height);
}

void ClassifierWidget::slotMenuSelection(QAction* action)
{
    ListPopupMenu::MenuType sel = m_pMenu->getMenuType(action);
    switch (sel) {
    case ListPopupMenu::mt_Attribute:
    case ListPopupMenu::mt_Operation:
    case ListPopupMenu::mt_Template:
        {
            UMLObject::ObjectType ot = ListPopupMenu::convert_MT_OT(sel);
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
    if (!m_showAttributes)
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

void ClassifierWidget::paint(QPainter & p, int offsetX, int offsetY)
{
    setPenFromSettings(p);
    if ( UMLWidget::useFillColor() )
        p.setBrush( UMLWidget::fillColor() );
    else
        p.setBrush( m_scene->viewport()->palette().color(QPalette::Background) );

    if (classifier()->isInterface() && m_drawAsCircle) {
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
        p.setPen( QPen(textColor()));
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

    p.setPen(QPen(textColor()));

    // draw stereotype
    font.setBold(true);
    QString stereo = m_pObject->stereotype();
    /* if no stereotype is given we don't want to show the empty << >> */
    const bool showStereotype = (m_showStereotype && !stereo.isEmpty());
    const bool showNameOnly = (!m_showOperations && !m_showAttributes && !showStereotype);
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
    if (m_showPackage) {
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
        p.setPen(textColor());
    }
    font.setBold(false);
    font.setItalic(false);
    p.setFont(font);

    // draw attributes
    const int numAtts = displayedAttributes();
    if (m_showAttributes) {
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
        p.setPen(QPen(textColor()));
    }

    // draw operations
    if (m_showOperations) {
        drawMembers(p, UMLObject::ot_Operation, m_ShowOpSigs, textX,
                    m_bodyOffsetY, fontHeight);
    }

    if (UMLWidget::m_selected)
        UMLWidget::drawSelected(&p, offsetX, offsetY);
}

void ClassifierWidget::drawAsCircle(QPainter& p, int offsetX, int offsetY)
{
    int w = width();

    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    QString name;
    if ( m_showPackage ) {
        name = m_pObject->fullyQualifiedName();
    } else {
        name = this->name();
    }

    p.drawEllipse(offsetX + w/2 - CIRCLE_SIZE/2, offsetY, CIRCLE_SIZE, CIRCLE_SIZE);
    p.setPen( QPen(textColor()));

    QFont font = UMLWidget::font();
    p.setFont(font);
    p.drawText(offsetX, offsetY + CIRCLE_SIZE, w, fontHeight, Qt::AlignCenter, name);

    if (m_selected) {
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
    if (m_showPackage) {
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

void ClassifierWidget::drawMembers(QPainter & p, UMLObject::ObjectType ot, Uml::SignatureType sigType,
                                   int x, int y, int fontHeight)
{
    QFont f = UMLWidget::font();
    f.setBold(false);
    UMLClassifierListItemList list = classifier()->getFilteredList(ot);
    foreach (UMLClassifierListItem *obj , list ) {
          if (m_showPublicOnly && obj->visibility() != Uml::Visibility::Public)
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
    m_drawAsCircle = drawAsCircle;
    updateComponentSize();
    update();
}

bool ClassifierWidget::getDrawAsCircle() const
{
    return m_drawAsCircle;
}

void ClassifierWidget::toggleDrawAsCircle()
{
    m_drawAsCircle = !m_drawAsCircle;
    updateSigs();
    updateComponentSize();
    update();
}

void ClassifierWidget::changeToClass()
{
    WidgetBase::setBaseType(WidgetBase::wt_Class);
    classifier()->setBaseType(UMLObject::ot_Class);

    const Settings::OptionState& ops = m_scene->optionState();
    m_showAttributes = ops.classState.showAtts;
    m_showStereotype = ops.classState.showStereoType;

    updateComponentSize();
    update();
}

void ClassifierWidget::changeToInterface()
{
    WidgetBase::setBaseType(WidgetBase::wt_Interface);
    classifier()->setBaseType(UMLObject::ot_Interface);

    m_showAttributes = false;
    m_showStereotype = true;

    updateComponentSize();
    update();
}

void ClassifierWidget::adjustAssocs(int x, int y)
{
    UMLWidget::adjustAssocs(x, y);

    if (m_doc->loading() || m_pAssocWidget == 0) {
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
    conceptElement.setAttribute( "showoperations", m_showOperations );
    conceptElement.setAttribute( "showpubliconly", m_showPublicOnly );
    conceptElement.setAttribute( "showopsigs", m_ShowOpSigs );
    conceptElement.setAttribute( "showpackage", m_showPackage );
    conceptElement.setAttribute( "showscope", m_showAccess );
    if (! umlc->isInterface()) {
        conceptElement.setAttribute("showattributes", m_showAttributes);
        conceptElement.setAttribute("showattsigs", m_ShowAttSigs);
    }
    if (umlc->isInterface() || umlc->isAbstract())
        conceptElement.setAttribute("drawascircle", m_drawAsCircle);
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

    m_showAttributes = (bool)showatts.toInt();
    m_showOperations = (bool)showops.toInt();
    m_showPublicOnly = (bool)showpubliconly.toInt();
    m_ShowAttSigs = Uml::SignatureType::Value(showattsigs.toInt());
    m_ShowOpSigs = Uml::SignatureType::Value(showopsigs.toInt());
    m_showPackage = (bool)showpackage.toInt();
    m_showAccess = (bool)showscope.toInt();
    m_drawAsCircle = (bool)drawascircle.toInt();

    return true;
}

