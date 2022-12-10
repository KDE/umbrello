/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
#include <QXmlStreamWriter>

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
class IDChangeLog;
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
 * Provides a wrapper that bypasses the restriction that
 * QGraphicsItem::setSelected() is not virtual
 *
 * The selection management of umbrello uses a virtual method
 * setSelected() for selection to achieve the desired behavior
 * in the different derivation levels regarding selection and
 * deselection.
 *
 * Within QGraphicsScene, QGraphicsItem::setSelected() is called
 * to manage the selection state, e.g. with clearSelection(), but
 * unfortunately cannot be directly overwritten by umbrello because
 * this method is not virtual (I consider this a design flaw).
 *
 * Fortunately there is a workaround for the problem by using
 * QGraphicsItem::itemChange(), which is overridden in this class
 * and calls the (now) virtual method setSelected() when the selection
 * state changes. This calls derived implementations of this method
 * and realizes the desired behavior.
 *
 * Within setSelected() you have to take care that
 * QGraphicsObject::setSelected() is not called if the call came
 * from itemChange() to avoid an endless loop.
 *
 * @author Ralf Habacker <ralf.habacker@freenet.de>
 */
class QGraphicsObjectWrapper: public QGraphicsObject
{
public:
    virtual void setSelected(bool state);
protected:
    bool m_calledFromItemChange{false};
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
};

