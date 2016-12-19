#ifndef UMLTYPEMODIFIERWIDGET_H
#define UMLTYPEMODIFIERWIDGET_H

#include "ui_umltypemodifierwidget.h"

// app includes
#include "basictypes.h"

// Qt includes
#include <QWidget>

class UMLClassifierListItem;

class UMLTypeModifierWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UMLTypeModifierWidget(QWidget *parent = 0);

    void apply();

    void setUMLClassifierItem(UMLClassifierListItem *o);

private:
    Ui::UMLTypeModifierWidget *ui;
    typedef QMap<Uml::TypeModifiers::Enum,QString> TextMap;
    TextMap m_texts;
    UMLClassifierListItem *m_typeModifier;

};

#endif // UMLTYPEMODIFIERWIDGET_H
