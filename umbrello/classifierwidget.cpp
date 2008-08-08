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

// app includes
#include "associationwidget.h"
#include "classifier.h"
#include "expanderbox.h"
#include "listpopupmenu.h"
#include "object_factory.h"
#include "operation.h"
#include "template.h"
#include "textitem.h"
#include "textitemgroup.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

const qreal ClassifierWidget::CircleMinimumRadius = 30;
const int ClassifierWidget::InvalidIndex = 99999;

/**
 * Constructs a ClassifierWidget.
 *
 * @param c The UMLObject to represent.
 */
ClassifierWidget::ClassifierWidget(UMLClassifier *c)
    : NewUMLRectWidget(c)
{
    createTextItemGroup(); // For classifier text items
    createTextItemGroup(); // For template text items'

    m_attributeExpanderBox = new ExpanderBox(false, this);
    connect(m_attributeExpanderBox, SIGNAL(expansionToggled(bool)), this, SLOT(slotShowAttributes(bool)));

    m_operationExpanderBox = new ExpanderBox(false, this);
    connect(m_operationExpanderBox, SIGNAL(expansionToggled(bool)), this, SLOT(slotShowOperations(bool)));

    // Null initially
    m_dummyAttributeItem = m_dummyOperationItem = 0;
    m_lineItem1Index = m_lineItem2Index = InvalidIndex;

    m_baseType = Uml::wt_Class;
    // Set the following properties by default. The remaining
    // properties are not set.
    m_visualProperties = ShowOperations | ShowVisibility | ShowAttributes;

    m_attributeSignatureType = Uml::st_ShowSig;
    m_operationSignatureType = Uml::st_ShowSig;

    // Intially this is null
    m_classAssociationWidget = 0;

    // Check if this widget is representing an interface and set
    // properties corresponding to that if it is.
    if (c && c->isInterface()) {
        m_baseType = Uml::wt_Interface;
        m_visualProperties = ShowOperations | ShowVisibility | ShowStereotype;
        setShowStereotype(true);
    }
}

/**
 * Destructor
 * @todo Reimplement this properly after implementing the Associationwidget
 */
ClassifierWidget::~ClassifierWidget()
{
    if (m_classAssociationWidget) {
        m_classAssociationWidget->removeAssocClassLine();
    }
}

