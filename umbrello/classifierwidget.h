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

#ifndef CLASSIFIERWIDGET_H
#define CLASSIFIERWIDGET_H

#include "newumlrectwidget.h"

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
 * @see NewUMLRectWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ClassifierWidget : public NewUMLRectWidget
{
    Q_OBJECT;
public:
    /**
     * This enumeration lists the visual properties that can be easily
     * set, reset and toggled and all these operate on an integer
     * which stores all the flag status.
     */
    enum VisualProperty {
        ShowStereotype = 0x1,
        ShowOperations = 0x2,
        ShowPublicOnly = 0x4,
        ShowVisibility = 0x8,
        ShowPackage = 0x10,
        ShowAttributes = 0x20,
        DrawAsCircle = 0x40,

        // These two are effective only in setters.
        ShowOperationSignature = 0x60,
        ShowAttributeSignature = 0x80
    };

    Q_DECLARE_FLAGS(VisualProperties, VisualProperty);

    ClassifierWidget(UMLClassifier * o);
    virtual ~ClassifierWidget();

    UMLClassifier *classifier() const;

    /**
     * @return An OR combination of all VisualProperty values
     *         indicating current visual property status.
     */
    VisualProperties visualProperties() const {
        return m_visualProperties;
    }
    void setVisualProperties(VisualProperties properties);

    bool visualProperty(VisualProperty property) const;
    void setVisualProperty(VisualProperty property, bool enable = true);
    void toggleVisualProperty(VisualProperty property);

    bool shouldDrawAsCircle() const;

    /// @return The Uml::Signature_Type value for the attributes.
    Uml::Signature_Type attributeSignatureType() const {
        return m_attributeSignatureType;
    }
    void setAttributeSignature(Uml::Signature_Type sig);

    /// @return The Uml::Signature_Type value for the operations.
    Uml::Signature_Type operationSignatureType() const {
        return m_operationSignatureType;
    }
    void setOperationSignature(Uml::Signature_Type sig);

    void changeToClass();
    void changeToInterface();

    /**
     * @return The AssociationWidget when this classifier acts as an
     *         association class (else return NULL.)
     */
    AssociationWidget *classAssociationWidget() const {
        return m_classAssociationWidget;
    }
    void setClassAssociationWidget(AssociationWidget *assocwidget);
    virtual void adjustAssociations();

    virtual bool loadFromXMI(QDomElement & qElement);
    virtual void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);

    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *);

protected:
    virtual void updateGeometry();
    virtual void updateTextItemGroups();
    virtual QVariant attributeChange(WidgetAttributeChange change, const QVariant& oldValue);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction* action);

private Q_SLOTS:
    void slotShowAttributes(bool state);
    void slotShowOperations(bool state);

private:
    void updateSignatureTypes();
    void calculateTemplateDrawing();
    void calculateClassifierDrawing();

    VisualProperties m_visualProperties;
    Uml::Signature_Type m_attributeSignatureType; ///< Loaded/saved item.
    Uml::Signature_Type m_operationSignatureType;  ///< Loaded/saved item.

    /**
     * Size of circle when interface is rendered as such
     */
    static const qreal CircleMinimumRadius;

    /**
     * The related AssociationWidget in case this classifier acts as
     * an association class
     */
    AssociationWidget *m_classAssociationWidget;

    enum {
        ClassifierGroupIndex=0,
        TemplateGroupIndex
    };
    enum {
        StereotypeItemIndex=0,
        NameItemIndex
    };

    QRectF m_classifierRect;
    QRectF m_templateRect;
    QLineF m_classifierLines[2];

    void invalidateDummies();

    int m_lineItem1Index;
    int m_lineItem2Index;
    TextItem *m_dummyAttributeItem;
    TextItem *m_dummyOperationItem;

    static const int InvalidIndex;

    ExpanderBox *m_attributeExpanderBox;
    ExpanderBox *m_operationExpanderBox;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ClassifierWidget::VisualProperties)

#endif
