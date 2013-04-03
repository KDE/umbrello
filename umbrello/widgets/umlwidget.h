/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLWIDGET_H
#define UMLWIDGET_H

#include "associationwidgetlist.h"
#include "basictypes.h"
#include "optionstate.h"
#include "umlscene.h"
#include "widgetbase.h"

#include <QDateTime>
#include <QFont>

class UMLWidgetController;

class UMLObject;
class UMLDoc;
class ListPopupMenu;
class IDChangeLog;

class QPainter;
class QMoveEvent;
class QFontMetrics;

/**
 * This is the base class for nearly all graphical widgets.
 *
 * @short The base class for graphical UML objects.
 * @author  Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLWidget : public WidgetBase
{
    Q_OBJECT
public:
    static const UMLSceneSize DefaultMinimumSize;
    static const UMLSceneSize DefaultMaximumSize;

    friend class UMLWidgetController;

    explicit UMLWidget(UMLScene * scene, WidgetType type = wt_UMLWidget, UMLObject * o = 0, UMLWidgetController *widgetController = 0);
    explicit UMLWidget(UMLScene * scene, WidgetType type = wt_UMLWidget, Uml::ID::Type id = Uml::ID::None, UMLWidgetController *widgetController = 0);
    virtual ~UMLWidget();

    // Copy constructor - not implemented.
    // UMLWidget(const UMLWidget& other);

    UMLWidget& operator=(const UMLWidget& other);

    bool operator==(const UMLWidget& other) const;

    virtual UMLSceneSize minimumSize();
    void setMinimumSize(const UMLSceneSize &size);

    virtual UMLSceneSize maximumSize();
    void setMaximumSize(const UMLSceneSize &size);

    void setUseFillColor(bool fc);

    virtual void setTextColor(const QColor &color);
    void setTextColorcmd(const QColor &color);

    virtual void setLineColor(const QColor &color);
    void setLineColorcmd(const QColor &color);

    virtual void setLineWidth(uint width);

    virtual void setFillColor(const QColor &color);
    void setFillColorcmd(const QColor &color);

    virtual void setSelected(bool _select);

    /**
     * Returns the state of whether the widget is selected.
     *
     * @return Returns the state of whether the widget is selected.
     */
    bool isSelected() const {
        return m_selected;
    }

    void setSelectedFlag(bool _select) {
        m_selected = _select;
    }

    void setScene(UMLScene * v);

    virtual bool activate(IDChangeLog* ChangeLog = 0);

    virtual UMLSceneValue onWidget(const UMLScenePoint & p);

    void setPenFromSettings(QPainter & p);

    virtual QFont font() const;
    virtual void setFont(QFont font);

    /**
     * Returns whether we triggered the update of position movement.
     * If so, you probably don't want to move it.
     *
     * @return The moving state.
     */
    bool getStartMove() {
        return m_startMove;
    }

    /**
     * Returns the height of widget.
     */
    int height() const {
        return rect().height();
    }

    /**
     * Returns the width of the widget.
     */
    UMLSceneValue width() const {
        return rect().width();
    }

    void setSize(UMLSceneValue width,UMLSceneValue height);

    bool getIgnoreSnapToGrid() const;
    void setIgnoreSnapToGrid(bool to);

    void moveByLocal(int dx, int dy);

    void removeAssoc(AssociationWidget* pAssoc);
    void addAssoc(AssociationWidget* pAssoc);

    /**
     *  Returns the list of associations connected to this widget.
     */
    AssociationWidgetList & associationWidgetList() {
        return m_Assocs;
    }

    /**
     * Read property of bool m_isInstance
     */
    bool isInstance() const {
        return m_isInstance;
    }

    /**
     * Write property of bool m_isInstance
     */
    void setIsInstance(bool isInstance) {
        m_isInstance = isInstance;
    }

    /**
     * Write property of m_instanceName
     */
    void setInstanceName(const QString &instanceName) {
        m_instanceName = instanceName;
    }

    /**
     * Read property of m_instanceName
     */
    QString instanceName() const {
        return m_instanceName;
    }

    bool showStereotype() const;
    virtual void setShowStereotype(bool flag);

    virtual void showPropertiesDialog();

    bool isActivated();

    virtual QString name() const;
    virtual void setName(const QString &strName);

    virtual ListPopupMenu* setupPopupMenu(ListPopupMenu *menu=0);

    virtual void adjustAssocs(int x, int y);
    void adjustUnselectedAssocs(int x, int y);

    void setActivated(bool Active = true);

    virtual void cleanup();

    static bool widgetHasUMLObject(WidgetBase::WidgetType type);

    void updateGeometry();

    void clipSize();

    void forceUpdateFontMetrics(QPainter *painter);

    virtual bool loadFromXMI(QDomElement &qElement);
    virtual void saveToXMI(QDomDocument &qDoc, QDomElement &qElement);

    UMLWidgetController* getWidgetController();

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * me);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *me);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *me);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * me);

    virtual void moveEvent(QGraphicsSceneMouseEvent *me);

    virtual void constrain(UMLSceneValue& width, UMLSceneValue& height);

    virtual void drawSelected(QPainter * p, int offsetX, int offsetY);
    virtual void drawShape(QPainter &p);

    virtual UMLSceneSize calculateSize();
    void resize();

    bool fixedAspectRatio()
    {
        return m_fixedAspectRatio;
    }

    void setFixedAspectRatio(bool state)
    {
        m_fixedAspectRatio = state;
    }

    typedef enum {
        FT_NORMAL = 0,
        FT_BOLD  = 1,
        FT_ITALIC = 2,
        FT_UNDERLINE = 3,
        FT_BOLD_ITALIC = 4,
        FT_BOLD_UNDERLINE = 5,
        FT_ITALIC_UNDERLINE = 6,
        FT_BOLD_ITALIC_UNDERLINE = 7,
        FT_INVALID = 8
    } FontType;

    virtual void setDefaultFontMetrics(UMLWidget::FontType fontType);
    virtual void setDefaultFontMetrics(UMLWidget::FontType fontType, QPainter &painter);

    QFontMetrics &getFontMetrics(UMLWidget::FontType fontType);
    void setFontMetrics(UMLWidget::FontType fontType, QFontMetrics fm);
    void setupFontType(QFont &font, UMLWidget::FontType fontType);

    void init();

    QRectF rect() const;
    void setRect(const QRectF& rect);
    void setRect(qreal x, qreal y, qreal width, qreal height);

    ///////////////// Data Loaded/Saved /////////////////////////////////

    /**
     * A list of AssociationWidgets between the UMLWidget and other UMLWidgets in the diagram
     */
    AssociationWidgetList m_Assocs;

    /**
     * getName() returns the name from the UMLObject if this widget has an
     * underlying UMLObject; if it does not, then getName() returns the local
     * m_Text (notably the case for FloatingTextWidget.)
     */
    QString m_Text;

    QFont m_Font;   ///< the font the widget will use

    QString m_instanceName;  ///< instance name (used if on a deployment diagram)
    bool m_isInstance;       ///< holds whether this widget is a component instance (i.e. on a deployment diagram)
    bool m_showStereotype;   ///< should the stereotype be displayed

    ///////////////// End of Data Loaded/Saved //////////////////////////

    bool m_selected, m_startMove;

    int            m_nPosX;
    ListPopupMenu *m_pMenu;
    bool           m_menuIsEmbedded;
    UMLDoc        *m_doc;  ///< shortcut for UMLApp::app()->getDocument()
    bool           m_resizable;
    QFontMetrics  *m_pFontMetrics[FT_INVALID];
    UMLSceneSize   m_minimumSize;
    UMLSceneSize   m_maximumSize;

    /**
     * It is true if the Activate Function has been called for this
     * class instance
     */
    bool m_activated;

    /**
     * Change Widget Behaviour
     */
    bool m_ignoreSnapToGrid;
    bool m_ignoreSnapComponentSizeToGrid;
    bool m_fixedAspectRatio;

    /**
     * Controller for user interaction events.
     */
    UMLWidgetController *m_widgetController;

public slots:
    virtual void slotRemovePopupMenu();
    virtual void updateWidget();
    virtual void slotMenuSelection(QAction* action);
    virtual void slotWidgetMoved(Uml::ID::Type id);
    virtual void slotFillColorChanged(Uml::ID::Type viewID);
    virtual void slotLineColorChanged(Uml::ID::Type viewID);
    virtual void slotTextColorChanged(Uml::ID::Type viewID);
    virtual void slotLineWidthChanged(Uml::ID::Type viewID);

    void slotClearAllSelected();
    void slotSnapToGrid();

signals:
    /**
     * Emit when the widget moves its' position.
     *
     * @param id The id of the object behind the widget.
     */
    void sigWidgetMoved(Uml::ID::Type id);

protected:
    QRectF      m_rect;     ///< widget size
};
#endif
