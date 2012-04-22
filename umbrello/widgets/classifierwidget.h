/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CLASSIFIERWIDGET_H
#define CLASSIFIERWIDGET_H

#include "basictypes.h"
#include "umlwidget.h"

class AssociationWidget;
class ExpanderBox;
class TextItem;
class UMLClassifier;

/**
 * @short Common implementation for class widget and interface widget
 *
 * @author Oliver Kellogg
 * @author Gopala Krishna
 *
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ClassifierWidget : public UMLWidget
{
    Q_OBJECT
    Q_ENUMS(VisualProperty)
public:
    /**
     * This enumeration lists the visual properties that can be easily
     * set, reset and toggled and all these operate on an integer
     * which stores all the flag status.
     */
    enum VisualProperty {
        ShowStereotype         = 0x1,
        ShowOperations         = 0x2,
        ShowPublicOnly         = 0x4,
        ShowVisibility         = 0x8,
        ShowPackage            = 0x10,
        ShowAttributes         = 0x20,
        DrawAsCircle           = 0x40,
        ShowOperationSignature = 0x60,  ///< only in setter
        ShowAttributeSignature = 0x80   ///< only in setter
    };

    Q_DECLARE_FLAGS(VisualProperties, VisualProperty)

    ClassifierWidget(UMLClassifier * o);
    virtual ~ClassifierWidget();

    UMLClassifier *classifier() const;

    VisualProperties visualProperties() const;
    void setVisualProperties(VisualProperties properties);

    bool visualProperty(VisualProperty property) const;
    void setVisualProperty(VisualProperty property, bool enable = true);
    void toggleVisualProperty(VisualProperty property);

    bool shouldDrawAsCircle() const;

    Uml::SignatureType attributeSignature() const;
    void setAttributeSignature(Uml::SignatureType sig);

    Uml::SignatureType operationSignature() const;
    void setOperationSignature(Uml::SignatureType sig);

    void changeToClass();
    void changeToInterface();

    AssociationWidget *classAssociationWidget() const;
    void setClassAssociationWidget(AssociationWidget *assocwidget);
    virtual void adjustAssociations(bool userAdjustChange);

    virtual void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);
    virtual bool loadFromXMI(QDomElement & qElement);

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

protected:
    virtual void updateGeometry();
    virtual void updateTextItemGroups();
    virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);

    virtual void hoverEnterEvent(UMLSceneHoverEvent * event);
    virtual void hoverLeaveEvent(UMLSceneHoverEvent * event);

private Q_SLOTS:
    void slotShowAttributes(bool state);
    void slotShowOperations(bool state);

private:
    void updateSignatureTypes();
    void calculateTemplateDrawing();
    void calculateClassifierDrawing();
    void invalidateDummies();

    VisualProperties   m_visualProperties;
    Uml::SignatureType m_attributeSignature;  ///< Loaded/saved item.
    Uml::SignatureType m_operationSignature;  ///< Loaded/saved item.
    AssociationWidget *m_classAssociationWidget;  ///< related AssociationWidget in case this classifier acts as an association class

    /**
     * Size of circle when interface is rendered as such
     */
    static const qreal CircleMinimumRadius;

    enum {
        HeaderGroupIndex=0,
        AttribOpGroupIndex,
        TemplateGroupIndex
    };
    enum {
        StereotypeItemIndex=0,
        NameItemIndex
    };

    QRectF m_classifierRect;
    QRectF m_templateRect;
    QLineF m_classifierLines[2];

    int m_lineItem2Index;
    TextItem *m_dummyAttributeItem;
    TextItem *m_dummyOperationItem;

    static const int InvalidIndex;

    ExpanderBox *m_attributeExpanderBox;
    ExpanderBox *m_operationExpanderBox;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ClassifierWidget::VisualProperties)

#endif
