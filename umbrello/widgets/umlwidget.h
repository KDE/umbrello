/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLWIDGET_H
#define UMLWIDGET_H

#include <QtCore/QDateTime>
#include <QtGui/QFont>

#include "basictypes.h"
#include "widgetbase.h"
#include "associationwidgetlist.h"
#include "optionstate.h"
#include "umlscene.h"

class UMLWidgetController;

class UMLObject;
class UMLScene;
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
class UMLWidget : public WidgetBase, public UMLSceneRectangle
{
    Q_OBJECT
public:
    static const UMLSceneSize DefaultMinimumSize;
    static const UMLSceneSize DefaultMaximumSize;

    friend class UMLWidgetController;

    explicit UMLWidget(UMLScene * scene, WidgetType type = wt_UMLWidget, UMLObject * o = 0, UMLWidgetController *widgetController = 0);
    explicit UMLWidget(UMLScene * scene, WidgetType type = wt_UMLWidget, Uml::IDType id = Uml::id_None, UMLWidgetController *widgetController = 0 );
    virtual ~UMLWidget();

    // Copy constructor - not implemented.
    // UMLWidget(const UMLWidget& other);

    UMLWidget& operator=(const UMLWidget& other);

    bool operator==(const UMLWidget& other) const;

    void setUseFillColor(bool fc);

    virtual void setTextColor(const QColor &color);
    void setTextColorcmd(const QColor &color);

    virtual void setLineColor(const QColor &color);
    void setLineColorcmd(const QColor &color);

    virtual void setLineWidth(uint width);

    virtual void setFillColor(const QColor &color);
    void setFillColorcmd(const QColor &color);

    /**
     * Returns whether this is a line of text.
     * Used for transparency in printing.
     *
     * @return always false
     */
    virtual bool isText() {
        return false;
    }

    virtual void setSelected(bool _select);

    /**
     * Returns the state of whether the widget is selected.
     *
     * @return Returns the state of whether the widget is selected.
     */
    bool getSelected() const {
        return m_selected;
    }

    void setSelectedFlag(bool _select) {
        m_selected = _select;
    }

    void setScene(UMLScene * v);

    virtual bool activate(IDChangeLog* ChangeLog = 0);

    virtual int onWidget(const QPoint & p);

    /**
     * Draws the UMLWidget on the given paint device
     *
     * @param p The painter for the drawing device
     * @param offsetX x position to start the drawing.
     * @param offsetY y position to start the drawing.
     *
     */
    virtual void paint(QPainter & p, int offsetX, int offsetY) = 0;

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

    virtual void setX(int x);
    virtual void setY(int y);
    virtual void setZ(int z);

    /**
     * Gets the x-coordinate.
     */
    int getX() const {
        return (int)UMLSceneItem::x();
    }

    /**
     * Gets the y-coordinate.
     */
    int getY() const {
        return (int)UMLSceneItem::y();
    }

    /**
     * Gets the z-coordinate.
     */
    int getZ() const {
        return (int)UMLSceneItem::z();
    }

    /**
     * Returns the height of widget.
     */
    int getHeight() const {
        return UMLSceneRectangle::height();
    }

    /**
     * Returns the width of the widget.
     */
    int getWidth() const {
        return UMLSceneRectangle::width();
    }

    void setSize(int width,int height);

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
     * Write property of bool m_isInstance
     */
    void setIsInstance(bool isInstance) {
        m_isInstance = isInstance;
    }

    /**
     * Read property of bool m_isInstance
     */
    bool isInstance() const {
        return m_isInstance;
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

    bool getShowStereotype() const;
    virtual void setShowStereotype(bool _status);

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

    void updateComponentSize();

    void clipSize();

    void forceUpdateFontMetrics(QPainter *painter);

    virtual bool loadFromXMI(QDomElement &qElement);
    virtual void saveToXMI(QDomDocument &qDoc, QDomElement &qElement);

    UMLWidgetController* getWidgetController();

    virtual void mousePressEvent(QMouseEvent *me);
    virtual void mouseMoveEvent(QMouseEvent* me);
    virtual void mouseReleaseEvent(QMouseEvent * me);
    virtual void mouseDoubleClickEvent(QMouseEvent *me);

protected:

    virtual void moveEvent(QMoveEvent *me);

    virtual void constrain(int& width, int& height);

    virtual void drawSelected(QPainter * p, int offsetX, int offsetY);
    virtual void drawShape(QPainter &p);

    virtual UMLSceneSize minimumSize();
    void setMinimumSize(const UMLSceneSize &size);
    void setMinimumSize(UMLSceneValue width, UMLSceneValue height);

    virtual UMLSceneSize maximumSize();
    void setMaximumSize(const UMLSceneSize &size);
    void setMaximumSize(UMLSceneValue width, UMLSceneValue height);

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

    /** Template Method, override this to set the default
     *  font metric.
     */
    virtual void setDefaultFontMetrics(UMLWidget::FontType fontType);
    virtual void setDefaultFontMetrics(UMLWidget::FontType fontType, QPainter &painter);

    /** Returns the font metric used by this object for Text which uses bold/italic fonts */
    QFontMetrics &getFontMetrics(UMLWidget::FontType fontType);
    /** set the font metric to use */
    void setFontMetrics(UMLWidget::FontType fontType, QFontMetrics fm);
    void setupFontType(QFont &font, UMLWidget::FontType fontType);

    void init();

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

    /**
     *  The font the widget will use.
     */
    QFont m_Font;

    /**
     * Holds whether this widget is a component instance (i.e. on a deployment diagram)
     */
    bool m_isInstance;

    /**
     * The instance name (used if on a deployment diagram)
     */
    QString m_instanceName;

    /**
     * Should the stereotype be displayed
     */
    bool m_showStereotype;

    ///////////////// End of Data Loaded/Saved //////////////////////////

    bool m_selected, m_startMove;

    int            m_nPosX;
    int            m_origZ;
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

    /**
     * Controller for user interaction events.
     */
    UMLWidgetController *m_widgetController;

public slots:

    virtual void slotRemovePopupMenu();
    virtual void updateWidget();
    virtual void slotMenuSelection(QAction* action);
    virtual void slotWidgetMoved(Uml::IDType id);
    virtual void slotColorChanged(Uml::IDType viewID);
    virtual void slotLineWidthChanged(Uml::IDType viewID);

    void slotClearAllSelected();
    void slotSnapToGrid();

signals:
    /**
     * Emit when the widget moves its' position.
     *
     * @param id The id of the object behind the widget.
     */
    void sigWidgetMoved(Uml::IDType id);
};

#endif
