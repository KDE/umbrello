#ifndef UMLTYPEQUALIFIERSWIDGET_H
#define UMLTYPEQUALIFIERSWIDGET_H

#include "basictypes.h"

#include <QtWidgets>
#include <QMap>

class UMLClassifierListItem;

class UmlTypeQualifiersWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UmlTypeQualifiersWidget(QWidget *parent = 0);

    void apply();
    void setUMLClassifierItem(UMLClassifierListItem *o);

    typedef QMap<Uml::TypeQualifiers::Enum,QString> TextMap;
    typedef QMap<Uml::TypeQualifiers::Enum,QRadioButton*> ButtonMap;

private:
    void init(const QString &title);

    TextMap m_texts;
    QGridLayout *m_box;
    ButtonMap m_buttons;
    UMLClassifierListItem *m_qualifier;
    QLabel *m_label;

};

#endif // UMLTYPEQUALIFIERSWIDGET_H
