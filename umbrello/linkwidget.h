/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LINKWIDGET_H
#define LINKWIDGET_H

#include <qobject.h>
#include <qfont.h>

#include "umlnamespace.h"

// forward declarations
class UMLView;
class UMLObject;
class UMLClassifier;
class FloatingText;

/**
 * This is the base class for the AssociationWidget and MessageWidget.
 * Initially, the design of this class was driven by the requirements of
 * class FloatingText.  As the architecture of Umbrello evolves (for
 * example, if the class FloatingText is redesigned), this class can be
 * cleaned up.  In particular, the fact that UMLWidget inherits from this
 * class is only driven by the fact that MessageWidget inherits from
 * UMLWidget.  Over time, the inheritance graph should be changed.
 *
 * @short	Base class for AssociationWidget and MessageWidget.
 * @author 	Oliver Kellogg <okellogg@users.sourceforge.net>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class LinkWidget : public QObject {
	Q_OBJECT
public:
	/**
	 * Creates a LinkWidget object.
	 *
	 * @param view	The view to be displayed on.
	 */
	LinkWidget(UMLView * view);

	/**
	 * Standard deconstructor
	 */
	virtual ~LinkWidget() {}

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
	const UMLView *getUMLView() const { return m_pView; };

	/**
	 * Sets the @ref UMLObject to represent.
	 *
	 * @param o	The object to represent.
	 */
	virtual void setUMLObject(UMLObject * o);

	/**
	 * Used by some child classes to get documentation.
	 *
	 * @return	The documentation from the UMLObject (if m_pObject is set.)
	 */
	virtual QString getDoc() const;

	/**
	 * Used by some child classes to set documentation.
	 *
	 * @param doc	The documentation to be set in the UMLObject
	 *		(if m_pObject is set.)
	 */
	virtual void setDoc( QString doc );

	/**
	 * Sets the font the widget is to use.
	 * Abstract operation implemented by inheriting classes.
	 * Motivated by FloatingText::slotMenuSelection(mt_Operation)
	 *
	 * @param font		Font to be set.
	 */
	virtual void setFont(QFont font) = 0;

	// The following methods are only documented by how they are motivated.
	// FIXME: Inheritance contamination - these methods are only required
	// for {Message,Association}Widget but not for the other widget classes.
	// IMHO the aim should be to get rid of these methods - presumably by
	// redesigning the FloatingText class.

	/**
	 * Motivated by FloatingText::slotMenuSelection(mt_Operation)
	 */
	virtual UMLClassifier *getOperationOwner(FloatingText *ft);

	/**
	 * Motivated by FloatingText::slotMenuSelection(mt_Operation)
	 */
	virtual void setOperationText(FloatingText *ft, QString opText);

	/**
	 * Motivated by FloatingText::slotMenuSelection(mt_Reset_Label_Positions)
	 */
	virtual void resetTextPositions();

	/**
	 * Motivated by FloatingText::setMessageText()
	 */
	virtual void setMessageText(FloatingText *ft);

	/**
	 * Motivated by FloatingText::handleRename()
	 */
	virtual void setText(FloatingText *ft, QString newText);

	/**
	 * Motivated by FloatingText::mouseDoubleClickEvent()
	 */
	virtual bool showDialog();

	/**
	 * Motivated by FloatingText::showOpDlg()
	 */
	virtual UMLClassifier *getSeqNumAndOp(FloatingText *ft, QString& seqNum, QString& op);

	/**
	 * Motivated by FloatingText::showOpDlg()
	 */
	virtual void setSeqNumAndOp(QString seqNum, QString op);

	/**
	 * Motivated by FloatingText::mouseMoveEvent()
	 */
	virtual void updateMessagePos(int textHeight, int& newX, int& newY);

	/**
	 * Motivated by FloatingText::mouseMoveEvent()
	 */
	void constrainY(int &y, int height);
 
	/**
	 * Motivated by FloatingText::setLink(). Bad.
	 */
	virtual void calculateNameTextSegment();

	/**
	 * Particularly horrible. Motivated by FloatingText::setLink().
	 */
	virtual void cleanupBeforeFTsetLink(FloatingText *ft);

	/**
	 * Particularly horrible. Motivated by FloatingText::setLink().
	 */
	virtual void setupAfterFTsetLink(FloatingText *ft);

protected:
	/**
	 * Type of widget.
	 */
	Uml::Widget_Type m_Type;

	UMLView   *m_pView;
	UMLObject *m_pObject;
};

#endif
