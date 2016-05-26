#ifndef UMLTYPEMODIFIERS_H
#define UMLTYPEMODIFIERS_H

#include "basictypes.h"

#include <QtWidgets>
#include <QMap>


class UmlTypeModifiers : public QWidget
{
    Q_OBJECT
public:
    explicit UmlTypeModifiers(QWidget *parent = 0);

    void addToLayout(QVBoxLayout *layout);

    typedef QMap<Uml::TypeModifiers::Enum,QString> TextMap;
    typedef QMap<Uml::TypeModifiers::Enum,QRadioButton*> ButtonMap;

private:
    void init(const QString &title);

    TextMap m_texts;
    QGroupBox *m_box;
    ButtonMap m_buttons;

};

#endif // UMLTYPEMODIFIERS_H
