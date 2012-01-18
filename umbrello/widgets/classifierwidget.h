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
public:

    /**
     * Constructs a ClassifierWidget.
     *
     * @param scene      The parent of this ClassifierWidget.
     * @param o The UMLObject to represent.
     */
    ClassifierWidget(UMLScene * scene, UMLClassifier * o);

    /**
     * Destructor.
     */
    virtual ~ClassifierWidget();

    /**
     * Toggles the status of whether to show StereoType.
     */
    void toggleShowStereotype();

    /**
     * Return the status of showing operations.
     *
     * @return  Return the status of showing operations.
     */
    bool getShowOps() const;

    /**
     *  Set the status of whether to show Operations
     *
     * @param _show             True if operations shall be shown.
     */
    void setShowOps(bool _show);

    /**
     * Toggles the status of showing operations.
     */
    void toggleShowOps();

    /**
     * Return true if public operations/attributes are shown only.
     */
    bool getShowPublicOnly() const;

    /**
     * Set whether to show public operations/attributes only.
     */
    void setShowPublicOnly(bool _status);

    /**
     * Toggle whether to show public operations/attributes only.
     */
    void toggleShowPublicOnly();

    /**
     * Returns the status of whether to show visibility.
     *
     * @return  True if visibility is shown.
     */
    bool getShowVisibility() const;

    /**
     * Set the status of whether to show visibility
     *
     * @param _visibility    True if visibility shall be shown.
     */
    void setShowVisibility(bool _visibility);

    /**
     * Toggles the status of whether to show visibility
     */
    void toggleShowVisibility();

    /**
     * Return the status of showing operation signatures.
     *
     * @return  Status of showing operation signatures.
     */
    Uml::SignatureType operationSignatureType() const;

    /**
     * Set the status of whether to show Operation signature
     *
     * @param _show   True if operation signatures shall be shown.
     */
    void setShowOpSigs(bool _show);

    /**
     * Toggles the status of showing operation signatures.
     */
    void toggleShowOpSigs();

    /**
     * Returns the status of whether to show Package.
     *
     * @return  True if package is shown.
     */
    bool getShowPackage() const;

    /**
     * Set the status of whether to show Package.
     *
     * @param _status             True if package shall be shown.
     */
    void setShowPackage(bool _status);

    /**
     * Toggles the status of whether to show package.
     */
    void toggleShowPackage();

    /**
     * Set the type of signature to display for an Operation
     *
     * @param sig       Type of signature to display for an operation.
     */
    void setOpSignature(Uml::SignatureType sig);

    /**
     * Return the number of displayed attributes.
     */
    int displayedAttributes();

    /**
     * Return the number of displayed operations.
     */
    int displayedOperations();

    /**
     * Returns whether to show attributes.
     * Only applies when m_pObject->getBaseType() is ot_Class.
     *
     * @return  True if attributes are shown.
     */
    bool getShowAtts() const {
        return m_showAttributes;
    }

    /**
     * Toggles whether to show attributes.
     * Only applies when m_pObject->getBaseType() is ot_Class.
     */
    void toggleShowAtts();

    /**
     * Returns whether to show attribute signatures.
     * Only applies when m_pObject->getBaseType() is ot_Class.
     *
     * @return  Status of how attribute signatures are shown.
     */
    Uml::SignatureType attributeSignatureType() {
        return m_ShowAttSigs;
    }

    /**
     * Toggles whether to show attribute signatures.
     * Only applies when m_pObject->getBaseType() is ot_Class.
     */
    void toggleShowAttSigs();

    /**
     * Sets whether to show attributes.
     * Only applies when m_pObject->getBaseType() is ot_Class.
     *
     * @param _show             True if attributes shall be shown.
     */
    void setShowAtts(bool _show);

    /**
     * Sets whether to show attribute signature
     * Only applies when m_pObject->getBaseType() is ot_Class.
     *
     * @param _show             True if attribute signatures shall be shown.
     */
    void setShowAttSigs(bool _show);

    /**
     * Sets the type of signature to display for an attribute.
     * Only applies when m_pObject->getBaseType() is ot_Class.
     *
     * @param sig       Type of signature to display for an attribute.
     */
    void setAttSignature(Uml::SignatureType sig);

    /**
     * Returns whether to draw as circle.
     * Only applies when m_pObject->getBaseType() is ot_Interface.
     *
     * @return  True if widget is drawn as circle.
     */
    bool getDrawAsCircle() const;

    /**
     * Toggles whether to draw as circle.
     * Only applies when m_pObject->getBaseType() is ot_Interface.
     */
    void toggleDrawAsCircle();

    /**
     * Sets whether to draw as circle.
     * Only applies when m_pObject->getBaseType() is ot_Interface.
     *
     * @param drawAsCircle      True if widget shall be drawn as circle.
     */
    void setDrawAsCircle(bool drawAsCircle);

    /**
     * Changes this classifier from an interface to a class.
     * Attributes and stereotype visibility is got from the view OptionState.
     * This widget is also updated.
     */
    void changeToClass();

    /**
     * Changes this classifier from a class to an interface.
     * Attributes are hidden and stereotype is shown.
     * This widget is also updated.
     */
    void changeToInterface();

    /**
     * Set the AssociationWidget when this ClassWidget acts as
     * an association class.
     */
    void setClassAssocWidget(AssociationWidget *assocwidget);

    /**
     * Return the AssociationWidget when this classifier acts as
     * an association class (else return NULL.)
     */
    AssociationWidget *getClassAssocWidget();

    /**
     * Return the UMLClassifier which this ClassifierWidget
     * represents.
     */
    UMLClassifier *classifier();

    /**
     * Overrides standard method.
     * Auxiliary to reimplementations in the derived classes.
     */
    void paint(QPainter & p, int offsetX, int offsetY);

    /**
     * Extends base method to adjust also the association of a class
     * association.
     * Executes the base method and then, if file isn't loading and the
     * classifier acts as a class association, the association position is
     * updated.
     *
     * @param x The x-coordinate.
     * @param y The y-coordinate.
     */
    virtual void adjustAssocs(int x, int y);

    /**
     * Creates the "classwidget" or "interfacewidget" XML element.
     */
    void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);

    /**
     * Loads the "classwidget" or "interfacewidget" XML element.
     */
    bool loadFromXMI(QDomElement & qElement);

