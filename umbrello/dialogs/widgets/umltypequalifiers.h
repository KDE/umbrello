#ifndef UMLTYPEQUALIFIERS_H
#define UMLTYPEQUALIFIERS_H

#include "basictypes.h"

#include <QtWidgets>
#include <QMap>


class UmlTypeQualifiers : public QWidget
{
    Q_OBJECT
public:
    explicit UmlTypeQualifiers(QWidget *parent = 0);

    void addToLayout(QVBoxLayout *layout);

    typedef QMap<Uml::TypeQualifiers::Enum,QString> TextMap;
    typedef QMap<Uml::TypeQualifiers::Enum,QRadioButton*> ButtonMap;

private:
    void init(const QString &title);

    TextMap m_texts;
    QGroupBox *m_box;
    ButtonMap m_buttons;

};

#endif // UMLTYPEQUALIFIERS_H
