/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef LINKWIDGET_H
#define LINKWIDGET_H

#include <QtGui/QFont>

#include "basictypes.h"

// forward declarations
class UMLClassifier;
class UMLOperation;
class FloatingTextWidget;
class UMLScene;
class UMLView;

/**
 * This is an interface realized by AssociationWidget and MessageWidget.
 * The design of this interface was driven by the requirements of
 * class FloatingTextWidget. As the architecture of Umbrello evolves (for
 * example, if the class FloatingTextWidget is redesigned), it can be
 * cleaned up.
 *
 * @short       Interface to FloatingTextWidget for AssociationWidget and MessageWidget.
 * @author      Oliver Kellogg <okellogg@users.sourceforge.net>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class LinkWidget
{
public:
    LinkWidget();
    virtual ~LinkWidget();

    /**
     * Sets the font the widget is to use.
     * Abstract operation implemented by inheriting classes.
     * Motivated by FloatingTextWidget::slotMenuSelection(mt_Operation)
     *
     * @param font   Font to be set.
     */
    virtual void lwSetFont(QFont font) = 0;

    virtual UMLClassifier *operationOwner();

    /**
     * Motivated by FloatingTextWidget::slotMenuSelection(mt_Operation)
     */
    virtual UMLOperation *operation() = 0;

    /**
     * Motivated by FloatingTextWidget::slotMenuSelection(mt_Operation)
     */
    virtual void setOperation(UMLOperation *op) = 0;

    /**
     * Motivated by getOperationText()
     */
    virtual QString customOpText() = 0;

    /**
     * Motivated by FloatingTextWidget::slotMenuSelection(mt_Operation)
     */
    virtual void setCustomOpText(const QString &opText) = 0;

    QString operationText(UMLView *view = 0);

    virtual void resetTextPositions();

    /**
     * Motivated by FloatingTextWidget::setMessageText()
     */
    virtual void setMessageText(FloatingTextWidget *ft) = 0;

    /**
     * Motivated by FloatingTextWidget::handleRename()
     */
    virtual void setText(FloatingTextWidget *ft, const QString &newText) = 0;

    virtual void showPropertiesDialog();

    /**
     * Motivated by FloatingTextWidget::showOpDlg()
     */
    virtual UMLClassifier *seqNumAndOp(QString& seqNum, QString& op) = 0;

    /**
     * Motivated by FloatingTextWidget::showOpDlg()
     */
    virtual void setSeqNumAndOp(const QString &seqNum, const QString &op) = 0;

    /**
     * Abstract operation implemented by inheriting classes.
     * Motivated by FloatingTextWidget::mouseMoveEvent()
     */
    virtual void constrainTextPos(int &textX, int &textY,
                                  int textWidth, int textHeight,
                                  Uml::TextRole tr) = 0;

    virtual void calculateNameTextSegment();

};

#endif
