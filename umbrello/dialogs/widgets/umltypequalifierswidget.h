#ifndef UMLTYPEQUALIFIERSWIDGET_H
#define UMLTYPEQUALIFIERSWIDGET_H

#include "ui_umltypequalifierswidget.h"
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

private:

    Ui::UMLTypeQualifiersWidget *ui;
    UMLClassifierListItem *m_qualifier;

};

#endif // UMLTYPEQUALIFIERSWIDGET_H
