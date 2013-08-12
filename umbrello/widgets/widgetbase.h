/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef WIDGETBASE_H
#define WIDGETBASE_H

#include "basictypes.h"

#include <QColor>
#include <QDomDocument>
#include <QFont>
#include <QGraphicsObject>
#include <QObject>
#include <QPainter>

// forward declarations
class ListPopupMenu;
class QAction;
class UMLDoc;
class UMLObject;
class UMLScene;

/**
 * @short       Common base class for UMLWidget and AssociationWidget
 * @author      Oliver Kellogg <okellogg@users.sourceforge.net>
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class WidgetBase : public QGraphicsObject
{
    Q_OBJECT
    Q_ENUMS(WidgetType)

public:
    enum WidgetType
    {
        wt_UMLWidget = 300,         // does not have UMLObject representation
        wt_Actor,                   // has UMLObject representation
        wt_UseCase,                 // has UMLObject representation
        wt_Class,                   // has UMLObject representation
        wt_Interface,               // has UMLObject representation
        wt_Datatype,                // has UMLObject representation
        wt_Enum,                    // has UMLObject representation
        wt_Entity,                  // has UMLObject representation
        wt_Package,                 // has UMLObject representation
        wt_Object,                  // has UMLObject representation
        wt_Note,                    // does not have UMLObject representation
        wt_Box,                     // does not have UMLObject representation
        wt_Message,                 // does not have UMLObject representation
        wt_Text,                    // does not have UMLObject representation
        wt_State,                   // does not have UMLObject representation
        wt_Activity,                // does not have UMLObject representation
        wt_Component,               // has UMLObject representation
        wt_Artifact,                // has UMLObject representation
        wt_Node,                    // has UMLObject representation
        wt_Association,             // has UMLObject representation
        wt_ForkJoin,                // does not have UMLObject representation
        wt_Precondition,            // does not have UMLObject representation
        wt_CombinedFragment,        // does not have UMLObject representation
        wt_FloatingDashLine,        // does not have UMLObject representation
        wt_Signal,                  // does not have UMLObject representation
        wt_Pin,
        wt_ObjectNode,
        wt_Region,
        wt_Category                 // has UMLObject representation
    };

    static QString toString(WidgetType wt);

    explicit WidgetBase(UMLScene * scene, WidgetType type= wt_UMLWidget);
    virtual ~WidgetBase();

    UMLObject* umlObject() const;
    virtual void setUMLObject(UMLObject *obj);

    Uml::ID::Type id() const;
    void setID(Uml::ID::Type id);

    WidgetType baseType() const;
    QLatin1String baseTypeStr() const;

    UMLScene* umlScene() const;
    UMLDoc* umlDoc() const;

    QString documentation() const;
    void setDocumentation(const QString& doc);

    QString name() const;
    void setName(const QString &strName);

    QColor lineColor() const;
    virtual void setLineColor(const QColor& color);

    uint lineWidth() const;
    virtual void setLineWidth(uint width);

    QColor textColor() const;
    virtual void setTextColor(const QColor& color);

    QColor fillColor() const;
    virtual void setFillColor(const QColor& color);

    bool usesDiagramLineColor() const;
    void setUsesDiagramLineColor(bool state);

    bool usesDiagramLineWidth() const;
    void setUsesDiagramLineWidth(bool state);

    bool useFillColor();
    void setUseFillColor(bool state);

    bool usesDiagramTextColor() const;
    void setUsesDiagramTextColor(bool state);

    bool usesDiagramFillColor() const;
    void setUsesDiagramFillColor(bool state);

    bool usesDiagramUseFillColor() const;
    void setUsesDiagramUseFillColor(bool state);

    QFont font() const;
    void setFont(const QFont& font);

    virtual void showPropertiesDialog();

    virtual bool loadFromXMI(QDomElement &qElement);
    virtual void saveToXMI(QDomDocument &qDoc, QDomElement &qElement);

    WidgetBase& operator=(const WidgetBase& other);

    QRectF rect() const;
    void setRect(const QRectF& rect);
    void setRect(qreal x, qreal y, qreal width, qreal height);

    virtual QRectF boundingRect() const;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

public Q_SLOTS:
    virtual void slotMenuSelection(QAction *trigger);

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void setupContextMenuActions(ListPopupMenu &menu);

    WidgetType  m_baseType;  ///< Type of widget.
    UMLScene   *m_scene;
    UMLObject  *m_umlObject;
    QString     m_Doc;   ///< Only used if m_umlObject is not set.
    QString     m_Text;
    QRectF      m_rect;  ///< widget size

    /**
     * This ID is only used when the widget does not have a
     * corresponding UMLObject (i.e. the m_umlObject pointer is NULL.)
     * For UMLObjects, the ID from the UMLObject is used.
     */
    Uml::ID::Type m_nId;

    QColor m_textColor;  ///< Color of the text of the widget. Is saved to XMI.
    QColor m_lineColor;  ///< Color of the lines of the widget. Is saved to XMI.
    QColor m_fillColor;  ///< color of the background of the widget
    QBrush m_brush;
    QFont  m_font;
    uint   m_lineWidth;  ///< Width of the lines of the widget. Is saved to XMI.
    bool   m_useFillColor;  ///< flag indicates if the UMLWidget uses the Diagram FillColour

    /**
     * true by default, false if the colors have
     * been explicitly set for this widget.
     * These are saved to XMI.
     */
    bool m_usesDiagramTextColor;
    bool m_usesDiagramLineColor;
    bool m_usesDiagramFillColor;
    bool m_usesDiagramUseFillColor;
    bool m_usesDiagramLineWidth;
};

#endif
