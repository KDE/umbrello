#ifndef UMLTYPEMODIFIERWIDGET_H
#define UMLTYPEMODIFIERWIDGET_H

#include <QtWidgets>

#include "basictypes.h"

class UMLClassifierListItem;

class UMLTypeModifierWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UMLTypeModifierWidget(QWidget *parent = 0);

    void apply();

    void setUMLClassifierItem(UMLClassifierListItem *o);

private:
    void init(const QString &title);
    typedef QMap<Uml::TypeModifiers::Enum,QString> TextMap;
    TextMap m_texts;
    UMLClassifierListItem *m_typeModifier;
    QComboBox *m_typeCB;

};

#endif // UMLTYPEMODIFIERWIDGET_H
