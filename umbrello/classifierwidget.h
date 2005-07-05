/*
 *  copyright (C) 2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CLASSIFIERWIDGET_H
#define CLASSIFIERWIDGET_H

#include "umlwidget.h"

class QPainter;
class UMLClassifier;
class AssociationWidget;

/**
 * @short Common implementation for class widget and interface widget
 * @author Oliver Kellogg
 * @see	UMLWidget
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class ClassifierWidget : public UMLWidget {
public:

    /**
     * Constructs a ClassifierWidget.
     *
     * @param view	The parent of this ClassifierWidget.
     * @param o	The UMLObject to represent.
     */
    ClassifierWidget(UMLView * view, UMLClassifier * o);

    /**
     * Destructor.
     */
    virtual ~ClassifierWidget();

    /**
     * Returns the status of whether to show StereoType.
     *
     * @return	True if stereotype is shown.
     */
    bool getShowStereotype() const;

    /**
     * Set the status of whether to show StereoType
     *
     * @param _show		True if stereotype shall be shown.
     */
    void setShowStereotype(bool _status);

    /**
     * Toggles the status of whether to show StereoType.
     */
    void toggleShowStereotype();

    /**
     * Return the status of showing operations.
     *
     * @return	Return the status of showing operations.
     */
    bool getShowOps() const;

    /**
     *  Set the status of whether to show Operations
     *
     * @param _show		True if operations shall be shown.
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
     * Returns the status of whether to show scope.
     *
     * @return	True if scope is shown.
     */
    bool getShowScope() const;

    /**
     * Set the status of whether to show scope
     *
     * @param _scope	True if scope shall be shown.
     */
    void setShowScope(bool _scope);

    /**
     * Toggles the status of whether to show scope
     */
    void toggleShowScope();

    /**
     * Return the status of showing operation signatures.
     *
     * @return	Status of showing operation signatures.
     */
    Uml::Signature_Type getShowOpSigs() const;

    /**
     * Set the status of whether to show Operation signature
     *
     * @param _show		True if operation signatures shall be shown.
     */
    void setShowOpSigs(bool _show);

    /**
     * Toggles the status of showing operation signatures.
     */
    void toggleShowOpSigs();

    /**
     * Returns the status of whether to show Package.
     *
     * @return	True if package is shown.
     */
    bool getShowPackage() const;

    /**
     * Set the status of whether to show Package.
     *
     * @param _show		True if package shall be shown.
     */
    void setShowPackage(bool _status);

    /**
     * Toggles the status of whether to show package.
     */
    void toggleShowPackage();

    /**
     * Set the type of signature to display for an Operation
     *
     * @param sig	Type of signature to display for an operation.
     */
    void setOpSignature(Uml::Signature_Type sig);

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
     * @return	True if attributes are shown.
     */
    bool getShowAtts() const {
        return m_bShowAttributes;
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
     * @return	Status of how attribute signatures are shown.
     */
    Uml::Signature_Type getShowAttSigs() {
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
     * @param _show		True if attributes shall be shown.
     */
    void setShowAtts(bool _show);

    /**
     * Sets whether to show attribute signature
     * Only applies when m_pObject->getBaseType() is ot_Class.
     *
     * @param _show		True if attribute signatures shall be shown.
     */
    void setShowAttSigs(bool _show);

    /**
     * Sets the type of signature to display for an attribute.
     * Only applies when m_pObject->getBaseType() is ot_Class.
     *
     * @param sig	Type of signature to display for an attribute.
     */
    void setAttSignature(Uml::Signature_Type sig);

    /**
     * Returns whether to draw as circle.
     * Only applies when m_pObject->getBaseType() is ot_Interface.
     *
     * @return	True if widget is drawn as circle.
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
     * @param _show		True if widget shall be drawn as circle.
     */
    void setDrawAsCircle(bool drawAsCircle);

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
    UMLClassifier *getClassifier();

    /**
     * Activate the object after serializing it from a QDataStream.
     */
    bool activate(IDChangeLog* ChangeLog  = 0 );

    /**
     * Overrides standard method.
     * Auxiliary to reimplementations in the derived classes.
     */
    void draw(QPainter & p, int offsetX, int offsetY);

    /**
     * Creates the <classwidget> or <interfacewidget >XML element.
     */
    void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);

    /**
     * Loads the <classwidget> or <interfacewidget> XML element.
     */
    bool loadFromXMI(QDomElement & qElement);

public slots:
    /**
     * Will be called when a menu selection has been made from the
     * popup menu.
     *
     * @param sel	The selection id that has been selected.
     */
    void slotMenuSelection(int sel);

protected:

    /**
     * Initializes key variables of the class.
     */
    void init();

    /**
     * Calculcates the size of the templates box in the top left
     * if it exists, returns QSize(0,0) if it doesn't.
     *
     * @return	QSize of the templates flap.
     */
    QSize calculateTemplatesBoxSize();

    /**
     * Method for automatically computing the size of the object.
     */
    void calculateSize();

    /**
     * Draws the interface as a circle with name underneath.
     * Only applies when m_pObject->getBaseType() is ot_Interface.
     */
    void drawAsCircle(QPainter& p, int offsetX, int offsetY);

    /**
     * Calculates the size of the object when drawn as a circle.
     * Only applies when m_pObject->getBaseType() is ot_Interface.
     */
    void calculateAsCircleSize();

    /**
     * Automatically calculates the size of the object when drawn as
     * a concept.
     */
    void calculateAsConceptSize();

    /**
     * Updates m_ShowOpSigs to match m_bShowScope.
     */
    void updateSigs();

    /**
     * Return the number of displayed members of the given Object_Type.
     * Takes into consideration m_bShowPublicOnly but not other settings,
     */
    int displayedMembers(Uml::Object_Type ot);

    /**
     * Auxiliary method for draw() of child classes:
     * Draw the attributes or operations.
     *
     * @param p		QPainter to paint to.
     * @param ot		Object type to draw, either ot_Attribute or ot_Operation.
     * @param sigType	Governs details of the member display.
     * @param x		X coordinate at which to draw the texts.
     * @param y		Y coordinate at which text drawing commences.
     * @param fontHeight	The font height.
     */
    void drawMembers(QPainter & p, Uml::Object_Type ot, Uml::Signature_Type sigType,
                     int x, int y, int fontHeight);

    bool m_bShowOperations;            ///< Loaded/saved item.
    bool m_bShowPublicOnly;            ///< Loaded/saved item.
    bool m_bShowAccess;                ///< Loaded/saved item.
    bool m_bShowPackage;               ///< Loaded/saved item.
    bool m_bShowStereotype;            ///< Loaded/saved item.
    bool m_bShowAttributes;            ///< Loaded/saved item.
    bool m_bDrawAsCircle;              ///< Loaded/saved item.
    Uml::Signature_Type m_ShowAttSigs; ///< Loaded/saved item.
    Uml::Signature_Type m_ShowOpSigs;  ///< Loaded/saved item.

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
