#ifndef UMLINSTANCEATTRIBUTE_H
#define UMLINSTANCEATTRIBUTE_H

#include "attribute.h"
#include "basictypes.h"

/**
 * This class is used to set up information for an instanceattribute.
 * It has type, name, and default value.
 *
 * @short Sets up instanceattribute information
 * @author Lays Rodrigues
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class UMLInstanceAttribute : public UMLAttribute
{
    Q_OBJECT
public:
    UMLInstanceAttribute(UMLObject* parent, const QString& name,
                         Uml::ID::Type id = Uml::ID::None,
                         Uml::Visibility::Enum s = Uml::Visibility::Private,
                         UMLObject *type = 0, const QString& value = QString());

    explicit UMLInstanceAttribute(UMLObject *parent);

    void setAttributes(const QString& attributes);
    QString getAttributes() const;

    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

    virtual bool showPropertiesDialog(QWidget* parent = 0);

protected:
    virtual bool load(QDomElement &element);

private:
    void init();
    QString m_attributes;
};

#endif // UMLINSTANCEATTRIBUTE_H
