/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016-2021                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLINSTANCE_H
#define UMLINSTANCE_H

#include "umlobject.h"
#include "instanceattribute.h"

class UMLClassifier;
class UMLClassifierListItem;

/**
 * This class contains the non-graphical information required for a UML
 * Instance.  An Instance may be either standalone not tied to any other
 * object, or it may be the instance of a classifier.
 * If it represents a classifier then it will contain concrete values for
 * the attributes of the classifier (see m_attrValues).
 * Conversely, in Umbrello a standalone instance cannot have attributes.
 * UMLObject::m_pSecondary is used for storing the reference to the
 * classifier. In case of a standalone instance, m_pSecondary is NULL.
 * UMLObject::m_name is used for storing the instance name.
 * The notation for an instance is
 *   instanceName : classifierName
 * where the entire text is underlined.
 * Both instanceName and classifierName are optional.
 * The notation for a standalone instance not representing a classifier is
 *   instanceName :
 * The notation for an anonymous instance of a classifier is
 *   : classifierName
 * This class inherits from @ref UMLObject which contains most of the
 * information.
 *
 * @short Non-graphical Information for an Instance.
 * @author Lays Rodrigues
 * @author Ralf Habacker
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLInstance : public UMLObject
{
    Q_OBJECT
public:
    explicit UMLInstance(const QString& instanceName = QString(),
                         Uml::ID::Type id = Uml::ID::None,
                         UMLClassifier *classifier = nullptr);

    void setClassifier(UMLClassifier *classifier);
    void setClassifierCmd(UMLClassifier *classifier, bool emitSignal = true);
    UMLClassifier *classifier();

    UMLInstanceAttribute *findChildObjectById(Uml::ID::Type id);

    virtual void saveToXMI1(QXmlStreamWriter& writer);

    /**
     * List of attribute values in case this UMLInstance is an instance of a
     * classifier.
     */
    typedef QList<UMLInstanceAttribute*> AttributeValues;

    AttributeValues& getAttrValues();

protected:
    bool load1(QDomElement& element);
    virtual bool resolveRef();
    bool showPropertiesDialog(QWidget *parent);

    /**
     * Attribute values in case this instance represents a classifier
     * (m_pSecondary != nullptr)
     */
    AttributeValues m_attrValues;

public slots:
    void attributeAdded(UMLClassifierListItem*);
    void attributeRemoved(UMLClassifierListItem*);
};

#endif // UMLINSTANCE_H
