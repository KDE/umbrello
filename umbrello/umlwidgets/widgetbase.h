/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef WIDGETBASE_H
#define WIDGETBASE_H

#include "basictypes.h"
#include "icon_utils.h"

#include <QColor>
#include <QDomDocument>
#include <QFont>
#include <QGraphicsObject>
#include <QObject>
#include <QPainter>
#include <QPointer>

// forward declarations
class QAction;
class ActivityWidget;
class ActorWidget;
class ArtifactWidget;
class AssociationWidget;
class BoxWidget;
class CategoryWidget;
class ClassifierWidget;
class CombinedFragmentWidget;
class ComponentWidget;
class DatatypeWidget;
class EntityWidget;
class EnumWidget;
class FloatingDashLineWidget;
class FloatingTextWidget;
class ForkJoinWidget;
class InterfaceWidget;
class MessageWidget;
class NodeWidget;
class NoteWidget;
class ObjectNodeWidget;
class ObjectWidget;
class PackageWidget;
class PinWidget;
class PortWidget;
class PinPortBase;
class PreconditionWidget;
class RegionWidget;
class SignalWidget;
class StateWidget;
//class TextWidget;
class UseCaseWidget;
class UMLDoc;
class UMLObject;
class UMLScene;
class UMLWidget;   // required by function onWidget()

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
        wt_Min = 299,               // lower bounds check value
        wt_UMLWidget,               // does not have UMLObject representation
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
        wt_Category,                // has UMLObject representation
        wt_Port,                    // has UMLObject representation
        wt_Instance,                // has UMLObject representation == wt_Object
        wt_Max                      // upper bounds check value
    };

    static QString toString(WidgetType wt);
    static QString toI18nString(WidgetType wt);
    static Icon_Utils::IconType toIcon(WidgetType wt);

    explicit WidgetBase(UMLScene * scene, WidgetType type= wt_UMLWidget);
    virtual ~WidgetBase();

    UMLObject* umlObject() const;
    virtual void setUMLObject(UMLObject *obj);

    Uml::ID::Type id() const;
    void setID(Uml::ID::Type id);

    void setLocalID(Uml::ID::Type id);
    Uml::ID::Type localID() const;

    virtual UMLWidget *widgetWithID(Uml::ID::Type id);

    WidgetType baseType() const;
    void setBaseType(const WidgetType& baseType);
    QLatin1String baseTypeStr() const;
    QString baseTypeStrWithoutPrefix() const;

    virtual void setSelected(bool select);

    UMLScene* umlScene() const;
    UMLDoc* umlDoc() const;

    QString documentation() const;
    bool hasDocumentation();
    virtual void setDocumentation(const QString& doc);

    QString name() const;
    virtual void setName(const QString &strName);

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
    virtual void setUseFillColor(bool state);

    bool usesDiagramTextColor() const;
    void setUsesDiagramTextColor(bool state);

    bool usesDiagramFillColor() const;
    void setUsesDiagramFillColor(bool state);

    bool usesDiagramUseFillColor() const;
    void setUsesDiagramUseFillColor(bool state);

    virtual QFont font() const;
    virtual void setFont(const QFont& font);

    bool autoResize();
    void setAutoResize(bool state);

    bool changesShape();
    void setChangesShape(bool state);

    virtual bool showPropertiesDialog();

    virtual bool loadFromXMI1(QDomElement &qElement);
    virtual void saveToXMI1(QDomDocument &qDoc, QDomElement &qElement);

    virtual void removeAssoc(AssociationWidget* pAssoc);
    virtual void addAssoc(AssociationWidget* pAssoc);

    WidgetBase& operator=(const WidgetBase& other);

    QRectF rect() const;
    void setRect(const QRectF& rect);
    void setRect(qreal x, qreal y, qreal width, qreal height);

    virtual QRectF boundingRect() const;

    virtual UMLWidget* onWidget(const QPointF &p);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    bool isActivityWidget()         { return baseType() == wt_Activity; }
    bool isActorWidget()            { return baseType() == wt_Actor; }
    bool isArtifactWidget()         { return baseType() == wt_Artifact; }
    bool isAssociationWidget()      { return baseType() == wt_Association; }
    bool isBoxWidget()              { return baseType() == wt_Box; }
    bool isCategoryWidget()         { return baseType() == wt_Category; }
    bool isClassWidget()            { return baseType() == wt_Class; }
    bool isCombinedFragmentWidget() { return baseType() == wt_CombinedFragment; }
    bool isComponentWidget()        { return baseType() == wt_Component; }
    bool isDatatypeWidget()         { return baseType() == wt_Datatype; }
    bool isEntityWidget()           { return baseType() == wt_Entity; }
    bool isEnumWidget()             { return baseType() == wt_Enum; }
    bool isFloatingDashLineWidget() { return baseType() == wt_FloatingDashLine; }
    bool isForkJoinWidget()         { return baseType() == wt_ForkJoin; }
    bool isInterfaceWidget()        { return baseType() == wt_Interface; }
    bool isMessageWidget()          { return baseType() == wt_Message; }
    bool isNodeWidget()             { return baseType() == wt_Node; }
    bool isNoteWidget()             { return baseType() == wt_Note; }
    bool isObjectNodeWidget()       { return baseType() == wt_ObjectNode; }
    bool isObjectWidget()           { return baseType() == wt_Object; }
    bool isPackageWidget()          { return baseType() == wt_Package; }
    bool isPinWidget()              { return baseType() == wt_Pin; }
    bool isPortWidget()             { return baseType() == wt_Port; }
    bool isPreconditionWidget()     { return baseType() == wt_Precondition; }
    bool isRegionWidget()           { return baseType() == wt_Region; }
    bool isSignalWidget()           { return baseType() == wt_Signal; }
    bool isStateWidget()            { return baseType() == wt_State; }
    bool isTextWidget()             { return baseType() == wt_Text; }
    bool isUseCaseWidget()          { return baseType() == wt_UseCase; }

    ActivityWidget* asActivityWidget();
    ActorWidget* asActorWidget();
    ArtifactWidget* asArtifactWidget();
    AssociationWidget* asAssociationWidget();
    BoxWidget* asBoxWidget();
    CategoryWidget* asCategoryWidget();
    ClassifierWidget* asClassifierWidget();
    CombinedFragmentWidget* asCombinedFragmentWidget();
    ComponentWidget* asComponentWidget();
    DatatypeWidget* asDatatypeWidget();
    EntityWidget* asEntityWidget();
    EnumWidget* asEnumWidget();
    FloatingDashLineWidget* asFloatingDashLineWidget();
    ForkJoinWidget* asForkJoinWidget();
    InterfaceWidget* asInterfaceWidget();
    MessageWidget* asMessageWidget();
    NodeWidget* asNodeWidget();
    NoteWidget* asNoteWidget();
    ObjectNodeWidget* asObjectNodeWidget();
    ObjectWidget* asObjectWidget();
    PackageWidget* asPackageWidget();
    PinWidget* asPinWidget();
    PinPortBase* asPinPortBase();
    PortWidget* asPortWidget();
    PreconditionWidget* asPreconditionWidget();
    RegionWidget* asRegionWidget();
    SignalWidget* asSignalWidget();
    StateWidget* asStateWidget();
    FloatingTextWidget* asFloatingTextWidget();
//    TextWidget* asTextWidget();
    UseCaseWidget* asUseCaseWidget();
    UMLWidget* asUMLWidget();

public Q_SLOTS:
    virtual void slotMenuSelection(QAction *trigger);

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
    WidgetType  m_baseType;  ///< Type of widget.
protected:
    UMLScene   *m_scene;
    QPointer<UMLObject> m_umlObject;
    QString     m_Doc;   ///< Only used if m_umlObject is not set.
    QString     m_Text;
    QRectF      m_rect;  ///< widget size

    /**
     * This ID is only used when the widget does not have a
     * corresponding UMLObject (i.e. the m_umlObject pointer is NULL.)
     * For UMLObjects, the ID from the UMLObject is used.
     */
    Uml::ID::Type m_nId;

    /**
     * This ID is only used when a widget could be added more then one time to a diagram
     */
    Uml::ID::Type m_nLocalID;

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
    bool m_usesDiagramFillColor;
    bool m_usesDiagramLineColor;
    bool m_usesDiagramLineWidth;
    bool m_usesDiagramTextColor;
    bool m_usesDiagramUseFillColor;
    bool m_autoResize;
    bool m_changesShape; ///< The widget changes its shape when the number of connections or their positions are changed
};

#endif
