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

#ifndef LINKWIDGET_H
#define LINKWIDGET_H

#include <qfont.h>

#include "umlnamespace.h"

// forward declarations
class UMLClassifier;
class UMLOperation;
class FloatingText;
class UMLView;

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
	LinkWidget();
	virtual ~LinkWidget();

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
	virtual UMLClassifier *getOperationOwner();

	/**
	 * Motivated by FloatingText::slotMenuSelection(mt_Operation)
	 */
	virtual UMLOperation *getOperation() = 0;

	/**
	 * Motivated by FloatingText::slotMenuSelection(mt_Operation)
	 */
	virtual void setOperation(UMLOperation *op) = 0;

	/**
	 * Motivated by getOperationText()
	 */
	virtual QString getCustomOpText() = 0;

	/**
	 * Motivated by FloatingText::slotMenuSelection(mt_Operation)
	 */
	virtual void setCustomOpText(const QString &opText) = 0;

	/**
	 * Uses getOperation() if set, else calls getCustomOpText().
	 */
	QString getOperationText(UMLView *view = NULL);

	/**
	 * Motivated by FloatingText::slotMenuSelection(mt_Reset_Label_Positions)
	 * Only applies to AssociationWidget.
	 */
	virtual void resetTextPositions();

	/**
	 * Motivated by FloatingText::setMessageText()
	 */
	virtual void setMessageText(FloatingText *ft) = 0;

	/**
	 * Motivated by FloatingText::handleRename()
	 */
	virtual void setText(FloatingText *ft, const QString &newText) = 0;

	/**
	 * Motivated by FloatingText::mouseDoubleClickEvent()
	 * Only applies to AssociationWidget.
	 */
	virtual bool showDialog();

	/**
	 * Motivated by FloatingText::showOpDlg()
	 */
	virtual UMLClassifier *getSeqNumAndOp(QString& seqNum, QString& op) = 0;

	/**
	 * Motivated by FloatingText::showOpDlg()
	 */
	virtual void setSeqNumAndOp(const QString &seqNum, const QString &op) = 0;

	/**
	 * Motivated by FloatingText::mouseMoveEvent()
	 * Only applies to MessageWidget.
	 */
	virtual void constrainTextPos(int &textX, int &textY, int textWidth, int textHeight,
				      Uml::Text_Role tr);
 
	/**
	 * Motivated by FloatingText::setLink().
	 * Only applies to AssociationWidget.
	 */
	virtual void calculateNameTextSegment();

};

#endif
