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
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class UMLInstance : public UMLClassifier
{
public:
    explicit UMLInstance(const QString& name = QString(), Uml::ID::Type id = Uml::ID::None);

    virtual UMLAttribute* createAttribute(const QString &name = QString(),
                                  UMLObject *type = 0, Uml::Visibility::Enum vis = Uml::Visibility::Private,
                                  const QString &init = QString());

    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

    virtual UMLClassifierListItemList getFilteredList(ObjectType ot) const;

protected:
    bool load(QDomElement& element);
};

#endif // UMLINSTANCE_H
