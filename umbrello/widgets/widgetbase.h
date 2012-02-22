/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef WIDGETBASE_H
#define WIDGETBASE_H

#include "basictypes.h"

#include <QtCore/QObject>
#include <QtGui/QColor>
#include <QtGui/QPainter>
#include <QtXml/QDomDocument>

// forward declarations
class UMLDoc;
class UMLScene;
class UMLObject;

/**
 * @short       Common base class for UMLWidget and AssociationWidget
 * @author      Oliver Kellogg <okellogg@users.sourceforge.net>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class WidgetBase : public QObject
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
    virtual void setUMLObject(UMLObject * o);

    Uml::IDType id() const;
    void setID(Uml::IDType id);

    WidgetType baseType() const;
    void setBaseType(WidgetType type);
    QLatin1String baseTypeStr() const;

    UMLScene* umlScene() const;
    UMLDoc* umlDoc() const;

    QString documentation() const;
    void setDocumentation(const QString &doc);

    QColor lineColor() const;
    void setLineColor(const QColor &colour);

    uint lineWidth() const;
    void setLineWidth(uint width);

    bool usesDiagramLineColour() const;
    void setUsesDiagramLineColour(bool usesDiagramLineColour);

    bool usesDiagramLineWidth() const;
    void setUsesDiagramLineWidth(bool usesDiagramLineWidth);

    virtual bool loadFromXMI( QDomElement & qElement );
    virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    WidgetBase& operator=(const WidgetBase& other);

protected:
    void init();

    WidgetType  m_Type;  ///< Type of widget.
    UMLScene   *m_scene;
    UMLObject  *m_pObject;
    QString     m_Doc;   ///< Only used if m_pObject is not set.

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
    bool m_usesDiagramLineColour, m_usesDiagramLineWidth;

};

#endif
