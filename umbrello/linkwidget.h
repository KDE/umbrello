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

#include <qfont.h>

#include "umlnamespace.h"

// forward declarations
class UMLClassifier;
class FloatingText;

/**
 * This is an interface realized by AssociationWidget and MessageWidget.
 * The design of this interface was driven by the requirements of
 * class FloatingText.  As the architecture of Umbrello evolves (for
 * example, if the class FloatingText is redesigned), it can be
 * cleaned up.
 *
 * @short	Interface to FloatingText for AssociationWidget and MessageWidget.
 * @author 	Oliver Kellogg <okellogg@users.sourceforge.net>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class LinkWidget {
public:
	// FIXME: Improve documentation.

	/**
	 * Sets the font the widget is to use.
	 * Abstract operation implemented by inheriting classes.
	 * Motivated by FloatingText::slotMenuSelection(mt_Operation)
	 *
	 * @param font		Font to be set.
	 */
	virtual void lwSetFont(QFont font) = 0;

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
	virtual void constrainTextPos(int &textX, int &textY, int textWidth, int textHeight,
				      Uml::Text_Role tr);
 
	/**
	 * Motivated by FloatingText::setLink(). Bad.
	 */
	virtual void calculateNameTextSegment();

};

#endif