/**
 * @short       Common base class for UMLWidget and AssociationWidget
 * @author      Oliver Kellogg <okellogg@users.sourceforge.net>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class WidgetBase : public QGraphicsObjectWrapper
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

    explicit WidgetBase(UMLScene * scene, WidgetType type= wt_UMLWidget, Uml::ID::Type id = Uml::ID::None);
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
    QStringLiteral baseTypeStr() const;
    QString baseTypeStrWithoutPrefix() const;

    virtual void setSelected(bool select);

    UMLScene* umlScene() const;
    UMLDoc* umlDoc() const;

    QString documentation() const;
    bool hasDocumentation() const;
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

    bool useFillColor() const;
    virtual void setUseFillColor(bool state);

    bool usesDiagramTextColor() const;
    void setUsesDiagramTextColor(bool state);

    bool usesDiagramFillColor() const;
    void setUsesDiagramFillColor(bool state);

    bool usesDiagramUseFillColor() const;
    void setUsesDiagramUseFillColor(bool state);

    virtual QFont font() const;
    virtual void setFont(const QFont& font);

    bool autoResize() const;
    void setAutoResize(bool state);

    bool changesShape() const;
    void setChangesShape(bool state);

    virtual bool showPropertiesDialog();

    virtual bool loadFromXMI(QDomElement &qElement);
    virtual void saveToXMI(QXmlStreamWriter& writer);

    virtual void removeAssoc(AssociationWidget* pAssoc);
    virtual void addAssoc(AssociationWidget* pAssoc);

    WidgetBase& operator=(const WidgetBase& other);

    QRectF rect() const;
    void setRect(const QRectF& rect);
    void setRect(qreal x, qreal y, qreal width, qreal height);

    virtual QRectF boundingRect() const;

    virtual UMLWidget* onWidget(const QPointF &p);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    bool isActivityWidget()         const { return baseType() == wt_Activity; }
    bool isActorWidget()            const { return baseType() == wt_Actor; }
    bool isArtifactWidget()         const { return baseType() == wt_Artifact; }
    bool isAssociationWidget()      const { return baseType() == wt_Association; }
    bool isBoxWidget()              const { return baseType() == wt_Box; }
    bool isCategoryWidget()         const { return baseType() == wt_Category; }
    bool isClassWidget()            const { return baseType() == wt_Class; }
    bool isCombinedFragmentWidget() const { return baseType() == wt_CombinedFragment; }
    bool isComponentWidget()        const { return baseType() == wt_Component; }
    bool isDatatypeWidget()         const { return baseType() == wt_Datatype; }
    bool isEntityWidget()           const { return baseType() == wt_Entity; }
    bool isEnumWidget()             const { return baseType() == wt_Enum; }
    bool isFloatingDashLineWidget() const { return baseType() == wt_FloatingDashLine; }
    bool isForkJoinWidget()         const { return baseType() == wt_ForkJoin; }
    bool isInterfaceWidget()        const { return baseType() == wt_Interface; }
    bool isMessageWidget()          const { return baseType() == wt_Message; }
    bool isNodeWidget()             const { return baseType() == wt_Node; }
    bool isNoteWidget()             const { return baseType() == wt_Note; }
    bool isObjectNodeWidget()       const { return baseType() == wt_ObjectNode; }
    bool isObjectWidget()           const { return baseType() == wt_Object; }
    bool isPackageWidget()          const { return baseType() == wt_Package; }
    bool isPinWidget()              const { return baseType() == wt_Pin; }
    bool isPortWidget()             const { return baseType() == wt_Port; }
    bool isPreconditionWidget()     const { return baseType() == wt_Precondition; }
    bool isRegionWidget()           const { return baseType() == wt_Region; }
    bool isSignalWidget()           const { return baseType() == wt_Signal; }
    bool isStateWidget()            const { return baseType() == wt_State; }
    bool isTextWidget()             const { return baseType() == wt_Text; }
    bool isUseCaseWidget()          const { return baseType() == wt_UseCase; }

    ActivityWidget*         asActivityWidget();
    ActorWidget*            asActorWidget();
    ArtifactWidget*         asArtifactWidget();
    AssociationWidget*      asAssociationWidget();
    BoxWidget*              asBoxWidget();
    CategoryWidget*         asCategoryWidget();
    ClassifierWidget*       asClassifierWidget();
    CombinedFragmentWidget* asCombinedFragmentWidget();
    ComponentWidget*        asComponentWidget();
    DatatypeWidget*         asDatatypeWidget();
    EntityWidget*           asEntityWidget();
    EnumWidget*             asEnumWidget();
    FloatingDashLineWidget* asFloatingDashLineWidget();
    ForkJoinWidget*         asForkJoinWidget();
    InterfaceWidget*        asInterfaceWidget();
    MessageWidget*          asMessageWidget();
    NodeWidget*             asNodeWidget();
    NoteWidget*             asNoteWidget();
    ObjectNodeWidget*       asObjectNodeWidget();
    ObjectWidget*           asObjectWidget();
    PackageWidget*          asPackageWidget();
    PinWidget*              asPinWidget();
    PinPortBase*            asPinPortBase();
    PortWidget*             asPortWidget();
    PreconditionWidget*     asPreconditionWidget();
    RegionWidget*           asRegionWidget();
    SignalWidget*           asSignalWidget();
    StateWidget*            asStateWidget();
    FloatingTextWidget*     asFloatingTextWidget();
//    TextWidget*               asTextWidget();
    UseCaseWidget*          asUseCaseWidget();
    UMLWidget*              asUMLWidget();

    const ActivityWidget*         asActivityWidget()         const;
    const ActorWidget*            asActorWidget()            const;
    const ArtifactWidget*         asArtifactWidget()         const;
    const AssociationWidget*      asAssociationWidget()      const;
    const BoxWidget*              asBoxWidget()              const;
    const CategoryWidget*         asCategoryWidget()         const;
    const ClassifierWidget*       asClassifierWidget()       const;
    const CombinedFragmentWidget* asCombinedFragmentWidget() const;
    const ComponentWidget*        asComponentWidget()        const;
    const DatatypeWidget*         asDatatypeWidget()         const;
    const EntityWidget*           asEntityWidget()           const;
    const EnumWidget*             asEnumWidget()             const;
    const FloatingDashLineWidget* asFloatingDashLineWidget() const;
    const ForkJoinWidget*         asForkJoinWidget()         const;
    const InterfaceWidget*        asInterfaceWidget()        const;
    const MessageWidget*          asMessageWidget()          const;
    const NodeWidget*             asNodeWidget()             const;
    const NoteWidget*             asNoteWidget()             const;
    const ObjectNodeWidget*       asObjectNodeWidget()       const;
    const ObjectWidget*           asObjectWidget()           const;
    const PackageWidget*          asPackageWidget()          const;
    const PinWidget*              asPinWidget()              const;
    const PinPortBase*            asPinPortBase()            const;
    const PortWidget*             asPortWidget()             const;
    const PreconditionWidget*     asPreconditionWidget()     const;
    const RegionWidget*           asRegionWidget()           const;
    const SignalWidget*           asSignalWidget()           const;
    const StateWidget*            asStateWidget()            const;
    const FloatingTextWidget*     asFloatingTextWidget()     const;
//    const TextWidget*               asTextWidget()             const;
    const UseCaseWidget*          asUseCaseWidget()          const;
    const UMLWidget*              asUMLWidget()              const;

    static bool widgetHasUMLObject(WidgetBase::WidgetType type);
    virtual bool activate(IDChangeLog *changeLog = 0);

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
    QRectF      m_rect;  ///< Widget size. In this rectangle, the x and y coordinates are always 0.

    /**
     * This ID is only used when the widget does not have a
     * corresponding UMLObject (i.e. the m_umlObject pointer is NULL.)
     * For UMLObjects, the ID from the UMLObject is used.
     */
    Uml::ID::Type m_nId;

    /**
     * This ID is only used when a widget could be added more than once to a diagram
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