/// @return the UMLClassifier which this ClassifierWidget represents.
UMLClassifier *ClassifierWidget::classifier() const
{
    return static_cast<UMLClassifier*>(umlObject());
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
 * @note Use @ref attributeSignatureType() and @ref
 *       operationSignatureType() to get signature status.  This
 *       method only indicates whether signature is visible or not.
 */
bool ClassifierWidget::visualProperty(VisualProperty property) const
{
    if (property == ShowAttributeSignature) {
        return (m_attributeSignatureType == Uml::st_ShowSig
                || m_attributeSignatureType == Uml::st_SigNoVis);
    }

    else if(property == ShowOperationSignature) {
        return (m_operationSignatureType == Uml::st_ShowSig
                || m_operationSignatureType == Uml::st_SigNoVis);
    }

    return m_visualProperties.testFlag(property);
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
void ClassifierWidget::setVisualProperty(VisualProperty property, bool enable)
{
    // Handle ShowAttributeSignature and ShowOperationSignature
    // specially.

    if (property == ShowAttributeSignature) {
        if (!enable) {
            m_attributeSignatureType = visualProperty(ShowVisibility) ?
                Uml::st_NoSig : Uml::st_NoSigNoVis;
        }
        else {
            m_attributeSignatureType = visualProperty(ShowVisibility) ?
                Uml::st_ShowSig : Uml::st_SigNoVis;
        }
        updateTextItemGroups();
    }

    else if (property == ShowOperationSignature) {
        if (!enable) {
            m_operationSignatureType = visualProperty(ShowVisibility) ?
                Uml::st_NoSig : Uml::st_NoSigNoVis;
        }
        else {
            m_operationSignatureType = visualProperty(ShowVisibility) ?
                Uml::st_ShowSig : Uml::st_SigNoVis;
        }
        updateTextItemGroups();
    }

    else if (property == ShowStereotype) {
        // Now just update flag and use base method for actual work.
        if (enable) {
            m_visualProperties |= property;
        }
        else {
            enable &= ~property;
        }
        setShowStereotype(enable);
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
        }
        else {
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
        oppositeStatus = !(m_operationSignatureType == Uml::st_ShowSig
                           || m_operationSignatureType == Uml::st_SigNoVis);
    }
    else if (property == ShowAttributeSignature) {
        oppositeStatus = !(m_attributeSignatureType == Uml::st_ShowSig
                           || m_attributeSignatureType == Uml::st_SigNoVis);
    }
    else {
        oppositeStatus = !visualProperty(property);
    }

    uDebug() << "VisualProperty:" << property << "to opposite status " << oppositeStatus;
    setVisualProperty(property, oppositeStatus);
}

/**
 * @retval true  If this widget represents interface and DrawAsCircle
 *               property is set.
 */
bool ClassifierWidget::shouldDrawAsCircle() const
{
    return baseType() == Uml::wt_Interface && visualProperty(DrawAsCircle) == true;
}

/**
 * Set's the attribute signature type to \a sigType.
 */
void ClassifierWidget::setAttributeSignature(Uml::Signature_Type sigType)
{
    m_attributeSignatureType = sigType;
    updateSignatureTypes();
}

/**
 * Set's the operation signature type to \a sigType.
 */
void ClassifierWidget::setOperationSignature(Uml::Signature_Type sigType)
{
    m_operationSignatureType = sigType;
    updateSignatureTypes();
}

/**
 * Changes this classifier from an interface to a class.  Attributes
 * and stereotype visibility is got from the view OptionState.  This
 * widget is also updated.
 */
void ClassifierWidget::changeToClass()
{
    m_baseType = Uml::wt_Class;
    classifier()->setBaseType(Uml::ot_Class);

    bool showAtts = true;
    bool showStereotype = false;

    if (umlScene()) {
        const Settings::OptionState& ops = umlScene()->getOptionState();
        showAtts = ops.classState.showAtts;
        showStereotype = ops.classState.showStereoType;
    }

    setVisualProperty(ShowAttributes, showAtts);
    setVisualProperty(ShowStereotype, showStereotype);

    updateTextItemGroups();
}

/**
 * Changes this classifier from a class to an interface.  Attributes
 * are hidden and stereotype is shown.  This widget is also updated.
 */
void ClassifierWidget::changeToInterface()
{
    m_baseType = Uml::wt_Interface;
    classifier()->setBaseType(Uml::ot_Interface);

    setVisualProperty(ShowAttributes, false);
    setVisualProperty(ShowStereotype, true);

    updateTextItemGroups();
}

/**
 * Set the AssociationWidget when this ClassWidget acts as an
 * association class.
 */
void ClassifierWidget::setClassAssociationWidget(AssociationWidget *assocwidget)
{
    m_classAssociationWidget = assocwidget;
    UMLAssociation *umlassoc = 0;
    if (assocwidget) {
        umlassoc = assocwidget->getAssociation();
    }
    classifier()->setClassAssoc(umlassoc);
}

/**
 * Extends base method to adjust also the association of a class
 * association.  Executes the base method and then, if file isn't
 * loading and the classifier acts as a class association, the
 * association position is updated.
 *
 * @param x The x-coordinate.
 * @param y The y-coordinate.
 *
 * @todo Implement this properly after implementing AssociationWidget.
 */
void ClassifierWidget::adjustAssociations()
{
    NewUMLRectWidget::adjustAssociations();

    if (umlDoc()->loading() || m_classAssociationWidget == 0) {
        return;
    }

    m_classAssociationWidget->computeAssocClassLine();
}

/**
 * Reimplemented from NewUMLRectWidget::loadFromXMI to load the
 * classifier widget from XMI.
 */
bool ClassifierWidget::loadFromXMI(QDomElement & qElement)
{
    if (!NewUMLRectWidget::loadFromXMI(qElement))
        return false;

    // Determine str
    QString defaultSigType = QString::number(Uml::st_NoSig);

    QString showatts = qElement.attribute( "showattributes", "0" );
    QString showops = qElement.attribute( "showoperations", "1" );
    QString showpubliconly = qElement.attribute( "showpubliconly", "0" );
    QString showattsigs = qElement.attribute( "showattsigs", defaultSigType );
    QString showopsigs = qElement.attribute( "showopsigs", defaultSigType );
    QString showpackage = qElement.attribute( "showpackage", "0" );
    QString showscope = qElement.attribute( "showscope", "0" );
    QString drawascircle = qElement.attribute("drawascircle", "0");

    // Save scene and remove this item to prevent too many updates.
    UMLScene *saved = umlScene();
    if (saved) {
        saved->removeItem(this);
    }

    setVisualProperty(ShowAttributes, (bool)showatts.toInt());
    setVisualProperty(ShowOperations, (bool)showops.toInt());
    setVisualProperty(ShowPublicOnly, (bool)showpubliconly.toInt());
    setVisualProperty(ShowPackage, (bool)showpackage.toInt());
    setVisualProperty(ShowVisibility, (bool)showscope.toInt());
    setVisualProperty(DrawAsCircle, (bool)drawascircle.toInt());

    setAttributeSignature((Uml::Signature_Type)showattsigs.toInt());
    setOperationSignature((Uml::Signature_Type)showopsigs.toInt());

    // Now place this item back in the scene it belonged to.
    if (saved) {
        saved->addItem(this);
    }
    return true;
}

/**
 * Reimplemented from NewUMLRectWidget::saveToXMI to save
 * classifierwidget data either to 'interfacewidget' or 'classwidget'
 * XMI element.
 */
void ClassifierWidget::saveToXMI(QDomDocument & qDoc, QDomElement & qElement)
{
    QDomElement conceptElement;
    UMLClassifier *umlc = classifier();

    QString tagName = umlc->isInterface() ?
        "interfacewidget" : " classwidget";
    conceptElement = qDoc.createElement(tagName);
    NewUMLRectWidget::saveToXMI( qDoc, conceptElement );

    conceptElement.setAttribute("showoperations", visualProperty(ShowOperations));
    conceptElement.setAttribute("showpubliconly", visualProperty(ShowPublicOnly));
    conceptElement.setAttribute("showopsigs", m_operationSignatureType);
    conceptElement.setAttribute("showpackage", visualProperty(ShowPackage));
    conceptElement.setAttribute("showscope", visualProperty(ShowVisibility));

    if (! umlc->isInterface()) {
        conceptElement.setAttribute("showattributes", visualProperty(ShowAttributes));
        conceptElement.setAttribute("showattsigs", m_attributeSignatureType);
    }

    if (umlc->isInterface() || umlc->getAbstract()) {
        conceptElement.setAttribute("drawascircle", visualProperty(DrawAsCircle));
    }
    qElement.appendChild(conceptElement);
}

/**
 * Reimplemented form NewUMLRectWidget::paint to draw
 * ClassifierWidget.
 */
void ClassifierWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget *)
{
    QPen pen(lineColor(), lineWidth());
    painter->setPen(pen);
    painter->setBrush(brush());

    if (shouldDrawAsCircle()) {
        // m_classifierRect represents rectangle within which the
        // circle should be drawn. We have to calculate sane dimension
        // and position of this circle.
        qreal radius = qMin(m_classifierRect.width(), m_classifierRect.height());
        QRectF circle(0, 0, radius, radius);
        circle.moveCenter(m_classifierRect.center());

        painter->drawEllipse(circle);
    }
    else {
        // The elements not to be drawn will have null dimension and
        // hence it effectively is not drawn. (automatic ;) )
        painter->drawRect(m_classifierRect);
        painter->drawLines(m_classifierLines, 2);

        pen.setStyle(Qt::DotLine);
        painter->setPen(pen);
        painter->drawRect(m_templateRect);
    }
}

/**
 * Reimplemented from NewUMLRectWidget::updateGeometry to calculate
 * minimum size for this widget based on the current state of this
 * widget.
 */
void ClassifierWidget::updateGeometry()
{
    // Min size including classifier and template box.
    QSizeF totalMinSize;

    // Min size only for classifier items.
    QSizeF classifierMinSize = textItemGroupAt(ClassifierGroupIndex)->minimumSize();

    if (shouldDrawAsCircle()) {
        totalMinSize.setWidth(qMax(classifierMinSize.width(), ClassifierWidget::CircleMinimumRadius));
        totalMinSize.setHeight(classifierMinSize.height() + 2 * ClassifierWidget::CircleMinimumRadius);
        setMinimumSize(totalMinSize);
        return;
    }

    // Draw the bounding rectangle
    QSizeF templateBoxSize = textItemGroupAt(TemplateGroupIndex)->minimumSize();

    // If we don't have template params, templatesBoxSize is empty.
    if (templateBoxSize.isEmpty()) {
        totalMinSize = classifierMinSize;
        setMinimumSize(totalMinSize);
    }
    else {
        // Add margin to both classifier and template box sizes.
        QSizeF marginSize(2 * margin(), 2 * margin());
        classifierMinSize += marginSize;
        templateBoxSize += marginSize;

        totalMinSize.setHeight(classifierMinSize.height() + templateBoxSize.height() - margin());
        totalMinSize.setWidth(classifierMinSize.width() + .5 * templateBoxSize.width());

        // Dont add margin as we have already added manually
        setMinimumSize(totalMinSize, NewUMLRectWidget::DontAddMargin);
    }

    NewUMLRectWidget::updateGeometry();
}

/**
 * Reimplemented from NewUMLRectWidget::attributeChange to handle @ref
 * SizeHasChanged notification and pre-calculate the TextItem position
 * and the drawing elements like rectangle, lines ...
 */
QVariant ClassifierWidget::attributeChange(WidgetAttributeChange change, const QVariant& oldValue)
{
    if (change == SizeHasChanged) {
        calculateTemplateDrawing();
        calculateClassifierDrawing();
    }
    return NewUMLRectWidget::attributeChange(change, oldValue);
}

/**
 * Calculates the template box dimensions and the position of text
 * items inside it based on the current size.
 */
void ClassifierWidget::calculateTemplateDrawing()
{
    const qreal w = size().width();
    const qreal m = margin();

    TextItemGroup *templateGroup = textItemGroupAt(TemplateGroupIndex);
    QSizeF templateGroupSize = templateGroup->minimumSize();

    // No template box if no templates or drawing as circle.
    if (templateGroupSize.isEmpty() || shouldDrawAsCircle()) {
        m_templateRect = QRectF();
    }
    else {
        m_templateRect.setTop(0);
        m_templateRect.setLeft(w - templateGroupSize.width() - 2 * m);
        m_templateRect.setSize(templateGroupSize + QSizeF(2 * m, 2 * m)); // add margin

        templateGroup->setGroupGeometry(m_templateRect.adjusted(+m, +m, -m, -m));
    }
}

/**
 * Calculates the classifier box dimensions and the position of text
 * items inside it based on the current size.
 *
 * @note Assumes calculateTemplateDrawing is called before calling
 *       this method.
 */
void ClassifierWidget::calculateClassifierDrawing()
{
    const qreal w = size().width();
    const qreal h = size().height();
    const qreal m = margin();

    TextItemGroup *classifierGroup = textItemGroupAt(ClassifierGroupIndex);

    if (shouldDrawAsCircle()) {
        // Allocates circle space algined at top and "minimum space"
        // for the text which is aligned bottom.
        qreal fontHeight = classifierGroup->minimumSize().height();
        m_classifierRect = rect();
        m_classifierRect.setBottom(m_classifierRect.bottom() - fontHeight - 2 * m);

        QRectF groupRect(m, m_classifierRect.bottom() + m, w - 2 * m, fontHeight);
        classifierGroup->setGroupGeometry(groupRect);
        m_classifierLines[0] = m_classifierLines[1] = QLineF();
    }
    else {
        // Utilize entire space if template box is empty.
        if (m_templateRect.isEmpty()) {
            m_classifierRect = rect();
        }
        else {
            m_classifierRect.setTop(m_templateRect.bottom() - m);
            m_classifierRect.setLeft(0);
            m_classifierRect.setBottomRight(QPointF(m_templateRect.center().x(), h));
        }

        classifierGroup->setGroupGeometry(m_classifierRect.adjusted(+m, +m, -m, -m));
        const int cnt = classifierGroup->textItemCount();
        qreal expanderDistance = 4;
        if (cnt > m_lineItem1Index) {
            TextItem *item = classifierGroup->textItemAt(m_lineItem1Index);
            qreal y = item->mapToParent(item->boundingRect().bottomLeft()).y();
            m_classifierLines[0].setLine(m_classifierRect.left(), y, m_classifierRect.right(), y);
            qreal expanderX = rect().left() - m_attributeExpanderBox->rect().width() - expanderDistance;
            m_attributeExpanderBox->setPos(expanderX, y);
        }
        if (cnt > m_lineItem1Index) {
            TextItem *item = classifierGroup->textItemAt(m_lineItem2Index);
            qreal y = item->mapToParent(item->boundingRect().bottomLeft()).y();
            m_classifierLines[1].setLine(m_classifierRect.left(), y, m_classifierRect.right(), y);
            qreal expanderX = rect().left() - m_operationExpanderBox->rect().width() - expanderDistance;
            m_operationExpanderBox->setPos(expanderX, y);
        }
    }
    classifierGroup->setHoverBrush(QBrush(Qt::blue, Qt::Dense6Pattern));
}

/**
 * Reimplemented from NewUMLRectWidget::updateTextItemGroups to
 * calculate the Text strings , their properties and also hide/show
 * them based on the current state.
 */
void ClassifierWidget::updateTextItemGroups()
{
    // Invalidate stuff and recalculate them.
    invalidateDummies();

    TextItemGroup *classifierGroup = textItemGroupAt(ClassifierGroupIndex);
    TextItemGroup *templateGroup = textItemGroupAt(TemplateGroupIndex);
    UMLClassifier *umlC = classifier();
    UMLClassifierListItemList attribList = umlC->getFilteredList(Uml::ot_Attribute);
    UMLClassifierListItemList opList = umlC->getFilteredList(Uml::ot_Operation);

    // Set up template group and template text items.
    UMLTemplateList tlist = umlC->getTemplateList();
    templateGroup->setTextItemCount(tlist.size());
    bool templateHide = shouldDrawAsCircle(); // Hide if draw as circle.
    for(int i = 0; i < tlist.size(); ++i) {
        UMLTemplate *t = tlist[i];
        templateGroup->textItemAt(i)->setText(t->toString());
        templateGroup->textItemAt(i)->setVisible(!templateHide);
    }

    // +2 because we have NameItem and StereotypeItem
    const int cnt = attribList.count() + opList.count() + 2;
    classifierGroup->setTextItemCount(cnt);

    // Setup Stereo text item.
    TextItem *stereoItem = classifierGroup->textItemAt(StereotypeItemIndex);
    stereoItem->setBold(true);
    stereoItem->setText(umlC->getStereotype(true));

    bool v = !shouldDrawAsCircle()
        && visualProperty(ShowStereotype)
        && !(umlC->getStereotype(false).isEmpty());
    stereoItem->setVisible(v);

    // name item is always visible.
    TextItem *nameItem = classifierGroup->textItemAt(NameItemIndex);
    nameItem->setBold(true);
    nameItem->setItalic(umlC->getAbstract());
    nameItem->setUnderline(shouldDrawAsCircle());
    QString nameText = name();
    if (visualProperty(ShowPackage) == true) {
        nameText = umlC->getFullyQualifiedName();
    }

    bool showNameOnly = (!visualProperty(ShowAttributes) && !visualProperty(ShowOperations)
                         && !visualProperty(ShowStereotype) && !shouldDrawAsCircle());
    nameItem->setText(nameText);
    if (!showNameOnly) {
        m_lineItem1Index = NameItemIndex;
    }

    int attribStartIndex = NameItemIndex + 1;
    int opStartIndex = attribStartIndex + attribList.size();

    // Now setup attribute texts.
    for (int i=0; i < attribList.size(); ++i) {
        UMLClassifierListItem *obj = attribList[i];

        TextItem *item = classifierGroup->textItemAt(attribStartIndex + i);
        item->setItalic(obj->getAbstract());
        item->setUnderline(obj->getStatic());
        item->setText(obj->toString(m_attributeSignatureType));

        bool v = !shouldDrawAsCircle()
            && ( !visualProperty(ShowPublicOnly)
                 || obj->getVisibility() == Uml::Visibility::Public)
            && visualProperty(ShowAttributes) == true;

        item->setVisible(v);
    }
    // Update expander box to reflect current state and also visibility
    m_attributeExpanderBox->setExpanded(visualProperty(ShowAttributes));
    m_attributeExpanderBox->setVisible(!visualProperty(DrawAsCircle) && !umlC->isInterface());

    const QString dummyText;
    // Setup line and dummies.
    if (!showNameOnly) {
        // Stuff in a dummy item as spacer if there are no attributes,
        if (attribList.isEmpty() || !visualProperty(ShowAttributes)) {
            m_dummyAttributeItem = new TextItem(dummyText);
            classifierGroup->insertTextItemAt(attribStartIndex, m_dummyAttributeItem);
            m_lineItem2Index = attribStartIndex;
            ++opStartIndex;
        }
        else {
            // Now set the second index.
            m_lineItem2Index = opStartIndex - 1;
        }
    }

    for (int i=0; i < opList.size(); ++i) {
        UMLClassifierListItem *obj = opList[i];

        TextItem *item = classifierGroup->textItemAt(opStartIndex + i);
        item->setItalic(obj->getAbstract());
        item->setUnderline(obj->getStatic());
        item->setText(obj->toString(m_operationSignatureType));

        bool v = !shouldDrawAsCircle()
            && ( !visualProperty(ShowPublicOnly)
                 || obj->getVisibility() == Uml::Visibility::Public)
            && visualProperty(ShowOperations);

        item->setVisible(v);
    }
    m_operationExpanderBox->setExpanded(visualProperty(ShowOperations));
    m_operationExpanderBox->setVisible(!visualProperty(DrawAsCircle));

    if (!showNameOnly && opList.isEmpty()) {
        m_dummyOperationItem = new TextItem(dummyText);
        classifierGroup->insertTextItemAt(opStartIndex, m_dummyOperationItem);
    }

    NewUMLRectWidget::updateTextItemGroups();
}

/**
 * Determines the appropriate signature types for both operations and
 * attributes based on their current state and also based on access.
 */
void ClassifierWidget::updateSignatureTypes()
{
    //turn on scope
    if (visualProperty(ShowVisibility)) {
        // Take care of operation first
        if (m_operationSignatureType == Uml::st_NoSigNoVis) {
            m_operationSignatureType = Uml::st_NoSig;
        } else if (m_operationSignatureType == Uml::st_SigNoVis) {
            m_operationSignatureType = Uml::st_ShowSig;
        }
        // Now take care of attributes
        if (m_attributeSignatureType == Uml::st_NoSigNoVis)
            m_attributeSignatureType = Uml::st_NoSig;
        else if (m_attributeSignatureType == Uml::st_SigNoVis)
            m_attributeSignatureType = Uml::st_ShowSig;

    }
    //turn off scope
    else {
        // Take care of operations first
        if (m_operationSignatureType == Uml::st_ShowSig) {
            m_operationSignatureType = Uml::st_SigNoVis;
        } else if (m_operationSignatureType == Uml::st_NoSig) {
            m_operationSignatureType = Uml::st_NoSigNoVis;
        }
        // Now take care of attributes.
        if (m_attributeSignatureType == Uml::st_ShowSig)
            m_attributeSignatureType = Uml::st_SigNoVis;
        else if(m_attributeSignatureType == Uml::st_NoSig)
            m_attributeSignatureType = Uml::st_NoSigNoVis;
    }

    updateTextItemGroups();
}

void ClassifierWidget::slotMenuSelection(QAction* action)
{
    // The menu is passed in as parent of action.
    ListPopupMenu *menu = qobject_cast<ListPopupMenu*>(action->parent());
    ListPopupMenu::Menu_Type sel = menu->getMenuType(action);
    switch (sel) {
    case ListPopupMenu::mt_Attribute:
    case ListPopupMenu::mt_Operation:
    case ListPopupMenu::mt_Template:
    {
        Uml::Object_Type ot = ListPopupMenu::convert_MT_OT(sel);
        if (Object_Factory::createChildObject(classifier(), ot)) {
            UMLApp::app()->getDocument()->setModified();
        }
        break;
    }
    case ListPopupMenu::mt_Show_Operations:
    case ListPopupMenu::mt_Show_Operations_Selection:
        toggleVisualProperty(ShowOperations);
        break;

    case ListPopupMenu::mt_Show_Attributes:
    case ListPopupMenu::mt_Show_Attributes_Selection:
        toggleVisualProperty(ShowAttributes);
        break;

    case ListPopupMenu::mt_Show_Public_Only:
    case ListPopupMenu::mt_Show_Public_Only_Selection:
        toggleVisualProperty(ShowPublicOnly);
        break;

    case ListPopupMenu::mt_Show_Operation_Signature:
    case ListPopupMenu::mt_Show_Operation_Signature_Selection:
        toggleVisualProperty(ShowOperationSignature);
        break;

    case ListPopupMenu::mt_Show_Attribute_Signature:
    case ListPopupMenu::mt_Show_Attribute_Signature_Selection:
        toggleVisualProperty(ShowAttributeSignature);
        break;

    case ListPopupMenu::mt_Visibility:
    case ListPopupMenu::mt_Visibility_Selection:
        toggleVisualProperty(ShowVisibility);
        break;

    case ListPopupMenu::mt_Show_Packages:
    case ListPopupMenu::mt_Show_Packages_Selection:
        toggleVisualProperty(ShowPackage);
        break;

    case ListPopupMenu::mt_Show_Stereotypes:
    case ListPopupMenu::mt_Show_Stereotypes_Selection:
        toggleVisualProperty(ShowStereotype);
        break;

    case ListPopupMenu::mt_DrawAsCircle:
    case ListPopupMenu::mt_DrawAsCircle_Selection:
        toggleVisualProperty(DrawAsCircle);
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
        NewUMLRectWidget::slotMenuSelection(action);
        break;
    }
}

/// Slot to show/hide attributes based on \a state.
void ClassifierWidget::slotShowAttributes(bool state)
{
    setVisualProperty(ShowAttributes, state);
}

/// Slot to show/hide operations based on \a state.
void ClassifierWidget::slotShowOperations(bool state)
{
    setVisualProperty(ShowOperations, state);
}

/**
 * Invalidates all dummies used so that they can be recalculated again.
 */
void ClassifierWidget::invalidateDummies()
{
    TextItemGroup *grp = textItemGroupAt(ClassifierGroupIndex);
    if (m_dummyAttributeItem) {
        grp->deleteTextItem(m_dummyAttributeItem);
        m_dummyAttributeItem = 0;
    }

    if (m_dummyOperationItem) {
        grp->deleteTextItem(m_dummyOperationItem);
        m_dummyOperationItem = 0;
    }

    m_lineItem1Index = m_lineItem2Index = InvalidIndex;
    m_classifierLines[0] = m_classifierLines[1] = QLineF();
}
