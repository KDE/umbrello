/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef WIDGETBASE_H
#define WIDGETBASE_H

#include <qobject.h>
#include <qcolor.h>
#include <qdom.h>

#include "umlnamespace.h"

// forward declarations
class UMLView;
class UMLObject;

/**
 * @short       Common base class for UMLWidget and AssociationWidget
 * @author      Oliver Kellogg <okellogg@users.sourceforge.net>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class WidgetBase : public QObject {
    Q_OBJECT
public:
    /**
     * Creates a WidgetBase object.
     *
     * @param view      The view to be displayed on.
     */
    WidgetBase(UMLView * view);

    /**
     * Standard deconstructor
     */
    virtual ~WidgetBase() {}

    /**
     * Write property of m_Type.
     */
    void setBaseType(Uml::Widget_Type type);

    /**
     * Read property of m_Type.
     */
    Uml::Widget_Type getBaseType() const;

    /**
     * Returns the @ref UMLObject set to represent.
     *
     * @return the UMLObject to represent.
     */
    UMLObject *getUMLObject();

    /**
     * Deliver a const pointer to the connected UMLView
     * ( needed esp. by event handling of LinePath )
     */
    const UMLView *getUMLView() const { return m_pView; }

    /**
     * Sets the @ref UMLObject to represent.
     *
     * @param o The object to represent.
     */
    virtual void setUMLObject(UMLObject * o);

    /**
     * Used by some child classes to get documentation.
     *
     * @return  The documentation from the UMLObject (if m_pObject is set.)
     */
    virtual QString getDoc() const;

    /**
     * Used by some child classes to set documentation.
     *
     * @param doc       The documentation to be set in the UMLObject
     *          (if m_pObject is set.)
     */
    virtual void setDoc( const QString &doc );

    /**
     * Sets the line colour
     *
     * @param colour the new line colour
     */
    virtual void setLineColor(const QColor &colour);

    /**
     * Sets the line width
     *
     * @param width the new line width
     */
    virtual void setLineWidth(uint width);

    /**
     * Read property of m_LineColour.
     */
    QColor getLineColor() const {
        return m_LineColour;
    }

    /**
     * Read property of m_LineWidth.
     */
    uint getLineWidth() const {
        return m_LineWidth;
    }

    /**
     * Returns m_bUsesDiagramLineColour
     */
    bool getUsesDiagramLineColour() const {
        return m_bUsesDiagramLineColour;
    }

    /**
     * Returns m_bUsesDiagramLineWidth
     */
    bool getUsesDiagramLineWidth() const {
        return m_bUsesDiagramLineWidth;
    }

    /**
     * Sets m_bUsesDiagramLineColour
     */
    void setUsesDiagramLineColour(bool usesDiagramLineColour) {
        m_bUsesDiagramLineColour = usesDiagramLineColour;
    }

    /**
     * Sets m_bUsesDiagramLineWidth
     */
    void setUsesDiagramLineWidth(bool usesDiagramLineWidth) {
        m_bUsesDiagramLineWidth = usesDiagramLineWidth;
    }

    /**
    * Write property of m_nId.
    */
    void setID( Uml::IDType id );

    /**
    * Read property of m_nId.
    */
    Uml::IDType getID() const;

    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    virtual bool loadFromXMI( QDomElement & qElement );

protected:
    /**
     * Initialize members.
     */
    void init(UMLView *view, Uml::Widget_Type type = Uml::wt_UMLWidget);

    /**
     * Type of widget.
     */
    Uml::Widget_Type m_Type;

    UMLView   *m_pView;
    UMLObject *m_pObject;
    QString m_Doc;  ///< Only used if m_pObject is not set.

    /**
     * This ID is only used when the widget does not have a
     * corresponding UMLObject (i.e. the m_pObject pointer is NULL.)
     * For UMLObjects, the ID from the UMLObject is used.
     */
    Uml::IDType m_nId;

    /**
     * Color of the lines of the widget. Is saved to XMI.
     */
    QColor m_LineColour;

    /**
     * Width of the lines of the widget. Is saved to XMI.
     */
    uint m_LineWidth;

    /**
     * true by default, false if the colours have
     * been explicitly set for this widget.
     * These are saved to XMI.
     */
    bool m_bUsesDiagramLineColour, m_bUsesDiagramLineWidth;

};

#endif
