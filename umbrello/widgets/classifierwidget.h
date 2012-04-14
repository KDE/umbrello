/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CLASSIFIERWIDGET_H
#define CLASSIFIERWIDGET_H

#include "basictypes.h"
#include "umlobject.h"
#include "umlwidget.h"

class QPainter;
class UMLClassifier;
class AssociationWidget;

/**
 * @short Common implementation for class widget and interface widget
 * @author Oliver Kellogg
 * @see UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ClassifierWidget : public UMLWidget
{
    Q_OBJECT
public:
    ClassifierWidget(UMLScene * scene, UMLClassifier * o);
    virtual ~ClassifierWidget();

    void toggleShowStereotype();

    bool getShowOps() const;
    void setShowOps(bool _show);
    void toggleShowOps();

    bool getShowPublicOnly() const;
    void setShowPublicOnly(bool _status);
    void toggleShowPublicOnly();

    bool getShowVisibility() const;
    void setShowVisibility(bool _visibility);
    void toggleShowVisibility();

    Uml::SignatureType operationSignatureType() const;
    void setShowOpSigs(bool _show);
    void toggleShowOpSigs();

    bool getShowPackage() const;
    void setShowPackage(bool _status);
    void toggleShowPackage();

    void setOpSignature(Uml::SignatureType sig);

    int displayedAttributes();
    int displayedOperations();

    bool getShowAtts() const;
    void setShowAtts(bool _show);
    void toggleShowAtts();

    Uml::SignatureType attributeSignatureType() const;
    void setAttSignature(Uml::SignatureType sig);

    void setShowAttSigs(bool _show);
    void toggleShowAttSigs();

    bool getDrawAsCircle() const;
    void setDrawAsCircle(bool drawAsCircle);
    void toggleDrawAsCircle();

    void changeToClass();
    void changeToInterface();

    void setClassAssociationWidget(AssociationWidget *assocwidget);
    AssociationWidget *getClassAssocWidget() const;

    UMLClassifier *classifier();

    void paint(QPainter & p, int offsetX, int offsetY);

    virtual void adjustAssocs(int x, int y);

    virtual void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);
    virtual bool loadFromXMI(QDomElement & qElement);

public slots:
    virtual void slotMenuSelection(QAction* action);

protected:
    void init();

    QSize calculateTemplatesBoxSize();

    UMLSceneSize minimumSize();

    void drawAsCircle(QPainter& p, int offsetX, int offsetY);
    QSize calculateAsCircleSize();

    void updateSigs();

    int displayedMembers(UMLObject::ObjectType ot);
    void drawMembers(QPainter & p, UMLObject::ObjectType ot, Uml::SignatureType sigType,
                     int x, int y, int fontHeight);

    static const int MARGIN;           ///< text width margin
    static const int CIRCLE_SIZE;      ///< size of circle when interface is rendered as such

    bool m_showOperations;             ///< Loaded/saved item.
    bool m_showPublicOnly;             ///< Loaded/saved item.
    bool m_showAccess;                 ///< Loaded/saved item.
    bool m_showPackage;                ///< Loaded/saved item.
    bool m_showAttributes;             ///< Loaded/saved item.
    bool m_drawAsCircle;               ///< Loaded/saved item.
    Uml::SignatureType m_ShowAttSigs;  ///< Loaded/saved item.
    Uml::SignatureType m_ShowOpSigs;   ///< Loaded/saved item.
    int m_bodyOffsetY;                 ///< auxiliary variable for size calculations and drawing
    AssociationWidget *m_pAssocWidget; ///< related AssociationWidget in case this classifier acts as an association class

};

#endif
