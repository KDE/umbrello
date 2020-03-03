/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLINSTANCE_H
#define UMLINSTANCE_H

#include "classifier.h"

/**
 * This class contains the non-graphical information required for a UML
 * Instance.
 * This class inherits from @ref UMLClassifier which contains most of the
 * information.
 *
 * @short Non-graphical Information for an Instance.
 * @author Lays Rodrigues
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLInstance : public UMLClassifier
{
public:
    explicit UMLInstance(const QString& name = QString(), Uml::ID::Type id = Uml::ID::None);

    void setClassifierName(const QString &name);
    QString classifierName();

    void setClassifier(UMLClassifier *classifier);
    void setClassifierCmd(UMLClassifier *classifier);
    UMLClassifier *classifier();

    virtual UMLAttribute* createAttribute(const QString &name = QString(),
                                  UMLObject *type = 0, Uml::Visibility::Enum vis = Uml::Visibility::Private,
                                  const QString &init = QString());

    virtual void saveToXMI1(QDomDocument& qDoc, QDomElement& qElement);


protected:
    bool load1(QDomElement& element);
    virtual bool resolveRef();
    bool showPropertiesDialog(QWidget *parent);

    QPointer<UMLClassifier> m_classifier;
};

#endif // UMLINSTANCE_H