public slots:
    /**
     * Will be called when a menu selection has been made from the
     * popup menu.
     *
     * @param action       The action that has been selected.
     */
    void slotMenuSelection(QAction* action);

protected:

    /**
     * Initializes key variables of the class.
     */
    void init();

    /**
     * Calculcates the size of the templates box in the top left
     * if it exists, returns QSize(0,0) if it doesn't.
     *
     * @return  QSize of the templates flap.
     */
    QSize calculateTemplatesBoxSize();

    /**
     * Overrides method from UMLWidget.
     */
    UMLSceneSize minimumSize();

    /**
     * Draws the interface as a circle with name underneath.
     * Only applies when m_pObject->getBaseType() is ot_Interface.
     */
    void drawAsCircle(QPainter& p, int offsetX, int offsetY);

    /**
     * Calculates the size of the object when drawn as a circle.
     * Only applies when m_pObject->getBaseType() is ot_Interface.
     */
    QSize calculateAsCircleSize();

    /**
     * Updates m_ShowOpSigs to match m_showVisibility.
     */
    void updateSigs();

    /**
     * Return the number of displayed members of the given ObjectType.
     * Takes into consideration m_showPublicOnly but not other settings,
     */
    int displayedMembers(UMLObject::ObjectType ot);

    /**
     * Auxiliary method for draw() of child classes:
     * Draw the attributes or operations.
     *
     * @param p          QPainter to paint to.
     * @param ot         Object type to draw, either ot_Attribute or ot_Operation.
     * @param sigType    Governs details of the member display.
     * @param x          X coordinate at which to draw the texts.
     * @param y          Y coordinate at which text drawing commences.
     * @param fontHeight The font height.
     */
    void drawMembers(QPainter & p, UMLObject::ObjectType ot, Uml::SignatureType sigType,
                     int x, int y, int fontHeight);

    bool m_showOperations;            ///< Loaded/saved item.
    bool m_showPublicOnly;            ///< Loaded/saved item.
    bool m_showAccess;                ///< Loaded/saved item.
    bool m_showPackage;               ///< Loaded/saved item.
    bool m_showAttributes;            ///< Loaded/saved item.
    bool m_drawAsCircle;              ///< Loaded/saved item.
    Uml::SignatureType m_ShowAttSigs;  ///< Loaded/saved item.
    Uml::SignatureType m_ShowOpSigs;   ///< Loaded/saved item.

    /**
     * Text width margin
     */
    static const int MARGIN;

    /**
     * Size of circle when interface is rendered as such
     */
    static const int CIRCLE_SIZE;

    /// Auxiliary variable for size calculations and drawing
    int m_bodyOffsetY;

    /**
     * The related AssociationWidget in case this classifier
     * acts as an association class
     */
    AssociationWidget *m_pAssocWidget;

};

#endif
