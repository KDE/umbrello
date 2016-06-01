#ifndef UMLTYPEQUALIFIERSWIDGET_H
#define UMLTYPEQUALIFIERSWIDGET_H

#include "basictypes.h"

#include <QtWidgets>
#include <QMap>

class UmlClassifierListItem;

class UmlTypeQualifiersWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UmlTypeQualifiersWidget(UmlClassifierListItem *o, QWidget *parent = 0);

    void addToLayout(QGridLayout *layout,int row);
    void apply();

    typedef QMap<Uml::TypeQualifiers::Enum,QString> TextMap;
    typedef QMap<Uml::TypeQualifiers::Enum,QRadioButton*> ButtonMap;

private:
    void init(const QString &title);

    TextMap m_texts;
    QHBoxLayout *m_box;
    ButtonMap m_buttons;
    UmlClassifierListItem *m_qualifier;
    QLabel *m_label;

};

#endif // UMLTYPEQUALIFIERSWIDGET_H
